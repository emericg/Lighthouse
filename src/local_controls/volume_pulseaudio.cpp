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
 * \date      2025
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "volume_pulseaudio.h"

#include <QtMath>
#include <QDebug>

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusArgument>
#include <QDBusObjectPath>
#endif

/* ************************************************************************** */

// PulseAudio D-Bus addressing
static const QString paServerLookupService = QStringLiteral("org.PulseAudio1");
static const QString paServerLookupPath = QStringLiteral("/org/pulseaudio/server_lookup1");
static const QString paServerLookupIface = QStringLiteral("org.PulseAudio.ServerLookup1");

static const QString paCorePath = QStringLiteral("/org/pulseaudio/core1");
static const QString paCoreIface = QStringLiteral("org.PulseAudio.Core1");
static const QString paDeviceIface = QStringLiteral("org.PulseAudio.Core1.Device");
static const QString freedesktopProperties = QStringLiteral("org.freedesktop.DBus.Properties");

static const QString paConnectionName = QStringLiteral("lighthouse_pulse");

// PA_VOLUME_NORM: the raw value matching 100% volume
static const quint32 paVolumeNorm = 0x10000; // 65536

/* ************************************************************************** */

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

//! Read a D-Bus property (org.freedesktop.DBus.Properties.Get) over a peer connection.
static QVariant paGetProperty(const QDBusConnection &bus, const QString &path,
                              const QString &iface, const QString &prop)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QString(), path,
                                                      freedesktopProperties, QStringLiteral("Get"));
    msg << iface << prop;

    QDBusMessage reply = bus.call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty())
    {
        return QVariant();
    }
    return reply.arguments().constFirst().value<QDBusVariant>().variant();
}

//! Write a D-Bus property (org.freedesktop.DBus.Properties.Set) over a peer connection.
static bool paSetProperty(const QDBusConnection &bus, const QString &path,
                          const QString &iface, const QString &prop, const QDBusVariant &value)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QString(), path,
                                                      freedesktopProperties, QStringLiteral("Set"));
    msg << iface << prop << QVariant::fromValue(value);

    QDBusMessage reply = bus.call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        qWarning() << "Volume_pulseaudio: failed to set" << prop << ":" << reply.errorMessage();
        return false;
    }
    return true;
}

//! Demarshal a PulseAudio 'au' volume array (one raw uint32 per channel).
static QList<quint32> paParseVolumeArray(const QVariant &v)
{
    QList<quint32> out;
    if (v.canConvert<QDBusArgument>())
    {
        const QDBusArgument arg = v.value<QDBusArgument>();
        arg.beginArray();
        while (!arg.atEnd())
        {
            quint32 x = 0;
            arg >> x;
            out.append(x);
        }
        arg.endArray();
    }
    return out;
}

//! Ask PulseAudio to start emitting a given signal towards us.
static void paListenForSignal(const QDBusConnection &bus, const QString &signal)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QString(), paCorePath,
                                                      paCoreIface, QStringLiteral("ListenForSignal"));
    // empty 'ao' object filter == listen on all objects
    QDBusArgument objs;
    objs.beginArray(qMetaTypeId<QDBusObjectPath>());
    objs.endArray();

    msg << signal << QVariant::fromValue(objs);
    bus.call(msg);
}

#endif // Q_OS_LINUX

/* ************************************************************************** */

Volume_pulseaudio::Volume_pulseaudio(QObject *parent) : Volume(parent)
{
    setup();
}

Volume_pulseaudio::~Volume_pulseaudio()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (!m_connName.isEmpty())
    {
        QDBusConnection::disconnectFromPeer(m_connName);
    }
#endif
}

/* ************************************************************************** */

void Volume_pulseaudio::setup()
{
    //qDebug() << "Volume_pulseaudio::setup()";

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (!connectToServer())
    {
        qWarning() << "Volume_pulseaudio: PulseAudio D-Bus API unavailable (is module-dbus-protocol loaded?)";
        m_available = false;
        Q_EMIT availabilityChanged();
        return;
    }

    if (!selectFallbackSink())
    {
        qWarning() << "Volume_pulseaudio: no usable sink found";
        m_available = false;
        Q_EMIT availabilityChanged();
        return;
    }

    subscribe();

    m_available = true;
    Q_EMIT availabilityChanged();
    Q_EMIT volumeChanged();
    Q_EMIT muteChanged();

    qDebug() << "Volume_pulseaudio: ready, sink" << m_sinkPath
             << "volume" << m_volume << "muted" << m_mute;
#endif
}

/* ************************************************************************** */

bool Volume_pulseaudio::connectToServer()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // Ask the well-known service (on the session bus) for the PulseAudio bus address
    QDBusInterface lookup(paServerLookupService, paServerLookupPath, paServerLookupIface,
                          QDBusConnection::sessionBus());
    if (!lookup.isValid())
    {
        return false;
    }

    const QString address = lookup.property("Address").toString();
    if (address.isEmpty())
    {
        return false;
    }

    // Drop a stale connection of the same name, then open a fresh peer connection
    QDBusConnection::disconnectFromPeer(paConnectionName);
    QDBusConnection bus = QDBusConnection::connectToPeer(address, paConnectionName);
    if (!bus.isConnected())
    {
        QDBusConnection::disconnectFromPeer(paConnectionName);
        return false;
    }

    m_connName = paConnectionName;
    return true;
#else
    return false;
#endif
}

bool Volume_pulseaudio::selectFallbackSink()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QDBusConnection bus(m_connName);

    // Prefer the server's fallback (default) sink
    QString sink;
    QVariant fb = paGetProperty(bus, paCorePath, paCoreIface, QStringLiteral("FallbackSink"));
    if (fb.isValid())
    {
        sink = fb.value<QDBusObjectPath>().path();
    }

    // Otherwise fall back to the first available sink
    if (sink.isEmpty())
    {
        QVariant sinksVar = paGetProperty(bus, paCorePath, paCoreIface, QStringLiteral("Sinks"));
        if (sinksVar.canConvert<QDBusArgument>())
        {
            const QDBusArgument arg = sinksVar.value<QDBusArgument>();
            arg.beginArray();
            if (!arg.atEnd())
            {
                QDBusObjectPath p;
                arg >> p;
                sink = p.path();
            }
            arg.endArray();
        }
    }

    if (sink.isEmpty())
    {
        return false;
    }

    m_sinkPath = sink;
    readSinkVolume();
    readSinkMute();
    return true;
#else
    return false;
#endif
}

bool Volume_pulseaudio::readSinkVolume()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (m_sinkPath.isEmpty()) return false;

    QDBusConnection bus(m_connName);
    QList<quint32> vols = paParseVolumeArray(paGetProperty(bus, m_sinkPath, paDeviceIface,
                                                           QStringLiteral("Volume")));
    if (vols.isEmpty()) return false;

    m_channels = vols.size();

    // represent the sink with its loudest channel
    quint32 raw = 0;
    for (quint32 v : vols) raw = qMax(raw, v);

    m_volume = static_cast<float>(raw) / static_cast<float>(paVolumeNorm);
    return true;
#else
    return false;
#endif
}

bool Volume_pulseaudio::readSinkMute()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (m_sinkPath.isEmpty()) return false;

    QDBusConnection bus(m_connName);
    QVariant m = paGetProperty(bus, m_sinkPath, paDeviceIface, QStringLiteral("Mute"));
    if (!m.isValid()) return false;

    m_mute = m.toBool();
    return true;
#else
    return false;
#endif
}

void Volume_pulseaudio::subscribe()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QDBusConnection bus(m_connName);

    paListenForSignal(bus, paDeviceIface + QStringLiteral(".VolumeUpdated"));
    paListenForSignal(bus, paDeviceIface + QStringLiteral(".MuteUpdated"));
    paListenForSignal(bus, paCoreIface + QStringLiteral(".FallbackSinkUpdated"));

    // empty path == match any device (we always re-read our current sink)
    bus.connect(QString(), QString(), paDeviceIface, QStringLiteral("VolumeUpdated"),
                this, SLOT(onSinkChanged()));
    bus.connect(QString(), QString(), paDeviceIface, QStringLiteral("MuteUpdated"),
                this, SLOT(onSinkChanged()));
    bus.connect(QString(), paCorePath, paCoreIface, QStringLiteral("FallbackSinkUpdated"),
                this, SLOT(onFallbackSinkChanged()));
#endif
}

/* ************************************************************************** */

void Volume_pulseaudio::onSinkChanged()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    float prevVol = m_volume;
    bool prevMute = m_mute;

    readSinkVolume();
    readSinkMute();

    if (!qFuzzyCompare(prevVol + 1.f, m_volume + 1.f)) Q_EMIT volumeChanged();
    if (prevMute != m_mute) Q_EMIT muteChanged();
#endif
}

void Volume_pulseaudio::onFallbackSinkChanged()
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (selectFallbackSink())
    {
        Q_EMIT volumeChanged();
        Q_EMIT muteChanged();
    }
#endif
}

/* ************************************************************************** */

void Volume_pulseaudio::setVolume(float volume)
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (!m_available || m_sinkPath.isEmpty()) return;

    volume = qBound(0.f, volume, m_volumeLimit);

    const quint32 raw = static_cast<quint32>(qRound(volume * static_cast<float>(paVolumeNorm)));

    QDBusArgument arg;
    arg.beginArray(QMetaType::UInt);
    for (int i = 0; i < qMax(1, m_channels); ++i) arg << raw;
    arg.endArray();

    QDBusConnection bus(m_connName);
    if (paSetProperty(bus, m_sinkPath, paDeviceIface, QStringLiteral("Volume"),
                      QDBusVariant(QVariant::fromValue(arg))))
    {
        if (!qFuzzyCompare(m_volume + 1.f, volume + 1.f))
        {
            m_volume = volume;
            Q_EMIT volumeChanged();
        }
    }
#else
    Q_UNUSED(volume)
#endif
}

void Volume_pulseaudio::setMute(bool mute)
{
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    if (!m_available || m_sinkPath.isEmpty()) return;

    QDBusConnection bus(m_connName);
    if (paSetProperty(bus, m_sinkPath, paDeviceIface, QStringLiteral("Mute"),
                      QDBusVariant(QVariant(mute))))
    {
        if (m_mute != mute)
        {
            m_mute = mute;
            Q_EMIT muteChanged();
        }
    }
#else
    Q_UNUSED(mute)
#endif
}

/* ************************************************************************** */
