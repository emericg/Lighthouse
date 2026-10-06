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

#ifndef INPUT_MONITOR_IDLENOTIFY_H
#define INPUT_MONITOR_IDLENOTIFY_H
/* ************************************************************************** */

#include "InputMonitor.h"

#include <cstdint>

struct wl_display;
struct wl_registry;
struct wl_seat;
struct ext_idle_notifier_v1;
struct ext_idle_notification_v1;

class QSocketNotifier;

/* ************************************************************************** */

/*!
 * \brief Wayland activity backend, using the ext-idle-notify-v1 protocol.
 *
 * The compositor only ever tells us "idle" or "active again", so no input
 * content is accessible, and no special permission is needed.
 * The flip side is that any input counts as activity, pointer included.
 *
 * It runs its own Wayland connection rather than Qt's, so it also works when
 * the application itself uses the xcb platform plugin inside a Wayland session.
 *
 * Supported by KWin, wlroots based compositors, Hyprland, COSMIC...
 * but not by Mutter (GNOME), where the backend reports itself unavailable.
 */
class InputMonitorIdleNotify: public InputMonitorBackend
{
    Q_OBJECT

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    wl_seat *m_seat = nullptr;
    ext_idle_notifier_v1 *m_notifier = nullptr;
    uint32_t m_notifierVersion = 0;
    ext_idle_notification_v1 *m_notification = nullptr;

    QSocketNotifier *m_socketNotifier = nullptr;

    //! Inactivity delay after which the compositor reports us idle
    static constexpr uint32_t s_idleTimeoutMs = 600;

    void teardown();
    void dispatch();

    static void registryGlobal(void *data, wl_registry *registry, uint32_t name,
                               const char *interface, uint32_t version);
    static void registryGlobalRemove(void *data, wl_registry *registry, uint32_t name);

    static void notificationIdled(void *data, ext_idle_notification_v1 *notification);
    static void notificationResumed(void *data, ext_idle_notification_v1 *notification);

public:
    explicit InputMonitorIdleNotify(QObject *parent = nullptr);
    ~InputMonitorIdleNotify() override;

    void start() override;
    void stop() override;

    bool isAvailable() const override { return m_notification != nullptr; }
};

/* ************************************************************************** */
#endif // INPUT_MONITOR_IDLENOTIFY_H
