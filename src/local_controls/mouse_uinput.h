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

#ifndef MOUSE_UINPUT_H
#define MOUSE_UINPUT_H
/* ************************************************************************** */

#include "mouse.h"

#include <linux/input.h>
#include <linux/uinput.h>

#include <QObject>

/*!
 * Virtual mouse (Linux uinput version)
 */
class Mouse_uinput: public Mouse
{
    Q_OBJECT

    int m_fd = -1;
    struct uinput_user_dev m_uidev;

    void emitevent(int type, int code, int val);

public:
    Mouse_uinput(QObject *parent = nullptr);
    virtual ~Mouse_uinput();

    virtual void setup();
    virtual void action(int action_code);
    virtual void action(int x, int y, int btn_left, int btn_right, int btn_middle);
};

/* ************************************************************************** */
#endif // MOUSE_UINPUT_H
