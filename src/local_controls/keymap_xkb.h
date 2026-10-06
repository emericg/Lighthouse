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

#ifndef KEYMAP_XKB_H
#define KEYMAP_XKB_H
/* ************************************************************************** */

#include <QByteArray>
#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QVarLengthArray>

struct xkb_context;
struct xkb_keymap;

/* ************************************************************************** */

/*!
 * \brief A single key press, with the modifiers held down around it.
 */
struct KeyStroke
{
    unsigned code = 0;                          //!< evdev key code
    QVarLengthArray<unsigned, 2> modifiers;     //!< evdev key codes of the modifiers
};

/* ************************************************************************** */

/*!
 * \brief Reverse keymap: which keys to press to type a given character.
 *
 * A virtual keyboard emits physical key positions, the compositor keymap then decides
 * which character they produce. This compiles the desktop keyboard layout with xkbcommon,
 * and records for every character the simplest key and modifiers (none, Shift, AltGr,
 * Shift + AltGr) producing it, plus the dead keys used to compose accented characters.
 *
 * The layout is read from the GNOME or KDE settings, then systemd-localed,
 * and the XKB_DEFAULT_* variables take precedence when set.
 *
 * \note Only the first layout of the configured list is used,
 *       the compositor's current layout group and lock modifiers are not known.
 */
class KeymapXkb
{
    xkb_context *m_context = nullptr;
    xkb_keymap *m_keymap = nullptr;
    QByteArrayList m_names;                     //!< model, layout, variant, options of m_keymap
    QElapsedTimer m_age;

    QHash<char32_t, KeyStroke> m_chars;
    QHash<quint32, KeyStroke> m_deadKeys;       //!< by dead keysym

    /*!
     * \brief Detect the desktop layout, and rebuild the reverse map if it changed.
     */
    void refresh();

    void build();

public:
    KeymapXkb();
    ~KeymapXkb();

    KeymapXkb(const KeymapXkb &) = delete;
    KeymapXkb &operator=(const KeymapXkb &) = delete;

    /*!
     * \return True if a keymap could be compiled for the desktop layout.
     */
    bool isValid() const { return m_keymap; }

    /*!
     * \param c A Unicode code point, '\n' is typed as Return.
     * \return The key strokes typing that character, two for a dead key composition,
     *         or none if the layout cannot produce it.
     * \note The layout is detected again when the previous detection is over 10 seconds old.
     */
    QList<KeyStroke> strokesFor(char32_t c);
};

/* ************************************************************************** */
#endif // KEYMAP_XKB_H
