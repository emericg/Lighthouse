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

#ifdef ENABLE_KEYBOARD_UINPUT
#ifndef KEYBOARD_UINPUT_H
#define KEYBOARD_UINPUT_H
/* ************************************************************************** */

#include "keyboard.h"
#include "keymap_xkb.h"

#include <linux/input.h>
#include <linux/uinput.h>

#include <QObject>

/*!
 * Virtual keyboard (Linux uinput version)
 */
class Keyboard_uinput: public Keyboard
{
    Q_OBJECT

    int m_fd = -1;
    struct uinput_user_dev m_uidev;

    KeymapXkb m_keymap;

    void emitevent(int type, int code, int val);

    /*!
     * \brief Press and release a key, with its modifiers held down around it.
     */
    void emitStroke(const KeyStroke &stroke);

public:
    Keyboard_uinput(QObject *parent = nullptr);
    virtual ~Keyboard_uinput();

    virtual void setup();
    virtual void action(int key_code);
    /*!
     * \brief Type a single character, using the desktop keyboard layout (see KeymapXkb).
     * \note Characters the layout cannot produce are ignored.
     *       Without a usable keymap, only letters, digits, space, tab and newline are typed,
     *       at fixed QWERTY positions (digits use the shifted AZERTY level).
     */
    virtual void key(char32_t key_value);
};

/* ************************************************************************** */
#endif // KEYBOARD_UINPUT_H
#endif // ENABLE_KEYBOARD_UINPUT
