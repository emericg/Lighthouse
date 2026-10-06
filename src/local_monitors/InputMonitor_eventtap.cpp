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

#include "InputMonitor_eventtap.h"

#include <unistd.h>

#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

InputMonitorEventTap::InputMonitorEventTap(QObject *parent) : InputMonitorBackend(parent)
{
    m_retryTimer = new QTimer(this);
    m_retryTimer->setInterval(s_retryDelayMs);
    connect(m_retryTimer, &QTimer::timeout, this, &InputMonitorEventTap::setup);
}

InputMonitorEventTap::~InputMonitorEventTap()
{
    teardown();
}

/* ************************************************************************** */

void InputMonitorEventTap::start()
{
    if (m_running) return;
    m_running = true;

    setup();
}

void InputMonitorEventTap::stop()
{
    if (!m_running) return;
    m_running = false;

    m_retryTimer->stop();

    const bool wasAvailable = isAvailable();
    teardown();
    if (wasAvailable) Q_EMIT availableChanged();
}

/* ************************************************************************** */

void InputMonitorEventTap::setup()
{
    if (!m_running || m_tap) return;

    if (!CGPreflightListenEventAccess())
    {
        // macOS only shows the prompt once, then the user has to go to the settings
        if (!m_permissionRequested)
        {
            m_permissionRequested = true;
            qWarning() << "InputMonitorEventTap: missing the Input Monitoring permission"
                       << "(System Settings > Privacy & Security > Input Monitoring)";
            CGRequestListenEventAccess();
        }

        m_retryTimer->start();
        return;
    }

    m_tap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap,
                             kCGEventTapOptionListenOnly,
                             CGEventMaskBit(kCGEventKeyDown),
                             &InputMonitorEventTap::tapCallback, this);
    if (!m_tap)
    {
        // the permission can be granted, yet only effective once the application restarts
        qWarning() << "InputMonitorEventTap: unable to create the event tap";
        m_retryTimer->start();
        return;
    }

    m_retryTimer->stop();

    // Qt's Cocoa event dispatcher runs the main run loop, so the callback lands on the main thread
    m_source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, m_tap, 0);
    CFRunLoopAddSource(CFRunLoopGetMain(), m_source, kCFRunLoopCommonModes);
    CGEventTapEnable(m_tap, true);

    qDebug() << "InputMonitorEventTap: listening to the keyboard";

    Q_EMIT availableChanged();
}

void InputMonitorEventTap::teardown()
{
    if (m_tap) CGEventTapEnable(m_tap, false);

    if (m_source)
    {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), m_source, kCFRunLoopCommonModes);
        CFRelease(m_source);
    }
    if (m_tap)
    {
        CFMachPortInvalidate(m_tap);
        CFRelease(m_tap);
    }

    m_source = nullptr;
    m_tap = nullptr;
}

/* ************************************************************************** */

CGEventRef InputMonitorEventTap::tapCallback(CGEventTapProxy, CGEventType type,
                                             CGEventRef event, void *userInfo)
{
    auto *self = static_cast<InputMonitorEventTap *>(userInfo);

    // the window server disables taps it deems too slow, or on secure input transitions
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput)
    {
        if (self->m_tap) CGEventTapEnable(self->m_tap, true);
        return event;
    }

    if (type != kCGEventKeyDown) return event;

    // autorepeats are not new key presses
    if (CGEventGetIntegerValueField(event, kCGKeyboardEventAutorepeat)) return event;

    // keys injected by local_controls
    if (CGEventGetIntegerValueField(event, kCGEventSourceUnixProcessID) == getpid()) return event;

    Q_EMIT self->keyPressed();

    return event;
}

/* ************************************************************************** */
