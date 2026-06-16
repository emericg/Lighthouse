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

#ifndef OS_COMMANDER_LOGIND_H
#define OS_COMMANDER_LOGIND_H
/* ************************************************************************** */

#include "os_commander.h"

/* ************************************************************************** */

/*!
 * OsCommander backend talking to systemd-logind over D-Bus.
 *
 * Suspend / PowerOff go through org.freedesktop.login1.Manager on the system bus.
 * Lock goes through this process' session object (same mechanism as `loginctl lock-session`).
 */
class OsCommander_logind: public OsCommander
{
    Q_OBJECT

public:
    OsCommander_logind(QObject *parent = nullptr) : OsCommander(parent) { }
    ~OsCommander_logind() = default;

    void lock() override;
    void sleep() override;
    void poweroff() override;
};

/* ************************************************************************** */
#endif // OS_COMMANDER_LOGIND_H
