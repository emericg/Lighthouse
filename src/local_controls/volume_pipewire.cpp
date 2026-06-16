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

#include "volume_pipewire.h"

#include <QProcess>
#include <QTimer>
#include <QStandardPaths>
#include <QtMath>
#include <QDebug>

/* ************************************************************************** */

static const int paPollIntervalMs = 1000;

//! Run wpctl and return its standard output (empty on failure).
static QString runWpctlCapture(const QString &wpctl, const QStringList &args)
{
    QProcess proc;
    proc.start(wpctl, args);
    if (!proc.waitForFinished(2000) || proc.exitStatus() != QProcess::NormalExit)
    {
        return QString();
    }
    return QString::fromUtf8(proc.readAllStandardOutput());
}

//! Run wpctl, discarding output, returning success.
static bool runWpctl(const QString &wpctl, const QStringList &args)
{
    QProcess proc;
    proc.start(wpctl, args);
    if (!proc.waitForFinished(2000) || proc.exitStatus() != QProcess::NormalExit)
    {
        return false;
    }
    return (proc.exitCode() == 0);
}

/* ************************************************************************** */

Volume_pipewire::Volume_pipewire(QObject *parent) : Volume(parent)
{
    setup();
}

Volume_pipewire::~Volume_pipewire()
{
    //
}

/* ************************************************************************** */

void Volume_pipewire::setup()
{
    //qDebug() << "Volume_pipewire::setup()";

    m_wpctl = QStandardPaths::findExecutable(QStringLiteral("wpctl"));
    if (m_wpctl.isEmpty())
    {
        qWarning() << "Volume_pipewire: 'wpctl' not found in PATH";
        m_available = false;
        Q_EMIT availabilityChanged();
        return;
    }

    if (!queryState())
    {
        qWarning() << "Volume_pipewire: could not read volume from wpctl";
        m_available = false;
        Q_EMIT availabilityChanged();
        return;
    }

    m_available = true;
    Q_EMIT availabilityChanged();
    Q_EMIT volumeChanged();
    Q_EMIT muteChanged();

    // WirePlumber has no notification stream we can subscribe to: poll instead
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(paPollIntervalMs);
    connect(m_pollTimer, &QTimer::timeout, this, &Volume_pipewire::poll);
    m_pollTimer->start();

    qDebug() << "Volume_pipewire: ready, volume" << m_volume << "muted" << m_mute;
}

/* ************************************************************************** */

bool Volume_pipewire::queryState()
{
    // "Volume: 0.50" or "Volume: 0.50 [MUTED]"
    const QString out = runWpctlCapture(m_wpctl, {QStringLiteral("get-volume"), m_target});
    if (out.isEmpty()) return false;

    const int idx = out.indexOf(QStringLiteral("Volume:"));
    if (idx < 0) return false;

    const QStringList parts = out.mid(idx + 7).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return false;

    bool ok = false;
    const float v = parts.constFirst().toFloat(&ok);
    if (!ok) return false;

    m_volume = qBound(0.f, v, 1.f);
    m_mute = out.contains(QStringLiteral("[MUTED]"));
    return true;
}

void Volume_pipewire::poll()
{
    const float prevVol = m_volume;
    const bool prevMute = m_mute;

    if (!queryState()) return;

    if (!qFuzzyCompare(prevVol + 1.f, m_volume + 1.f)) Q_EMIT volumeChanged();
    if (prevMute != m_mute) Q_EMIT muteChanged();
}

/* ************************************************************************** */

void Volume_pipewire::setVolume(float volume)
{
    if (!m_available) return;

    volume = qBound(0.f, volume, m_volumeLimit);

    if (runWpctl(m_wpctl, {QStringLiteral("set-volume"), m_target, QString::number(volume, 'f', 3)}))
    {
        if (!qFuzzyCompare(m_volume + 1.f, volume + 1.f))
        {
            m_volume = volume;
            Q_EMIT volumeChanged();
        }
    }
}

void Volume_pipewire::setMute(bool mute)
{
    if (!m_available) return;

    if (runWpctl(m_wpctl, {QStringLiteral("set-mute"), m_target, mute ? QStringLiteral("1") : QStringLiteral("0")}))
    {
        if (m_mute != mute)
        {
            m_mute = mute;
            Q_EMIT muteChanged();
        }
    }
}

/* ************************************************************************** */
