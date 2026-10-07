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
 * \date      2026
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifdef ENABLE_KEYBOARD_CGEVENT
#ifndef KEYBOARD_CGEVENT_H
#define KEYBOARD_CGEVENT_H
/* ************************************************************************** */

#include "keyboard.h"

#include <QObject>

#include <CoreGraphics/CoreGraphics.h>

/*!
 * Virtual keyboard (macOS Quartz events version)
 *
 * Posting events requires the "Accessibility" permission (Privacy & Security
 * settings), which macOS asks for the first time the keyboard is used.
 * Without it, events are silently dropped by the system.
 */
class Keyboard_cgevent: public Keyboard
{
    Q_OBJECT

    CGEventSourceRef m_source = nullptr;
    bool m_permissionRequested = false;

    /*!
     * \brief Press and release a key, with the given modifiers.
     * \param text: when set, the character the key produces, regardless of the keyboard layout.
     */
    void postKey(CGKeyCode code, CGEventFlags flags = 0, char16_t text = 0);

    /*!
     * \brief Press and release a media key (NX_KEYTYPE_*), as sent by the Apple keyboards' top row.
     */
    void postMediaKey(int nx_key);

public:
    Keyboard_cgevent(QObject *parent = nullptr);
    virtual ~Keyboard_cgevent();

    virtual void setup();
    virtual void action(int action_code);
    /*!
     * \brief Type a single character.
     * \note The character is carried by the event itself, so the keyboard layout does not matter,
     *       but applications reading raw key codes (games, remote desktops) will not see it.
     */
    virtual void key(char32_t key_value);
};

/* ************************************************************************** */
#endif // KEYBOARD_CGEVENT_H
#endif // ENABLE_KEYBOARD_CGEVENT
