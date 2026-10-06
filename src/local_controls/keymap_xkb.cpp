/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 2022 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2026
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "keymap_xkb.h"

#include <xkbcommon/xkbcommon.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QDebug>

static constexpr qint64 kKeymapMaxAgeMs = 10000;
static constexpr unsigned kMaxKeyCode = 255;        //!< the uinput keyboard only enables these

static constexpr unsigned kEvdevLeftShift = 42;     //!< KEY_LEFTSHIFT
static constexpr unsigned kEvdevRightAlt = 100;     //!< KEY_RIGHTALT

/* ************************************************************************** */

/*!
 * \brief XKB RMLVO names, without the rules (always the default "evdev").
 */
struct XkbNames
{
    QByteArray model;
    QByteArray layout;
    QByteArray variant;
    QByteArray options;
};

/*!
 * \return The output of 'gsettings get <schema> <key>', or an empty string.
 */
static QString gsettingsGet(const QString &schema, const QString &key)
{
    QProcess p;
    p.start(QStringLiteral("gsettings"), { QStringLiteral("get"), schema, key });
    if (!p.waitForFinished(500) || p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0)
    {
        return QString();
    }
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}

/*!
 * \brief Read the current GNOME input source, ex: "[('xkb', 'fr+oss'), ('xkb', 'us')]".
 * \note 'mru-sources' starts with the current source, 'sources' is the configured order.
 */
static bool namesFromGnome(XkbNames &n)
{
    static const QRegularExpression rxSource(QStringLiteral(R"(\('xkb',\s*'([^']+)'\))"));
    static const QRegularExpression rxOption(QStringLiteral("'([^']+)'"));
    const QString schema = QStringLiteral("org.gnome.desktop.input-sources");

    QRegularExpressionMatch m = rxSource.match(gsettingsGet(schema, QStringLiteral("mru-sources")));
    if (!m.hasMatch()) m = rxSource.match(gsettingsGet(schema, QStringLiteral("sources")));
    if (!m.hasMatch()) return false;

    const QStringList lv = m.captured(1).split('+');
    n.layout = lv.value(0).toUtf8();
    n.variant = lv.value(1).toUtf8();

    QStringList options;
    auto it = rxOption.globalMatch(gsettingsGet(schema, QStringLiteral("xkb-options")));
    while (it.hasNext()) options << it.next().captured(1);
    n.options = options.join(',').toUtf8();

    return !n.layout.isEmpty();
}

/*!
 * \brief Read the first KDE layout, from the [Layout] group of kxkbrc.
 * \note When 'Use' is false, KDE keeps the system layout, read from localed instead.
 */
static bool namesFromKde(XkbNames &n)
{
    const QString path = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                                                QStringLiteral("kxkbrc"));
    if (path.isEmpty()) return false;

    QSettings s(path, QSettings::IniFormat);
    s.beginGroup(QStringLiteral("Layout"));
    if (!s.value(QStringLiteral("Use"), false).toBool()) return false;

    n.model = s.value(QStringLiteral("Model")).toString().toUtf8();
    n.layout = s.value(QStringLiteral("LayoutList")).toStringList().value(0).toUtf8();
    n.variant = s.value(QStringLiteral("VariantList")).toStringList().value(0).toUtf8();
    n.options = s.value(QStringLiteral("Options")).toStringList().join(',').toUtf8();

    return !n.layout.isEmpty();
}

/*!
 * \brief Read the system layout from systemd-localed, its first layout only.
 */
static bool namesFromLocale1(XkbNames &n)
{
    QDBusInterface localed(QStringLiteral("org.freedesktop.locale1"),
                           QStringLiteral("/org/freedesktop/locale1"),
                           QStringLiteral("org.freedesktop.locale1"),
                           QDBusConnection::systemBus());
    if (!localed.isValid()) return false;

    n.model = localed.property("X11Model").toString().toUtf8();
    n.layout = localed.property("X11Layout").toString().section(',', 0, 0).toUtf8();
    n.variant = localed.property("X11Variant").toString().section(',', 0, 0).toUtf8();
    n.options = localed.property("X11Options").toString().toUtf8();

    return !n.layout.isEmpty();
}

/*!
 * \return The desktop layout names, empty to let xkbcommon use XKB_DEFAULT_* or its defaults.
 */
static XkbNames detectNames()
{
    if (qEnvironmentVariableIsSet("XKB_DEFAULT_LAYOUT")) return {};

    const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    XkbNames n;

    if (desktop.contains(QStringLiteral("GNOME"), Qt::CaseInsensitive) && namesFromGnome(n)) return n;
    n = {};
    if (desktop.contains(QStringLiteral("KDE"), Qt::CaseInsensitive) && namesFromKde(n)) return n;
    n = {};
    if (namesFromLocale1(n)) return n;

    return {};
}

/*!
 * \return True for the keypad keys, which some applications (ex: terminals) remap.
 */
static bool isKeypad(unsigned evdevCode, xkb_keysym_t sym)
{
    if (sym >= XKB_KEY_KP_Space && sym <= XKB_KEY_KP_Equal) return true;

    return (evdevCode >= 71 && evdevCode <= 83) || evdevCode == 55 || evdevCode == 95 ||
           evdevCode == 96 || evdevCode == 98 || evdevCode == 117 || evdevCode == 118 ||
           evdevCode == 121;
}

/*!
 * \return The dead key composing this combining mark, or XKB_KEY_NoSymbol.
 */
static xkb_keysym_t deadKeyForMark(char32_t mark)
{
    switch (mark)
    {
    case 0x0300: return XKB_KEY_dead_grave;
    case 0x0301: return XKB_KEY_dead_acute;
    case 0x0302: return XKB_KEY_dead_circumflex;
    case 0x0303: return XKB_KEY_dead_tilde;
    case 0x0308: return XKB_KEY_dead_diaeresis;
    case 0x030A: return XKB_KEY_dead_abovering;
    case 0x030C: return XKB_KEY_dead_caron;
    case 0x0327: return XKB_KEY_dead_cedilla;
    default: return XKB_KEY_NoSymbol;
    }
}

/*!
 * \return The dead key that, followed by a space, types this spacing accent, or XKB_KEY_NoSymbol.
 */
static xkb_keysym_t deadKeyForSpacingAccent(char32_t c)
{
    switch (c)
    {
    case U'`': return XKB_KEY_dead_grave;
    case U'^': return XKB_KEY_dead_circumflex;
    case U'~': return XKB_KEY_dead_tilde;
    case 0x00A8: return XKB_KEY_dead_diaeresis;
    case 0x00B4: return XKB_KEY_dead_acute;
    default: return XKB_KEY_NoSymbol;
    }
}

/* ************************************************************************** */

KeymapXkb::KeymapXkb()
{
    m_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_context) qWarning() << "KeymapXkb: cannot create the xkb context";
}

KeymapXkb::~KeymapXkb()
{
    xkb_keymap_unref(m_keymap);
    xkb_context_unref(m_context);
}

/* ************************************************************************** */

void KeymapXkb::refresh()
{
    m_age.start();
    if (!m_context) return;

    const XkbNames n = detectNames();
    const QByteArrayList names = { n.model, n.layout, n.variant, n.options };
    if (m_keymap && names == m_names) return;

    const xkb_rule_names rmlvo = {
        nullptr, n.model.constData(), n.layout.constData(), n.variant.constData(), n.options.constData()
    };
    xkb_keymap *keymap = xkb_keymap_new_from_names(m_context, &rmlvo, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap)
    {
        qWarning() << "KeymapXkb: cannot compile the keymap for" << names;
        return;
    }

    xkb_keymap_unref(m_keymap);
    m_keymap = keymap;
    m_names = names;
    build();

    qDebug() << "KeymapXkb: layout" << names << "maps" << m_chars.size() << "characters";
}

void KeymapXkb::build()
{
    m_chars.clear();
    m_deadKeys.clear();

    const xkb_keycode_t min = xkb_keymap_min_keycode(m_keymap);
    const xkb_keycode_t max = qMin(xkb_keymap_max_keycode(m_keymap), xkb_keycode_t(kMaxKeyCode + 8));

    // xkb keycodes are the evdev codes offset by 8
    auto producesSym = [this](xkb_keycode_t kc, xkb_keysym_t sym) {
        const xkb_keysym_t *syms = nullptr;
        return xkb_keymap_key_get_syms_by_level(m_keymap, kc, 0, 0, &syms) == 1 && syms[0] == sym;
    };
    // the physical key first, keymaps also bind modifiers to virtual keys (ex: <LVL3>)
    auto findModifierKey = [&](xkb_keysym_t sym, unsigned preferredEvdev) -> xkb_keycode_t {
        if (producesSym(preferredEvdev + 8, sym)) return preferredEvdev + 8;
        for (xkb_keycode_t kc = min; kc <= max; kc++)
        {
            if (producesSym(kc, sym)) return kc;
        }
        return XKB_KEYCODE_INVALID;
    };
    const xkb_keycode_t shift = findModifierKey(XKB_KEY_Shift_L, kEvdevLeftShift);
    const xkb_keycode_t level3 = findModifierKey(XKB_KEY_ISO_Level3_Shift, kEvdevRightAlt);

    // keypad keys are only used for the characters no other key produces
    QHash<char32_t, KeyStroke> keypad;

    // simplest combination first, so each character keeps the easiest way to type it
    QList<QVarLengthArray<xkb_keycode_t, 2>> combos = { {} };
    if (shift != XKB_KEYCODE_INVALID) combos.push_back({ shift });
    if (level3 != XKB_KEYCODE_INVALID) combos.push_back({ level3 });
    if (shift != XKB_KEYCODE_INVALID && level3 != XKB_KEYCODE_INVALID) combos.push_back({ shift, level3 });

    for (const auto &combo : std::as_const(combos))
    {
        xkb_state *state = xkb_state_new(m_keymap);
        if (!state) continue;

        KeyStroke stroke;
        for (const xkb_keycode_t mod : combo)
        {
            xkb_state_update_key(state, mod, XKB_KEY_DOWN);
            stroke.modifiers.push_back(mod - 8);
        }

        for (xkb_keycode_t kc = qMax(min, xkb_keycode_t(9)); kc <= max; kc++)
        {
            if (combo.contains(kc)) continue;
            stroke.code = kc - 8;

            const xkb_keysym_t sym = xkb_state_key_get_one_sym(state, kc);
            if (sym >= XKB_KEY_dead_grave && sym <= XKB_KEY_dead_greek)
            {
                if (!m_deadKeys.contains(sym)) m_deadKeys.insert(sym, stroke);
                continue;
            }

            const char32_t c = xkb_state_key_get_utf32(state, kc);
            if ((c >= 0x20 && c != 0x7F) || c == U'\t' || c == U'\r')
            {
                QHash<char32_t, KeyStroke> &map = isKeypad(stroke.code, sym) ? keypad : m_chars;
                if (!map.contains(c)) map.insert(c, stroke);
            }
        }

        xkb_state_unref(state);
    }

    for (auto it = keypad.cbegin(); it != keypad.cend(); ++it)
    {
        if (!m_chars.contains(it.key())) m_chars.insert(it.key(), it.value());
    }
}

/* ************************************************************************** */

QList<KeyStroke> KeymapXkb::strokesFor(char32_t c)
{
    if (!m_age.isValid() || m_age.hasExpired(kKeymapMaxAgeMs)) refresh();
    if (!m_keymap) return {};

    if (c == U'\n') c = U'\r';

    const auto direct = m_chars.constFind(c);
    if (direct != m_chars.cend()) return { *direct };

    // composed character: dead key, then the base character (ex: 'ê' as dead '^' then 'e')
    const QString decomposed = QChar::decomposition(c);
    if (QChar::decompositionTag(c) == QChar::Canonical && decomposed.size() == 2)
    {
        const auto base = m_chars.constFind(decomposed.at(0).unicode());
        const auto dead = m_deadKeys.constFind(deadKeyForMark(decomposed.at(1).unicode()));
        if (base != m_chars.cend() && dead != m_deadKeys.cend()) return { *dead, *base };
    }

    // spacing accent: dead key, then space (ex: '^' as dead '^' then ' ')
    const auto dead = m_deadKeys.constFind(deadKeyForSpacingAccent(c));
    const auto space = m_chars.constFind(U' ');
    if (dead != m_deadKeys.cend() && space != m_chars.cend()) return { *dead, *space };

    return {};
}

/* ************************************************************************** */
