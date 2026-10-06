/*!
 * This file is part of Lighthouse.
 * Copyright (c) 2022 Emeric Grange - All Rights Reserved
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

#include "InputMonitor_idlenotify.h"

#include <algorithm>
#include <cstring>

#include <wayland-client.h>
#include "ext-idle-notify-v1-client-protocol.h"

#include <QSocketNotifier>
#include <QDebug>

/* ************************************************************************** */

InputMonitorIdleNotify::InputMonitorIdleNotify(QObject *parent) : InputMonitorBackend(parent)
{
    //
}

InputMonitorIdleNotify::~InputMonitorIdleNotify()
{
    teardown();
}

/* ************************************************************************** */

void InputMonitorIdleNotify::start()
{
    if (m_display) return;

    static const wl_registry_listener s_registryListener = {
        &InputMonitorIdleNotify::registryGlobal,
        &InputMonitorIdleNotify::registryGlobalRemove,
    };
    static const ext_idle_notification_v1_listener s_notificationListener = {
        &InputMonitorIdleNotify::notificationIdled,
        &InputMonitorIdleNotify::notificationResumed,
    };

    m_display = wl_display_connect(nullptr);
    if (!m_display)
    {
        qWarning() << "InputMonitorIdleNotify: no Wayland session";
        return;
    }

    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &s_registryListener, this);
    wl_display_roundtrip(m_display);

    if (!m_notifier || !m_seat)
    {
        qWarning() << "InputMonitorIdleNotify: the compositor does not support ext-idle-notify-v1";
        teardown();
        return;
    }

#ifdef EXT_IDLE_NOTIFIER_V1_GET_INPUT_IDLE_NOTIFICATION_SINCE_VERSION
    // the input variant ignores idle inhibitors, or a playing video would keep us "active" forever
    if (m_notifierVersion >= EXT_IDLE_NOTIFIER_V1_GET_INPUT_IDLE_NOTIFICATION_SINCE_VERSION)
        m_notification = ext_idle_notifier_v1_get_input_idle_notification(m_notifier, s_idleTimeoutMs, m_seat);
    else
#endif
        m_notification = ext_idle_notifier_v1_get_idle_notification(m_notifier, s_idleTimeoutMs, m_seat);

    ext_idle_notification_v1_add_listener(m_notification, &s_notificationListener, this);
    wl_display_flush(m_display);

    m_socketNotifier = new QSocketNotifier(wl_display_get_fd(m_display), QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &InputMonitorIdleNotify::dispatch);

    Q_EMIT availableChanged();
}

void InputMonitorIdleNotify::stop()
{
    const bool wasAvailable = isAvailable();
    teardown();
    if (wasAvailable) Q_EMIT availableChanged();
}

void InputMonitorIdleNotify::teardown()
{
    if (m_socketNotifier)
    {
        // this can run from within the notifier's own activation
        m_socketNotifier->setEnabled(false);
        m_socketNotifier->deleteLater();
        m_socketNotifier = nullptr;
    }

    if (m_notification) ext_idle_notification_v1_destroy(m_notification);
    if (m_notifier) ext_idle_notifier_v1_destroy(m_notifier);
    if (m_seat) wl_seat_destroy(m_seat);
    if (m_registry) wl_registry_destroy(m_registry);
    if (m_display) wl_display_disconnect(m_display);

    m_notification = nullptr;
    m_notifier = nullptr;
    m_notifierVersion = 0;
    m_seat = nullptr;
    m_registry = nullptr;
    m_display = nullptr;
}

void InputMonitorIdleNotify::dispatch()
{
    if (!m_display) return;

    // the socket is readable, so this reads the pending events without blocking
    if (wl_display_dispatch(m_display) < 0)
    {
        qWarning() << "InputMonitorIdleNotify: lost the Wayland connection";
        stop();
        return;
    }

    wl_display_flush(m_display);
}

/* ************************************************************************** */

void InputMonitorIdleNotify::registryGlobal(void *data, wl_registry *registry, uint32_t name,
                                            const char *interface, uint32_t version)
{
    auto *self = static_cast<InputMonitorIdleNotify *>(data);

    if (std::strcmp(interface, wl_seat_interface.name) == 0)
    {
        // activity is tracked on the first seat only, which is all a desktop usually has
        if (!self->m_seat)
            self->m_seat = static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
    }
    else if (std::strcmp(interface, ext_idle_notifier_v1_interface.name) == 0)
    {
        self->m_notifierVersion = std::min<uint32_t>(version, ext_idle_notifier_v1_interface.version);
        self->m_notifier = static_cast<ext_idle_notifier_v1 *>(
            wl_registry_bind(registry, name, &ext_idle_notifier_v1_interface, self->m_notifierVersion));
    }
}

void InputMonitorIdleNotify::registryGlobalRemove(void *, wl_registry *, uint32_t)
{
    //
}

void InputMonitorIdleNotify::notificationIdled(void *data, ext_idle_notification_v1 *)
{
    Q_EMIT static_cast<InputMonitorIdleNotify *>(data)->activityChanged(false);
}

void InputMonitorIdleNotify::notificationResumed(void *data, ext_idle_notification_v1 *)
{
    Q_EMIT static_cast<InputMonitorIdleNotify *>(data)->activityChanged(true);
}

/* ************************************************************************** */
