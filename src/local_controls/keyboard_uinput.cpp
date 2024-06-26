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

Keyboard_uinput::Keyboard_uinput(QObject *parent): Keyboard(parent)
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

void Keyboard_uinput::key(QChar key_value)
{
    // get key macro
    unsigned keymacro = 0;
    unsigned keymodifier = 0;

    if (key_value >= '0' && key_value <= '9')
    {
        keymodifier = KEY_LEFTSHIFT;
        if (key_value == '0') keymacro = KEY_0;
        else keymacro = 1 + key_value.digitValue();
    }
    else if ((key_value >= 'a' && key_value <= 'z') ||
             (key_value >= 'A' && key_value <= 'A'))
    {
        if (key_value >= 'A' && key_value <= 'Z')
            keymodifier = KEY_CAPSLOCK;

        if (key_value.toLower() == 'a') keymacro = KEY_A;
        if (key_value.toLower() == 'b') keymacro = KEY_B;
        if (key_value.toLower() == 'c') keymacro = KEY_C;
        if (key_value.toLower() == 'd') keymacro = KEY_D;
        if (key_value.toLower() == 'e') keymacro = KEY_E;
        if (key_value.toLower() == 'f') keymacro = KEY_F;
        if (key_value.toLower() == 'g') keymacro = KEY_G;
        if (key_value.toLower() == 'h') keymacro = KEY_H;
        if (key_value.toLower() == 'i') keymacro = KEY_I;
        if (key_value.toLower() == 'j') keymacro = KEY_J;
        if (key_value.toLower() == 'k') keymacro = KEY_K;
        if (key_value.toLower() == 'l') keymacro = KEY_L;
        if (key_value.toLower() == 'm') keymacro = KEY_M;
        if (key_value.toLower() == 'n') keymacro = KEY_N;
        if (key_value.toLower() == 'o') keymacro = KEY_O;
        if (key_value.toLower() == 'p') keymacro = KEY_P;
        if (key_value.toLower() == 'q') keymacro = KEY_Q;
        if (key_value.toLower() == 'r') keymacro = KEY_R;
        if (key_value.toLower() == 's') keymacro = KEY_S;
        if (key_value.toLower() == 't') keymacro = KEY_T;
        if (key_value.toLower() == 'u') keymacro = KEY_U;
        if (key_value.toLower() == 'v') keymacro = KEY_V;
        if (key_value.toLower() == 'w') keymacro = KEY_W;
        if (key_value.toLower() == 'x') keymacro = KEY_X;
        if (key_value.toLower() == 'y') keymacro = KEY_Y;
        if (key_value.toLower() == 'z') keymacro = KEY_Z;
    }
    else
    {
        // TODO
    }

    // simulate keystroke
    if (keymacro > 0)
    {
        if (m_fd < 0) setup(); // setup?

        if (m_fd >= 0)
        {
            if (keymodifier > 0) emitevent(EV_KEY, keymodifier, 1);
            emitevent(EV_KEY, keymacro, 1);
            emitevent(EV_SYN, SYN_REPORT, 0);
            //usleep(33000); // 33 ms

            if (keymodifier > 0) emitevent(EV_KEY, keymodifier, 0);
            emitevent(EV_KEY, keymacro, 0);
            emitevent(EV_SYN, SYN_REPORT, 0);
            //usleep(33000); // 33 ms
        }
    }
}

/* ************************************************************************** */
