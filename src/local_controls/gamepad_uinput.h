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
 * \date      2023
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifdef ENABLE_UINPUT
#ifndef GAMEPAD_UINPUT_H
#define GAMEPAD_UINPUT_H
/* ************************************************************************** */

#include "gamepad.h"

#include <linux/input.h>
#include <linux/uinput.h>

#include <QObject>

/* ************************************************************************** */

/*!
 * Virtual gamepad (Linux uinput version).
 */
class Gamepad_uinput: public Gamepad
{
    Q_OBJECT

    int m_fd = -1;
    struct uinput_user_dev m_uidev;

    void emitevent(int type, int code, int val);

public:
    Gamepad_uinput(QObject *parent = nullptr);
    virtual ~Gamepad_uinput();

    virtual void setup();
    virtual void action(int x1, int y1, int x2, int y2,
                        int a, int b, int x, int y);

    virtual void setup_pbp();
    virtual void action_pbp(int x, int y, int a, int b);
};

/* ************************************************************************** */
#endif // GAMEPAD_UINPUT_H
#endif // ENABLE_UINPUT
