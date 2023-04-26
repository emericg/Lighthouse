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

#ifndef DEVICE_LOCAL_ACTIONS_H
#define DEVICE_LOCAL_ACTIONS_H
/* ************************************************************************** */

#include <QObject>
#include <QQmlContext>
#include <QQmlApplicationEngine>

/* ************************************************************************** */

/*!
 * LocalActions
 */
class LocalActions: public QObject
{
    Q_OBJECT

public:
    static void registerQML()
    {
        qRegisterMetaType<LocalActions::Actions>("LocalActions::LocalActions");

        qmlRegisterType<LocalActions>("LocalActions", 1, 0, "LocalActions");
    }

    //! LocalControls actions list
    enum Actions
    {
        ACTION_NO_ACTION = 0,

        ACTION_keys,
        ACTION_commands,

        ACTION_MOUSE_START = 16,
            ACTION_MOUSE_click_right,
            ACTION_MOUSE_click_left,
            ACTION_MOUSE_click_middle,
        ACTION_MOUSE_STOP,

        ACTION_KEYBOARD_START = 32,
            ACTION_KEYS_START,
                ACTION_KEYBOARD_up,
                ACTION_KEYBOARD_down,
                ACTION_KEYBOARD_left,
                ACTION_KEYBOARD_right,
            ACTION_KEYS_STOP,

            ACTION_SHORTCUTS_START = 64,
                ACTION_KEYBOARD_computer_lock,
                ACTION_KEYBOARD_computer_sleep,
                ACTION_KEYBOARD_computer_poweroff,

                ACTION_KEYBOARD_monitor_brightness_up,
                ACTION_KEYBOARD_monitor_brightness_down,
                ACTION_KEYBOARD_keyboard_brightness_onoff,
                ACTION_KEYBOARD_keyboard_brightness_up,
                ACTION_KEYBOARD_keyboard_brightness_down,

                ACTION_KEYBOARD_desktop_backward = 64,
                ACTION_KEYBOARD_desktop_forward,
                ACTION_KEYBOARD_desktop_refresh,
                ACTION_KEYBOARD_desktop_stop,
                ACTION_KEYBOARD_desktop_fullscreen,

                ACTION_KEYBOARD_desktop_web,
                ACTION_KEYBOARD_desktop_mail,
                ACTION_KEYBOARD_desktop_calendar,
                ACTION_KEYBOARD_desktop_calculator,
                ACTION_KEYBOARD_desktop_files,
            ACTION_SHORTCUTS_STOP,

            ACTION_MEDIA_START = 96,
                ACTION_KEYBOARD_media_playpause,
                ACTION_KEYBOARD_media_stop,
                ACTION_KEYBOARD_media_next,
                ACTION_KEYBOARD_media_prev,

                ACTION_KEYBOARD_volume_mute,
                ACTION_KEYBOARD_volume_up,
                ACTION_KEYBOARD_volume_down,
            ACTION_MEDIA_STOP,
        ACTION_KEYBOARD_STOP,

        ACTION_MPRIS_START = 128,
            ACTION_MPRIS_playpause,
            ACTION_MPRIS_stop,
            ACTION_MPRIS_next,
            ACTION_MPRIS_prev,
            ACTION_MPRIS_volume_mute,
            ACTION_MPRIS_volume_up,
            ACTION_MPRIS_volume_down,
        ACTION_MPRIS_STOP,
    };
    Q_ENUM(Actions)
};

/* ************************************************************************** */
#endif // DEVICE_LOCAL_ACTIONS_H
