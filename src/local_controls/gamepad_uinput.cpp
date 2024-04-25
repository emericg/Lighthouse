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

#include "gamepad_uinput.h"
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

Gamepad_uinput::Gamepad_uinput(QObject *parent): Gamepad(parent)
{
    //
}

Gamepad_uinput::~Gamepad_uinput()
{
    if (m_fd >= 0)
    {
        // close virtual gamepad
        if (ioctl(m_fd, UI_DEV_DESTROY) < 0)
        {
            qWarning() << "Cannot close /dev/uinput";
        }
        close(m_fd);
    }
}

/* ************************************************************************** */

void Gamepad_uinput::emitevent(int type, int code, int val)
{
    if (m_fd)
    {
        struct input_event event;
        memset(&event, 0, sizeof(event));

        event.type = type;
        event.code = code;
        event.value = val;

        // set timestamps
        gettimeofday(&event.time, NULL);

        // ignore timestamps?
        //event.time.tv_sec = 0;
        //event.time.tv_usec = 0;

        write(m_fd, &event, sizeof(event));
    }
}

/* ************************************************************************** */

void Gamepad_uinput::setup()
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

        // enable axis events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_ABS);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_ABS) error"; }

        // enable keys events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_KEY) error"; }

        // enable syn events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_SYN);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_SYN) error"; }

        // setup buttons and axis
        ioctl(m_fd, UI_SET_ABSBIT, ABS_X);
        ioctl(m_fd, UI_SET_ABSBIT, ABS_Y);
        ioctl(m_fd, UI_SET_ABSBIT, ABS_RX);
        ioctl(m_fd, UI_SET_ABSBIT, ABS_RY);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_A);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_B);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_X);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_Y);

        // init virtual keyboard
        memset(&m_uidev, 0, sizeof(m_uidev));
        snprintf(m_uidev.name, UINPUT_MAX_NAME_SIZE, "Lighthouse virtual gamepad");
        m_uidev.id.bustype = BUS_VIRTUAL;
        m_uidev.id.version = 1;
        m_uidev.id.vendor = 0x1234;
        m_uidev.id.product = 0x3333;

        err = write(m_fd, &m_uidev, sizeof(m_uidev));
        if (err < 0) { qWarning() << "Unable to write /dev/uinput device"; }

        err = ioctl(m_fd, UI_DEV_CREATE);
        if (err < 0) { qWarning() << "ioctl(UI_DEV_CREATE) error"; }
    }
}

void Gamepad_uinput::action(int x1, int y1, int x2, int y2,
                            int a, int b, int x, int y)
{
    if (m_fd < 0) setup(); // setup?

    if (m_fd >= 0)
    {
        // first axis
        emitevent(EV_ABS, ABS_X, x1);
        emitevent(EV_ABS, ABS_Y, y1);
        emitevent(EV_SYN, SYN_REPORT, 0);

        // second axis
        emitevent(EV_ABS, ABS_RX, x2);
        emitevent(EV_ABS, ABS_RY, y2);
        emitevent(EV_SYN, SYN_REPORT, 0);

        emitevent(EV_KEY, BTN_A, a);
        emitevent(EV_KEY, BTN_B, b);
        emitevent(EV_KEY, BTN_X, x);
        emitevent(EV_KEY, BTN_Y, y);
        emitevent(EV_SYN, SYN_REPORT, 0);
    }
}

/* ************************************************************************** */

void Gamepad_uinput::setup_pbp()
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

        // enable axis events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_ABS);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_ABS) error"; }

        // enable key events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_KEY) error"; }

        // enable syn events
        err = ioctl(m_fd, UI_SET_EVBIT, EV_SYN);
        if (err < 0) { qWarning() << "ioctl(UI_SET_EVBIT, EV_SYN) error"; }

        // setup buttons and axis
        ioctl(m_fd, UI_SET_ABSBIT, ABS_X);
        ioctl(m_fd, UI_SET_ABSBIT, ABS_Y);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_A);
        ioctl(m_fd, UI_SET_KEYBIT, BTN_B);

        // init virtual keyboard
        memset(&m_uidev, 0, sizeof(m_uidev));
        snprintf(m_uidev.name, UINPUT_MAX_NAME_SIZE, "Pokéball Plus");
        m_uidev.id.bustype = BUS_BLUETOOTH;
        m_uidev.id.version = 1;
        m_uidev.id.vendor = 0x057E;
        m_uidev.id.product = 0x9876;

        err = write(m_fd, &m_uidev, sizeof(m_uidev));
        if (err < 0) { qWarning() << "Unable to write /dev/uinput device"; }

        err = ioctl(m_fd, UI_DEV_CREATE);
        if (err < 0) { qWarning() << "ioctl(UI_DEV_CREATE) error"; }
    }
}

void Gamepad_uinput::action_pbp(int x, int y, int a, int b)
{
    if (m_fd < 0) setup_pbp(); // setup?

    if (m_fd >= 0)
    {
        emitevent(EV_ABS, ABS_X, x);
        emitevent(EV_ABS, ABS_Y, y);
        emitevent(EV_KEY, BTN_A, a);
        emitevent(EV_KEY, BTN_B, b);

        emitevent(EV_SYN, SYN_REPORT, 0);
    }
}

/* ************************************************************************** */
