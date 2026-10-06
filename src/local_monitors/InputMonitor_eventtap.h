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

#ifndef INPUT_MONITOR_EVENTTAP_H
#define INPUT_MONITOR_EVENTTAP_H
/* ************************************************************************** */

#include "InputMonitor.h"

#include <CoreGraphics/CoreGraphics.h>

class QTimer;

/* ************************************************************************** */

/*!
 * \brief macOS keyboard activity backend, using a listen only Quartz event tap.
 *
 * The tap only observes key down events, it never alters or delays them.
 * Requires the "Input Monitoring" permission (Privacy & Security settings),
 * which macOS asks for the first time the backend starts.
 *
 * While the permission is missing the backend reports itself unavailable,
 * and checks back periodically so it can start as soon as it is granted.
 *
 * Events posted by this very process are skipped, or keys sent from a client
 * would be reflected back to it as local typing.
 */
class InputMonitorEventTap: public InputMonitorBackend
{
    Q_OBJECT

    CFMachPortRef m_tap = nullptr;
    CFRunLoopSourceRef m_source = nullptr;

    QTimer *m_retryTimer = nullptr;

    bool m_running = false;
    bool m_permissionRequested = false;

    //! Delay between two attempts at creating the tap, while the permission is missing
    static constexpr int s_retryDelayMs = 2000;

    //! Create the tap, if the permission allows it
    void setup();
    void teardown();

    static CGEventRef tapCallback(CGEventTapProxy proxy, CGEventType type,
                                  CGEventRef event, void *userInfo);

public:
    explicit InputMonitorEventTap(QObject *parent = nullptr);
    ~InputMonitorEventTap() override;

    void start() override;
    void stop() override;

    bool isAvailable() const override { return m_tap != nullptr; }
};

/* ************************************************************************** */
#endif // INPUT_MONITOR_EVENTTAP_H
