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

#ifndef VOLUME_PULSEAUDIO_H
#define VOLUME_PULSEAUDIO_H
/* ************************************************************************** */

#include "volume.h"

#include <QObject>

/* ************************************************************************** */

/*!
 * Volume controller (Linux / PulseAudio D-Bus API).
 *
 * PulseAudio exposes a D-Bus API through the 'module-dbus-protocol' module. It is
 * NOT reachable on the session or system bus directly: we must first ask the well
 * known 'org.PulseAudio1' service (on the session bus) for the address of the
 * dedicated PulseAudio bus, then open a peer-to-peer connection to it.
 *
 * Reference (equivalent CLI):
 * pactl get-sink-volume @DEFAULT_SINK@
 * pactl set-sink-volume @DEFAULT_SINK@ 25%
 */
class Volume_pulseaudio: public Volume
{
    Q_OBJECT

    QString m_connName;   //!< name of the peer QDBusConnection to the PulseAudio bus
    QString m_sinkPath;   //!< D-Bus object path of the controlled sink
    int m_channels = 2;   //!< number of channels of the controlled sink

    bool connectToServer();   //!< discover the PA bus address and open the peer connection
    bool selectFallbackSink();
    bool readSinkVolume();
    bool readSinkMute();
    void subscribe();

private slots:
    void onSinkChanged();        //!< Volume/Mute updated on the current sink
    void onFallbackSinkChanged(); //!< the default sink itself changed

public:
    Volume_pulseaudio(QObject *parent = nullptr);
    virtual ~Volume_pulseaudio();

    virtual void setup() override;

    virtual void setVolume(float volume) override;
    virtual void setMute(bool mute) override;
};

/* ************************************************************************** */
#endif // VOLUME_PULSEAUDIO_H
