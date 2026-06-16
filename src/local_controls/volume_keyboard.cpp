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

#include "volume_keyboard.h"
#include "keyboard.h"
#include "local_actions.h"

/* ************************************************************************** */

Volume_keyboard::Volume_keyboard(Keyboard *keyboard, QObject *parent)
    : Volume(parent), m_keyboard(keyboard)
{
    setup();
}

/* ************************************************************************** */

void Volume_keyboard::setup()
{
    m_available = (m_keyboard != nullptr);
    Q_EMIT availabilityChanged();
}

/* ************************************************************************** */

void Volume_keyboard::setVolume(float volume)
{
    // media keys cannot target an absolute level
    Q_UNUSED(volume)
}

void Volume_keyboard::setMute(bool mute)
{
    // there is no explicit mute/unmute key, only a toggle (see toggleMute())
    Q_UNUSED(mute)
}

void Volume_keyboard::volumeUp(float step)
{
    Q_UNUSED(step)
    if (m_keyboard) m_keyboard->action(LocalActions::ACTION_KEYBOARD_volume_up);
}

void Volume_keyboard::volumeDown(float step)
{
    Q_UNUSED(step)
    if (m_keyboard) m_keyboard->action(LocalActions::ACTION_KEYBOARD_volume_down);
}

void Volume_keyboard::toggleMute()
{
    if (m_keyboard) m_keyboard->action(LocalActions::ACTION_KEYBOARD_volume_mute);
}

/* ************************************************************************** */
