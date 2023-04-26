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

#include "mouse_uinput.h"
#include "local_actions.h"

// uinput
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <QDebug>

/* ************************************************************************** */

Mouse_uinput::Mouse_uinput(QObject *parent): Mouse(parent)
{
    //
}

Mouse_uinput::~Mouse_uinput()
{
    if (m_fd >= 0)
    {
        // close virtual mouse
        if (ioctl(m_fd, UI_DEV_DESTROY) < 0)
        {
            qWarning() << "Cannot close /dev/uinput";
        }
        close(m_fd);
    }
}

/* ************************************************************************** */

void Mouse_uinput::emitevent(int type, int code, int val)
{
    if (m_fd >= 0)
    {
        struct input_event event;
        memset(&event, 0, sizeof(event));

        event.type = type;
        event.code = code;
        event.value = val;

        // set timestamps
        gettimeofday(&event.time, nullptr);

        // ignore timestamps?
        //event.time.tv_sec = 0;
        //event.time.tv_usec = 0;

        write(m_fd, &event, sizeof(event));
    }
}

/* ************************************************************************** */

void Mouse_uinput::setup()
{
    int err = 0;

    if (m_fd < 0)
    {
        m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (m_fd < 0)
        {
            qWarning() << "Cannot open /dev/uinput";
            return;
        }

        // enable key events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_KEY) error"; }

        // enable rel(ative) events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_REL);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_REL) error"; }

        // enable syn events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_SYN);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_SYN) error"; }

        // setup mouse buttons

        ioctl(m_fd, UI_SET_RELBIT, REL_X);
        ioctl(m_fd, UI_SET_RELBIT, REL_Y);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_LEFT);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_RIGHT);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_MIDDLE);

        // init virtual mouse
        memset(&m_uidev, 0, sizeof(m_uidev));
        snprintf(m_uidev.name, UINPUT_MAX_NAME_SIZE, "Lighthouse virtual mouse");
        m_uidev.id.bustype = BUS_VIRTUAL;
        m_uidev.id.version = 1;
        m_uidev.id.vendor = 0x1234;
        m_uidev.id.product = 0x1111;

        err = write(m_fd, &m_uidev, sizeof(m_uidev));
        if (err < 0) { qWarning() << "Unable to write /dev/uinput device"; }

        err = ioctl(m_fd, UI_DEV_CREATE);
        if (err < 0) { qWarning() << "ioctl(UI_DEV_CREATE) error"; }
    }
}

void Mouse_uinput::action(int action_code)
{
    if (m_fd < 0) setup(); // setup?

    // get key
    unsigned keymacro = 0;

    if (action_code == LocalActions::ACTION_MOUSE_click_left) keymacro = BTN_LEFT;
    else if (action_code == LocalActions::ACTION_MOUSE_click_middle) keymacro = BTN_MIDDLE;
    else if (action_code == LocalActions::ACTION_MOUSE_click_right) keymacro = BTN_RIGHT;

    // simulate mouse event
    if (m_fd >= 0)
    {
        emitevent(EV_KEY, keymacro, 1);
        emitevent(EV_SYN, SYN_REPORT, 0);
        //usleep(33000); // 33 ms
        emitevent(EV_KEY, keymacro, 0);
        emitevent(EV_SYN, SYN_REPORT, 0);
        //usleep(33000); // 33 ms
    }
}

void Mouse_uinput::action(int x, int y, int btn_left, int btn_right, int btn_middle)
{
    if (m_fd < 0) setup(); // setup?

    // simulate mouse
    if (m_fd >= 0)
    {
        emitevent(EV_REL, REL_X, x);
        emitevent(EV_REL, REL_Y, y);
        emitevent(EV_KEY, BTN_LEFT, btn_left);
        emitevent(EV_KEY, BTN_MIDDLE, btn_middle);
        emitevent(EV_KEY, BTN_RIGHT, btn_right);
        emitevent(EV_SYN, SYN_REPORT, 0);
    }
}

/* ************************************************************************** */
