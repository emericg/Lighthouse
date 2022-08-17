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
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>

#include <QDebug>

/* ************************************************************************** */

Keyboard_uinput::Keyboard_uinput(QObject *parent): Keyboard(parent)
{
    //
}

Keyboard_uinput::~Keyboard_uinput()
{
    //
}

/* ************************************************************************** */

void Keyboard_uinput::action(int action_code)
{
    unsigned keymacro = 0;

    if (action_code == LocalActions::ACTION_KEYBOARD_computer_lock) keymacro = KEY_SCREENLOCK;
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

    if (fd < 0)
    {
        fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    }

    if (fd >= 0)
    {
        // enable keys events
        ioctl(fd, UI_SET_EVBIT, EV_KEY);
        for (int i = 0; i < 256; i++)
        {
            ioctl(fd, UI_SET_KEYBIT, i);
        }

        // enable syn events
        ioctl(fd, UI_SET_EVBIT, EV_SYN);

        // init virtual keyboard
        memset(&uidev, 0, sizeof(uidev));
        snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Lighthouse virtual keyboard");
        uidev.id.version = 1;
        uidev.id.vendor = 0x1234;
        uidev.id.product = 0x5678;
        uidev.id.bustype = BUS_VIRTUAL;

        write(fd, &uidev, sizeof(uidev));
        ioctl(fd, UI_DEV_CREATE);

        usleep(111000); // 111 ms
    }
    else
    {
        qWarning() << "Cannot open /dev/uinput";
    }

    // simulate keystroke
    if (fd >= 0)
    {
        struct input_event event;
        memset(&event, 0, sizeof(event));

        //gettimeofday(&event.time, NULL);
        event.type = EV_KEY;
        event.code = keymacro;
        event.value = 1;
        write(fd, &event, sizeof(event));

        memset(&event, 0, sizeof(event));

        event.type = EV_SYN;
        event.code = SYN_REPORT;
        event.value = 0;
        write(fd, &event, sizeof(event));
    }

    // close virtual keyboard
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

/* ************************************************************************** */
