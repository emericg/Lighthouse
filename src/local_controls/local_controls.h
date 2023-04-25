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

#ifndef DEVICE_LOCAL_CONTROLS_H
#define DEVICE_LOCAL_CONTROLS_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

#include <QMediaPlayer>
#include <QAudioOutput>

#include "mpris_dbus.h"
#include "mouse.h"
#include "keyboard.h"
#include "gamepad.h"

/* ************************************************************************** */

/*!
 * LocalControls is the action dispatcher
 */
class LocalControls: public QObject
{
    Q_OBJECT

    MprisController *mpris = nullptr;
    Mouse *mouse = nullptr;
    Keyboard *keyboard = nullptr;
    Gamepad *gamepad = nullptr;

    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;

    // Singleton
    static LocalControls *instance;
    LocalControls();
    ~LocalControls();

public:
    static LocalControls *getInstance();

    Q_INVOKABLE void action(int action_code, const QString &action_params = QString());

    Q_INVOKABLE void mouse_action(int x, int y, int btn_left, int btn_right);

    Q_INVOKABLE void joystick_action(float x, float y, int b);

    // shortcuts
    Q_INVOKABLE void keyboard_computer_lock();
    Q_INVOKABLE void keyboard_media_playpause();
    Q_INVOKABLE void keyboard_media_stop();
    Q_INVOKABLE void keyboard_media_next();
    Q_INVOKABLE void keyboard_media_prev();
    Q_INVOKABLE void keyboard_volume_up();
    Q_INVOKABLE void keyboard_volume_down();
    Q_INVOKABLE void keyboard_volume_mute();
};

/* ************************************************************************** */
#endif // DEVICE_LOCAL_CONTROLS_H
