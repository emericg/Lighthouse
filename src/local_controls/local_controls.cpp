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

#include "local_controls.h"
#include "local_actions.h"

#include "keyboard_xtest.h"
#include "mouse_uinput.h"
#include "keyboard_uinput.h"
#include "gamepad_uinput.h"
#include "mpris_dbus.h"

#include <QProcess>
#include <QDebug>

/* ************************************************************************** */

LocalControls *LocalControls::instance = nullptr;

LocalControls *LocalControls::getInstance()
{
    if (instance == nullptr)
    {
        instance = new LocalControls();
    }

    return instance;
}

LocalControls::LocalControls()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

    // nothing to control there...

#elif defined(Q_OS_LINUX)

    //keyboard = new Keyboard_xtest(); // only works with X11
    mouse = new Mouse_uinput();
    keyboard = new Keyboard_uinput();
    gamepad = new Gamepad_uinput();
    mpris = MprisController::getInstance();

#else

    qWarning() << "No backends available for LocalControls()";

#endif
}

LocalControls::~LocalControls()
{
    //
}

/* ************************************************************************** */

void LocalControls::action(int action_code, const QString &action_params)
{
    if (action_code == LocalActions::ACTION_keys)
    {
        // TODO
    }
    else if (action_code == LocalActions::ACTION_commands)
    {
        QString cmd;
        QStringList args = action_params.split(" ");

        if (args.size() > 1)
        {
            cmd = args.at(0);
            args.pop_front();
        }
        else
        {
            cmd = action_params;
            args.clear();
        }

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
        QProcess::startDetached(cmd, args);
#endif
    }
    else if (action_code > LocalActions::ACTION_MOUSE_START &&
             action_code < LocalActions::ACTION_MOUSE_STOP)
    {
        if (mouse)
        {
            mouse->action(action_code);
        }
    }
    else if (action_code > LocalActions::ACTION_KEYBOARD_START &&
             action_code < LocalActions::ACTION_KEYBOARD_STOP)
    {
        if (action_code == LocalActions::ACTION_KEYBOARD_computer_lock)
        {
            // sound effect?

            if (!player)
            {
                audioOutput = new QAudioOutput;
                audioOutput->setVolume(100);

                player = new QMediaPlayer;
                player->setAudioOutput(audioOutput);
            }

            if (player)
            {
                player->setSource(QUrl("qrc:/assets/sounds/car-lock-sound-effect.ogg"));
                player->play();
            }
        }

        if (keyboard)
        {
            keyboard->action(action_code);
        }
    }
    else if (action_code > LocalActions::ACTION_MPRIS_START &&
             action_code < LocalActions::ACTION_MPRIS_STOP)
    {
#if defined(ENABLE_MPRIS)
        if (mpris)
        {
            mpris->action(action_code);
        }
#endif
    }
    else
    {
        if (action_code != LocalActions::ACTION_NO_ACTION)
        {
            qDebug() << "LocalControls::action() unknown action code '" << action_code << "' :(";
        }
    }
}

/* ************************************************************************** */

void LocalControls::mouse_action(int x, int y, int btn_left, int btn_right, int btn_middle)
{
    mouse->action(x, y, btn_left, btn_right, btn_middle);
}

/* ************************************************************************** */

void LocalControls::gamepad_action(float x1, float y1, float x2, float y2,
                                   int a, int b, int x, int y)
{
    gamepad->action(x1, y1, x2, y2, a, b, x, y);
}

/* ************************************************************************** */

void LocalControls::keyboard_computer_lock()
{
    action(LocalActions::ACTION_KEYBOARD_computer_lock);
}

void LocalControls::keyboard_media_prev()
{
    action(LocalActions::ACTION_KEYBOARD_media_prev);
}
void LocalControls::keyboard_media_stop()
{
    action(LocalActions::ACTION_KEYBOARD_media_stop);
}
void LocalControls::keyboard_media_playpause()
{
    action(LocalActions::ACTION_KEYBOARD_media_playpause);
}
void LocalControls::keyboard_media_next()
{
    action(LocalActions::ACTION_KEYBOARD_media_next);
}

void LocalControls::keyboard_volume_mute()
{
    action(LocalActions::ACTION_KEYBOARD_volume_mute);
}
void LocalControls::keyboard_volume_down()
{
    action(LocalActions::ACTION_KEYBOARD_volume_down);
}
void LocalControls::keyboard_volume_up()
{
    action(LocalActions::ACTION_KEYBOARD_volume_up);
}

/* ************************************************************************** */
