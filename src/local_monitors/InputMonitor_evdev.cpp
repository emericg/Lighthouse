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

#include "InputMonitor_evdev.h"

#include <cerrno>
#include <climits>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QSocketNotifier>
#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

static constexpr const char *s_inputDir = "/dev/input";

//! Prefix of the uinput devices created by local_controls
static constexpr const char *s_ownDevicePrefix = "Lighthouse virtual";

static constexpr size_t s_bitsPerLong = sizeof(unsigned long) * CHAR_BIT;
static constexpr size_t longsFor(size_t bits) { return (bits + s_bitsPerLong - 1) / s_bitsPerLong; }
static bool testBit(const unsigned long *array, size_t bit)
{
    return (array[bit / s_bitsPerLong] >> (bit % s_bitsPerLong)) & 1UL;
}

/* ************************************************************************** */

InputMonitorEvdev::InputMonitorEvdev(QObject *parent) : InputMonitorBackend(parent)
{
    m_rescanTimer = new QTimer(this);
    m_rescanTimer->setSingleShot(true);
    m_rescanTimer->setInterval(s_rescanDelayMs);
    connect(m_rescanTimer, &QTimer::timeout, this, &InputMonitorEvdev::scan);
}

InputMonitorEvdev::~InputMonitorEvdev()
{
    for (const Device &d : std::as_const(m_devices)) ::close(d.fd);
}

/* ************************************************************************** */

void InputMonitorEvdev::start()
{
    if (m_running) return;
    m_running = true;

    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(QString::fromLatin1(s_inputDir));
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, m_rescanTimer, qOverload<>(&QTimer::start));

    scan();
}

void InputMonitorEvdev::stop()
{
    if (!m_running) return;
    m_running = false;

    m_rescanTimer->stop();
    delete m_watcher;
    m_watcher = nullptr;

    const bool wasAvailable = isAvailable();
    const QStringList paths = m_devices.keys();
    for (const QString &path : paths) closeDevice(path);

    if (wasAvailable) Q_EMIT availableChanged();
}

/* ************************************************************************** */

void InputMonitorEvdev::scan()
{
    if (!m_running) return;

    const bool wasAvailable = isAvailable();

    const QDir dir(QString::fromLatin1(s_inputDir));
    const QStringList nodes = dir.entryList({QStringLiteral("event*")}, QDir::System);

    // forget unplugged devices, in case no read error reported them yet
    const QStringList opened = m_devices.keys();
    for (const QString &path : opened)
    {
        if (!nodes.contains(QFileInfo(path).fileName())) closeDevice(path);
    }

    for (const QString &node : nodes)
    {
        const QString path = dir.absoluteFilePath(node);
        if (!m_devices.contains(path)) openDevice(path);
    }

    if (wasAvailable != isAvailable()) Q_EMIT availableChanged();
}

bool InputMonitorEvdev::openDevice(const QString &path)
{
    const int fd = ::open(QFile::encodeName(path).constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
    {
        if (errno == EACCES && !m_permissionWarned)
        {
            m_permissionWarned = true;
            qWarning() << "InputMonitorEvdev: no read access to" << path
                       << "(add your user to the 'input' group)";
        }
        return false;
    }

    char name[256] = {};
    ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name);

    if (!isKeyboard(fd) || qstrncmp(name, s_ownDevicePrefix, qstrlen(s_ownDevicePrefix)) == 0)
    {
        ::close(fd);
        return false;
    }

    qDebug() << "InputMonitorEvdev: listening to" << path << name;

    Device d;
    d.fd = fd;
    d.notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(d.notifier, &QSocketNotifier::activated, this, [this, path]() { readDevice(path); });
    m_devices.insert(path, d);

    return true;
}

void InputMonitorEvdev::closeDevice(const QString &path)
{
    const auto it = m_devices.constFind(path);
    if (it == m_devices.cend()) return;

    // this can run from within the notifier's own activation
    it->notifier->setEnabled(false);
    it->notifier->deleteLater();
    ::close(it->fd);

    m_devices.erase(it);
}

void InputMonitorEvdev::readDevice(const QString &path)
{
    const auto it = m_devices.constFind(path);
    if (it == m_devices.cend()) return;

    const int fd = it->fd;
    bool pressed = false;
    bool gone = false;

    input_event events[64];
    while (true)
    {
        const ssize_t n = ::read(fd, events, sizeof(events));
        if (n < 0)
        {
            if (errno == EINTR) continue;
            if (errno != EAGAIN) gone = true; // ENODEV once unplugged
            break;
        }
        if (n == 0) break;

        const size_t count = size_t(n) / sizeof(input_event);
        for (size_t i = 0; i < count; i++)
        {
            // value 1 is a press, 0 a release and 2 an autorepeat;
            // codes from BTN_MISC upward are mouse/joystick buttons
            const input_event &ev = events[i];
            if (ev.type == EV_KEY && ev.value == 1 && ev.code < BTN_MISC) pressed = true;
        }
    }

    if (gone)
    {
        closeDevice(path);
        if (!isAvailable()) Q_EMIT availableChanged();
    }

    if (pressed) Q_EMIT keyPressed();
}

/* ************************************************************************** */

bool InputMonitorEvdev::isKeyboard(int fd)
{
    unsigned long evBits[longsFor(EV_MAX + 1)] = {};
    if (ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0) return false;
    if (!testBit(evBits, EV_KEY)) return false;

    unsigned long keyBits[longsFor(KEY_MAX + 1)] = {};
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0) return false;

    // power buttons, lid switches, headset or media remotes only expose a few keys
    return testBit(keyBits, KEY_A) && testBit(keyBits, KEY_Z) && testBit(keyBits, KEY_SPACE);
}

/* ************************************************************************** */
