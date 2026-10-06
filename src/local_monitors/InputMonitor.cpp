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

#include "InputMonitor.h"

#if defined(ENABLE_INPUT_EVDEV)
#include "InputMonitor_evdev.h"
#elif defined(ENABLE_INPUT_IDLENOTIFY)
#include "InputMonitor_idlenotify.h"
#elif defined(ENABLE_INPUT_EVENTTAP)
#include "InputMonitor_eventtap.h"
#endif

#include <QCoreApplication>
#include <QTimer>
#include <QQmlEngine>
#include <QJSEngine>

/* ************************************************************************** */

InputMonitor *InputMonitor::getInstance()
{
    static InputMonitor *instance = new InputMonitor(QCoreApplication::instance());
    return instance;
}

InputMonitor *InputMonitor::create(QQmlEngine *, QJSEngine *)
{
    InputMonitor *instance = getInstance();
    QJSEngine::setObjectOwnership(instance, QJSEngine::CppOwnership);
    return instance;
}

InputMonitor::Backend InputMonitor::getBackend()
{
#if defined(ENABLE_INPUT_EVDEV)
    return BackendEvdev;
#elif defined(ENABLE_INPUT_IDLENOTIFY)
    return BackendIdleNotify;
#elif defined(ENABLE_INPUT_EVENTTAP)
    return BackendEventTap;
#else
    return BackendNone;
#endif
}

InputMonitor::InputMonitor(QObject *parent) : QObject(parent)
{
#if defined(ENABLE_INPUT_EVDEV)
    m_backend = new InputMonitorEvdev(this);
#elif defined(ENABLE_INPUT_IDLENOTIFY)
    m_backend = new InputMonitorIdleNotify(this);
#elif defined(ENABLE_INPUT_EVENTTAP)
    m_backend = new InputMonitorEventTap(this);
#endif

    if (!m_backend) return;

    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(s_idleTimeoutMs);
    connect(m_idleTimer, &QTimer::timeout, this, [this]() { setTyping(false); });

    connect(m_backend, &InputMonitorBackend::keyPressed, this, &InputMonitor::onKeyPressed);
    connect(m_backend, &InputMonitorBackend::activityChanged, this, [this](bool active) {
        m_idleTimer->stop();
        setTyping(active);
    });
    connect(m_backend, &InputMonitorBackend::availableChanged, this, &InputMonitor::availableChanged);
}

/* ************************************************************************** */

bool InputMonitor::isAvailable() const
{
    return m_backend && m_backend->isAvailable();
}

void InputMonitor::setActive(bool active)
{
    if (!m_backend || m_active == active) return;
    m_active = active;

    if (m_active)
    {
        m_backend->start();
    }
    else
    {
        m_backend->stop();
        m_idleTimer->stop();
        setTyping(false);
    }
}

void InputMonitor::setTyping(bool typing)
{
    if (m_typing == typing) return;
    m_typing = typing;
    Q_EMIT typingChanged();
}

void InputMonitor::onKeyPressed()
{
    m_idleTimer->start();
    setTyping(true);
}

/* ************************************************************************** */
