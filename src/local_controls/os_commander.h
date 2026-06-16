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

#ifndef OS_COMMANDER_H
#define OS_COMMANDER_H
/* ************************************************************************** */

#include <QObject>

/* ************************************************************************** */

/*!
 * Minimal API to drive OS session/power actions (lock, sleep, power off).
 *
 * Unlike the virtual keyboard, these don't rely on synthetic key events beingpicked up downstream.
 * Backends talk to the system session manager directly.
 */
class OsCommander: public QObject
{
    Q_OBJECT

public:
    OsCommander(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~OsCommander() = default;

    virtual void lock() = 0;
    virtual void sleep() = 0;
    virtual void poweroff() = 0;
};

/* ************************************************************************** */
#endif // OS_COMMANDER_H
