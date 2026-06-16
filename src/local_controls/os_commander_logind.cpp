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

#include "os_commander_logind.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QDebug>

#include <unistd.h> // getpid(), getuid()

/* ************************************************************************** */

static const QString login1Service = QStringLiteral("org.freedesktop.login1");
static const QString login1ManagerPath = QStringLiteral("/org/freedesktop/login1");
static const QString login1ManagerIface = QStringLiteral("org.freedesktop.login1.Manager");
static const QString login1SessionIface = QStringLiteral("org.freedesktop.login1.Session");
static const QString login1UserIface = QStringLiteral("org.freedesktop.login1.User");

/* ************************************************************************** */

/*!
 * Resolve the object path of the session to lock.
 *
 * A GUI app started from the desktop lives in an app-*.scope, not a
 * session-N.scope, so GetSessionByPID() usually fails for us. We instead ask
 * logind for our user's primary graphical session (User.Display), which is
 * exactly the session a screen lock should target. GetSessionByPID() is kept
 * as a fallback for the cases where we *are* in a session scope.
 */
static QString resolveSessionPath()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager(login1Service, login1ManagerPath, login1ManagerIface, bus);

    // primary path: this user's graphical (Display) session
    QDBusReply<QDBusObjectPath> user = manager.call("GetUser", (uint)getuid());
    if (user.isValid())
    {
        // User.Display is a (so) struct: { session-id, object-path }
        QDBusInterface props(login1Service, user.value().path(), QStringLiteral("org.freedesktop.DBus.Properties"), bus);
        QDBusReply<QDBusVariant> display = props.call("Get", login1UserIface, QStringLiteral("Display"));
        if (display.isValid())
        {
            const QDBusArgument arg = display.value().variant().value<QDBusArgument>();
            QString sessionId;
            QDBusObjectPath sessionPath;
            arg.beginStructure();
            arg >> sessionId >> sessionPath;
            arg.endStructure();

            if (sessionPath.path().length() > 1)
            {
                // "/" means "no display session"
                return sessionPath.path();
            }
        }
    }

    // fallback: our own session, when we happen to be inside a session scope
    QDBusReply<QDBusObjectPath> session = manager.call("GetSessionByPID", (uint)getpid());
    if (session.isValid()) return session.value().path();

    qWarning() << "OsCommander_logind: no lockable session"
               << "(GetUser:" << user.error().message()
               << "GetSessionByPID:" << session.error().message() << ")";

    return QString();
}

/* ************************************************************************** */

void OsCommander_logind::lock()
{
    // logind has no global "lock", so we resolve our graphical session and lock
    // it (same mechanism as `loginctl lock-session`, which any modern DE honors)
    const QString sessionPath = resolveSessionPath();
    if (sessionPath.isEmpty()) return;

    QDBusInterface s(login1Service, sessionPath, login1SessionIface, QDBusConnection::systemBus());

    QDBusReply<void> reply = s.call("Lock");
    if (!reply.isValid())
    {
        qWarning() << "OsCommander_logind::lock() failed:" << reply.error().message();
    }
}

void OsCommander_logind::sleep()
{
    QDBusInterface manager(login1Service, login1ManagerPath, login1ManagerIface, QDBusConnection::systemBus());

    // false: non-interactive, don't pop a polkit prompt
    QDBusReply<void> reply = manager.call("Suspend", false);
    if (!reply.isValid())
    {
        qWarning() << "OsCommander_logind::sleep() failed:" << reply.error().message();
    }
}

void OsCommander_logind::poweroff()
{
    QDBusInterface manager(login1Service, login1ManagerPath, login1ManagerIface, QDBusConnection::systemBus());

    // false: non-interactive, don't pop a polkit prompt
    QDBusReply<void> reply = manager.call("PowerOff", false);
    if (!reply.isValid())
    {
        qWarning() << "OsCommander_logind::poweroff() failed:" << reply.error().message();
    }
}

/* ************************************************************************** */
