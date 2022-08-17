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

#include "keyboard.h"
#include "mpris_dbus.h"

/* ************************************************************************** */

/*!
 * LocalControls is the action dispatcher
 */
class LocalControls: public QObject
{
    Q_OBJECT

    Keyboard *keyboard = nullptr;
    MprisController *mpris = nullptr;

    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;

    // Singleton
    static LocalControls *instance;
    LocalControls();
    ~LocalControls();

public:
    static LocalControls *getInstance();

    Q_INVOKABLE void action(int action_code, const QString &action_params = QString());

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
