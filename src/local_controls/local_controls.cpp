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
#include "SettingsManager.h"

#include "mouse_uinput.h"
#include "keyboard_uinput.h"
#include "gamepad_uinput.h"
#include "keyboard_xtest.h"
#include "mpris_dbus.h"
#include "volume_keyboard.h"
#if defined(ENABLE_VOLUME_PIPEWIRE)
#include "volume_pipewire.h"
#elif defined(ENABLE_VOLUME_PULSEAUDIO)
#include "volume_pulseaudio.h"
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
#include <QMediaPlayer>
#include <QAudioOutput>
#endif
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

    return; // nothing to control there...

#endif

#if defined(Q_OS_LINUX)

#if defined(ENABLE_KEYBOARD_UINPUT)
    mouse = new Mouse_uinput();
    keyboard = new Keyboard_uinput();
    gamepad = new Gamepad_uinput();
#elif defined(ENABLE_KEYBOARD_XTEST)
    // only works with X11
    keyboard = new Keyboard_xtest();
#endif

#if defined(ENABLE_MEDIA_MPRIS)
    mpris = Media_MPRIS::getInstance();
    mpris->select_player();
#endif

#if defined(ENABLE_VOLUME_PIPEWIRE)
    volume = new Volume_pipewire();
#elif defined(ENABLE_VOLUME_PULSEAUDIO)
    volume = new Volume_pulseaudio();
#endif

    // relative volume / mute through the virtual keyboard, so the desktop shows its OSD
#if defined(ENABLE_KEYBOARD_UINPUT) || defined(ENABLE_KEYBOARD_XTEST)
    if (keyboard)
    {
        volume_keys = new Volume_keyboard(keyboard);
    }
#endif

#if defined(ENABLE_VOLUME_PIPEWIRE) || defined(ENABLE_VOLUME_PULSEAUDIO)
    if (volume)
    {
        connect(volume, &Volume::volumeChanged, this, &LocalControls::volumeChanged);
        connect(volume, &Volume::muteChanged, this, &LocalControls::muteChanged);

        // apply the configurable desktop volume cap, and keep it in sync
        SettingsManager *sm = SettingsManager::getInstance();
        if (sm)
        {
            volume->setVolumeLimit(sm->getVolumeLimit() / 100.f);
            connect(sm, &SettingsManager::volumeLimitChanged, this, [this]() {
                SettingsManager *sm = SettingsManager::getInstance();
                if (volume && sm) volume->setVolumeLimit(sm->getVolumeLimit() / 100.f);
            });
        }
    }
#endif

#endif // Q_OS_LINUX


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
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
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
#endif
        }

        // fall back to the virtual keyboard (ex: no logind backend, or a real key)
        if (keyboard)
        {
            keyboard->action(action_code);
        }
    }
    else if (action_code > LocalActions::ACTION_MEDIA_START &&
             action_code < LocalActions::ACTION_MEDIA_STOP)
    {
#if defined(ENABLE_MEDIA_MPRIS)
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

void LocalControls::keyboard_key(QChar key)
{
    if (keyboard)
    {
        keyboard->key(key);
    }
}

/* ************************************************************************** */

void LocalControls::mouse_action(int dx, int dy, int btn_left, int btn_right, int btn_middle)
{
    if (mouse) mouse->action_rel(dx, dy, btn_left, btn_right, btn_middle);
}

void LocalControls::mouse_move(int dx, int dy)
{
    if (mouse) mouse->move_rel(dx, dy);
}
void LocalControls::mouse_button(int code, bool pressed)
{
    if (mouse) mouse->button(code, pressed);
}
void LocalControls::mouse_scroll(int dx, int dy)
{
    if (mouse) mouse->scroll(dx, dy);
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
void LocalControls::keyboard_computer_sleep()
{
    action(LocalActions::ACTION_KEYBOARD_computer_sleep);
}
void LocalControls::keyboard_computer_poweroff()
{
    action(LocalActions::ACTION_KEYBOARD_computer_poweroff);
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

void LocalControls::volume_set(float level)
{
    // absolute level only the audio backend can do (and it enforces the cap)
    if (volume) volume->setVolume(level);
}
void LocalControls::volume_up()
{
    // prefer media keys (desktop OSD); fall back to the audio backend
    if (volume_keys) volume_keys->volumeUp();
    else if (volume) volume->volumeUp();
}
void LocalControls::volume_down()
{
    if (volume_keys) volume_keys->volumeDown();
    else if (volume) volume->volumeDown();
}
void LocalControls::volume_mute()
{
    // explicit on/off is deterministic only on the audio backend
    if (volume) volume->mute();
    else if (volume_keys) volume_keys->toggleMute();
}
void LocalControls::volume_unmute()
{
    if (volume) volume->unmute();
}
void LocalControls::volume_toggle_mute()
{
    // prefer media keys (desktop OSD); fall back to the audio backend
    if (volume_keys) volume_keys->toggleMute();
    else if (volume) volume->toggleMute();
}

float LocalControls::getVolumeLevel() const
{
    return volume ? volume->getVolume() : -1.f;
}
bool LocalControls::isMuted() const
{
    return volume ? volume->getMute() : false;
}

/* ************************************************************************** */
