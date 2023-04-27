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

#if defined(ENABLE_XTEST)

#include "keyboard_xtest.h"
#include "local_actions.h"

// XTest
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/extensions/XTest.h>

#include <QObject>
#include <QDebug>

/* ************************************************************************** */

Keyboard_xtest::Keyboard_xtest(QObject *parent): Keyboard(parent)
{
    //
}

Keyboard_xtest::~Keyboard_xtest()
{
    if (m_display)
    {
        if (XCloseDisplay((Display *)m_display) != 0)
        {
            qWarning() << "XCloseDisplay() error";
        }
    }
}

/* ************************************************************************** */

void Keyboard_xtest::setup()
{
    m_display = XOpenDisplay(nullptr);
    if (!m_display)
    {
        qWarning() << "XOpenDisplay() error";
    }
}

/* ************************************************************************** */

void Keyboard_xtest::action(int action_code)
{
    unsigned keymacro = 0;

    if (action_code == LocalActions::ACTION_KEYBOARD_computer_lock) keymacro = XF86XK_ScreenSaver;
    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_sleep) keymacro = XF86XK_Sleep;
    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_poweroff) keymacro = XF86XK_PowerOff;

    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_up) keymacro = XF86XK_MonBrightnessUp;
    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_down) keymacro = XF86XK_MonBrightnessDown;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_onoff) keymacro = XF86XK_KbdLightOnOff;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_up) keymacro = XF86XK_KbdBrightnessUp;
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_down) keymacro = XF86XK_KbdBrightnessDown;

    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_backward) keymacro = XF86XK_Back;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_forward) keymacro = XF86XK_Forward;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_refresh) keymacro = XF86XK_Refresh;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_stop) keymacro = XF86XK_Stop;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_fullscreen) keymacro = XF86XK_FullScreen;

    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_web) keymacro = XF86XK_WWW;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_mail) keymacro = XF86XK_Mail;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calendar) keymacro = XF86XK_Calendar;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calculator) keymacro = XF86XK_Calculator;
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_files) keymacro = XF86XK_Explorer;

    else if (action_code == LocalActions::ACTION_KEYBOARD_media_playpause) keymacro = XF86XK_AudioPlay;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_stop) keymacro = XF86XK_AudioStop;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_next) keymacro = XF86XK_AudioNext;
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_prev) keymacro = XF86XK_AudioPrev;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_mute) keymacro = XF86XK_AudioMute;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_up) keymacro = XF86XK_AudioRaiseVolume;
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_down) keymacro = XF86XK_AudioLowerVolume;

    if (keymacro)
    {
        Display *display = XOpenDisplay(nullptr);
        if (display)
        {
            unsigned keycode = XKeysymToKeycode(display, keymacro);
            XTestFakeKeyEvent(display, keycode, True, 0);
            XTestFakeKeyEvent(display, keycode, False, 0);
            XFlush(display);

            XCloseDisplay(display);
        }
    }
}

/* ************************************************************************** */
#endif // defined(ENABLE_XTEST)
