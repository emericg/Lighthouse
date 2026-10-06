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

#ifndef INPUT_MONITOR_EVDEV_H
#define INPUT_MONITOR_EVDEV_H
/* ************************************************************************** */

#include "InputMonitor.h"

#include <QHash>
#include <QString>

class QSocketNotifier;
class QFileSystemWatcher;
class QTimer;

/* ************************************************************************** */

/*!
 * \brief Linux keyboard activity backend, reading the evdev nodes directly.
 *
 * Works the same under Wayland and X11, as it sits below the display server.
 * Requires read access to /dev/input/event*, usually through the "input" group.
 *
 * Devices are read without being grabbed, so input keeps flowing normally.
 * Our own uinput virtual keyboard is skipped, or keys sent from a client
 * would be reflected back to it as local typing.
 */
class InputMonitorEvdev: public InputMonitorBackend
{
    Q_OBJECT

    struct Device
    {
        int fd = -1;
        QSocketNotifier *notifier = nullptr;
    };

    QHash <QString, Device> m_devices;      //!< opened keyboards, by node path

    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_rescanTimer = nullptr;

    bool m_running = false;
    bool m_permissionWarned = false;

    //! Delay between a /dev/input change and the rescan, so udev can set the node permissions
    static constexpr int s_rescanDelayMs = 1000;

    void scan();

    /*!
     * \brief Open a node and keep it only if it is a keyboard we should listen to.
     * \return True if the node is now being listened to.
     */
    bool openDevice(const QString &path);
    void closeDevice(const QString &path);
    void readDevice(const QString &path);

    //! Whether an evdev node reports the alphanumeric keys of an actual keyboard
    static bool isKeyboard(int fd);

public:
    explicit InputMonitorEvdev(QObject *parent = nullptr);
    ~InputMonitorEvdev() override;

    void start() override;
    void stop() override;

    bool isAvailable() const override { return !m_devices.isEmpty(); }
};

/* ************************************************************************** */
#endif // INPUT_MONITOR_EVDEV_H
