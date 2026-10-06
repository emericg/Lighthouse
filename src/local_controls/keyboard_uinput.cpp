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
 * \date      2022
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "keyboard_uinput.h"
#include "local_actions.h"

// uinput
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <QDebug>

/* ************************************************************************** */

Keyboard_uinput::Keyboard_uinput(QObject *parent) : Keyboard(parent)
{
    //
}

Keyboard_uinput::~Keyboard_uinput()
{
    if (m_fd >= 0)
    {
        // close virtual keyboard
        if (ioctl(m_fd, UI_DEV_DESTROY) < 0)
        {
            qWarning() << "Cannot close /dev/uinput";
        }
        close(m_fd);
    }
}

/* ************************************************************************** */

void Keyboard_uinput::emitevent(int type, int code, int val)
{
    if (m_fd >= 0)
    {
        struct input_event event;
        memset(&event, 0, sizeof(event));

        event.type = type;
        event.code = code;
        event.value = val;

        // set timestamps
        gettimeofday(&event.time, nullptr);

        // ignore timestamps?
        //event.time.tv_sec = 0;
        //event.time.tv_usec = 0;

        write(m_fd, &event, sizeof(event));
    }
}

/* ************************************************************************** */

void Keyboard_uinput::setup()
{
    int err = 0;

    if (m_fd < 0)
    {
        m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (m_fd < 0)
        {
            qWarning() << "Cannot open /dev/uinput";
            return;
        }

        // enable key events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_KEY) error"; }

        // enable syn events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_SYN);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_SYN) error"; }

        // setup keys
        for (int i = 0; i < 256; i++)
        {
            ioctl(m_fd, UI_SET_KEYBIT, i);
        }

        // init virtual keyboard
        memset(&m_uidev, 0, sizeof(m_uidev));
        snprintf(m_uidev.name, UINPUT_MAX_NAME_SIZE, "Lighthouse virtual keyboard");
        m_uidev.id.bustype = BUS_VIRTUAL;
        m_uidev.id.version = 1;
        m_uidev.id.vendor = 0x1234;
        m_uidev.id.product = 0x2222;

        err = write(m_fd, &m_uidev, sizeof(m_uidev));
        if (err < 0) { qWarning() << "Unable to write /dev/uinput device"; }

        err = ioctl(m_fd, UI_DEV_CREATE);
        if (err < 0) { qWarning() << "ioctl(UI_DEV_CREATE) error"; }
    }
}

void Keyboard_uinput::action(int action_code)
{
    if (m_fd < 0) setup(); // setup?

    // get key macro
    unsigned keymacro = 0;

    if (action_code == LocalActions::ACTION_KEYBOARD_up) keymacro = KEY_UP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_down) keymacro = KEY_DOWN;
    else if (action_code == LocalActions::ACTION_KEYBOARD_left) keymacro = KEY_LEFT;
    else if (action_code == LocalActions::ACTION_KEYBOARD_right) keymacro = KEY_RIGHT;
    else if (action_code == LocalActions::ACTION_KEYBOARD_enter) keymacro = KEY_ENTER;
    else if (action_code == LocalActions::ACTION_KEYBOARD_escape) keymacro = KEY_ESC;
    else if (action_code == LocalActions::ACTION_KEYBOARD_backspace) keymacro = KEY_BACKSPACE;

    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_lock) keymacro = KEY_SCREENLOCK;
    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_sleep) keymacro = KEY_SLEEP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_poweroff) keymacro = KEY_POWER;

    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_up) keymacro = KEY_BRIGHTNESSUP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_down) keymacro = KEY_BRIGHTNESSDOWN;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_onoff) keymacro = KEY_KBDILLUMTOGGLE;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_up) keymacro = KEY_KBDILLUMUP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_down) keymacro = KEY_KBDILLUMDOWN;

    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_backward) keymacro = KEY_BACK;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_forward) keymacro = KEY_FORWARD;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_refresh) keymacro = KEY_REFRESH;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_stop) keymacro = KEY_STOP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_fullscreen) keymacro = KEY_FULL_SCREEN;

    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calculator) keymacro = KEY_CALC;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_web) keymacro = KEY_WWW;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_mail) keymacro = KEY_MAIL;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calendar) keymacro = KEY_CALENDAR;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_files) keymacro = KEY_FILE;

    else if (action_code == LocalActions::ACTION_KEYBOARD_media_playpause) keymacro = KEY_PLAYPAUSE;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_stop) keymacro = KEY_STOPCD;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_next) keymacro = KEY_NEXTSONG;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_prev) keymacro = KEY_PREVIOUSSONG;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_mute) keymacro = KEY_MUTE;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_up) keymacro = KEY_VOLUMEUP;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_down) keymacro = KEY_VOLUMEDOWN;

    // simulate keystroke
    if (m_fd >= 0)
    {
        emitevent(EV_KEY, keymacro, 1);
        emitevent(EV_SYN, SYN_REPORT, 0);
        //usleep(33000); // 33 ms
        emitevent(EV_KEY, keymacro, 0);
        emitevent(EV_SYN, SYN_REPORT, 0);
        //usleep(33000); // 33 ms
    }
}

/* ************************************************************************** */

/*!
 * \return The key stroke typing this character at a fixed QWERTY position, if any.
 */
static QList<KeyStroke> fixedStrokesFor(char32_t c)
{
    // KEY_A to KEY_Z are not contiguous, they follow the physical rows
    static constexpr unsigned kLetterKeys[26] = {
        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
        KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    };

    KeyStroke s;

    if (c >= U'0' && c <= U'9')
    {
        s.code = (c == U'0') ? KEY_0 : KEY_1 + (c - U'1');
        s.modifiers.push_back(KEY_LEFTSHIFT);
    }
    else if (c >= U'a' && c <= U'z')
    {
        s.code = kLetterKeys[c - U'a'];
    }
    else if (c >= U'A' && c <= U'Z')
    {
        s.code = kLetterKeys[c - U'A'];
        s.modifiers.push_back(KEY_LEFTSHIFT);
    }
    else if (c == U' ') s.code = KEY_SPACE;
    else if (c == U'\t') s.code = KEY_TAB;
    else if (c == U'\n') s.code = KEY_ENTER;

    if (s.code == 0) return {};
    return { s };
}

void Keyboard_uinput::key(char32_t key_value)
{
    QList<KeyStroke> strokes = m_keymap.strokesFor(key_value);
    if (strokes.isEmpty() && !m_keymap.isValid()) strokes = fixedStrokesFor(key_value);
    if (strokes.isEmpty()) return;

    if (m_fd < 0) setup();
    if (m_fd < 0) return;

    for (const KeyStroke &s : std::as_const(strokes))
    {
        emitStroke(s);
    }
}

void Keyboard_uinput::emitStroke(const KeyStroke &stroke)
{
    for (const unsigned m : stroke.modifiers) emitevent(EV_KEY, m, 1);
    emitevent(EV_KEY, stroke.code, 1);
    emitevent(EV_SYN, SYN_REPORT, 0);

    emitevent(EV_KEY, stroke.code, 0);
    for (auto it = stroke.modifiers.crbegin(); it != stroke.modifiers.crend(); ++it)
    {
        emitevent(EV_KEY, *it, 0);
    }
    emitevent(EV_SYN, SYN_REPORT, 0);
}

/* ************************************************************************** */
