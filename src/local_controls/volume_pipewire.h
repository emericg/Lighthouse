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

#ifndef VOLUME_PIPEWIRE_H
#define VOLUME_PIPEWIRE_H
/* ************************************************************************** */

#include "volume.h"

#include <QObject>
#include <QString>

class QTimer;

/* ************************************************************************** */

/*!
 * Volume controller (Linux / PipeWire, through the WirePlumber 'wpctl' CLI).
 *
 * WirePlumber does not expose a usable change-notification stream to us, so the
 * current level/mute state is refreshed by polling on a timer.
 *
 * Equivalent CLI:
 * wpctl get-volume @DEFAULT_AUDIO_SINK@
 * wpctl set-volume @DEFAULT_AUDIO_SINK@ 0.25
 * wpctl set-mute   @DEFAULT_AUDIO_SINK@ 1
 */
class Volume_pipewire: public Volume
{
    Q_OBJECT

    QString m_wpctl;   //!< absolute path to the 'wpctl' executable
    QString m_target = QStringLiteral("@DEFAULT_AUDIO_SINK@");
    QTimer *m_pollTimer = nullptr;

    bool queryState();   //!< refresh m_volume / m_mute from 'wpctl get-volume'

private slots:
    void poll();         //!< periodic refresh, emits change signals on delta

public:
    Volume_pipewire(QObject *parent = nullptr);
    virtual ~Volume_pipewire();

    virtual void setup() override;

    virtual void setVolume(float volume) override;
    virtual void setMute(bool mute) override;
};

/* ************************************************************************** */
#endif // VOLUME_PIPEWIRE_H
