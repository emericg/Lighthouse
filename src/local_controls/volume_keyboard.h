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
 * \date      2025
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef VOLUME_KEYBOARD_H
#define VOLUME_KEYBOARD_H
/* ************************************************************************** */

#include "volume.h"

#include <QObject>

class Keyboard;

/* ************************************************************************** */

/*!
 * Volume controller actuated through the virtual keyboard's media keys
 * (XF86AudioRaiseVolume / LowerVolume / Mute).
 *
 * Unlike the audio-server backends, this one is OS-independent: it relies on the
 * Keyboard abstraction (uinput / xtest / sendinput / ...). Its main advantage is
 * that the desktop environment handles the key, so it shows its native volume OSD.
 *
 * It is relative-only: media keys cannot set an absolute level, so setVolume() is a
 * no-op and the level is never known (getVolume() stays -1).
 */
class Volume_keyboard: public Volume
{
    Q_OBJECT

    Keyboard *m_keyboard = nullptr; //!< not owned

public:
    Volume_keyboard(Keyboard *keyboard, QObject *parent = nullptr);
    virtual ~Volume_keyboard() = default;

    virtual void setup() override;

    virtual void setVolume(float volume) override; //!< unsupported (no absolute level via media keys)
    virtual void setMute(bool mute) override;      //!< unsupported (only a toggle key exists)

    virtual void volumeUp(float step = 0.05f) override;
    virtual void volumeDown(float step = 0.05f) override;
    virtual void toggleMute() override;
};

/* ************************************************************************** */
#endif // VOLUME_KEYBOARD_H
