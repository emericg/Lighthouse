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

#ifndef KEYBOARD_UINPUT_H
#define KEYBOARD_UINPUT_H
/* ************************************************************************** */

#include "keyboard.h"

#include <linux/input.h>
#include <linux/uinput.h>

#include <QObject>

/*!
 * Keyboard (Linux uinput version)
 */
class Keyboard_uinput: public Keyboard
{
    Q_OBJECT

    int fd = -1;
    struct uinput_user_dev uidev;

public:
    Keyboard_uinput(QObject *parent = nullptr);
    virtual ~Keyboard_uinput();

    virtual void action(int key_code);
};

/* ************************************************************************** */
#endif // KEYBOARD_UINPUT_H
