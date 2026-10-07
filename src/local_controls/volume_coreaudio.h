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
 * \date      2026
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef VOLUME_COREAUDIO_H
#define VOLUME_COREAUDIO_H
/* ************************************************************************** */

#include "volume.h"

#include <QObject>

#include <CoreAudio/CoreAudio.h>

/* ************************************************************************** */

/*!
 * Volume controller (macOS / Core Audio).
 *
 * Controls the default output device, and follows it when it changes
 * (headphones plugged in, AirPlay or Bluetooth device selected...).
 *
 * The level is the "virtual main volume", the same control as the menu bar
 * slider: it also works with devices only having per channel volumes.
 * Changes are notified by Core Audio, so there is no polling.
 *
 * Some devices (HDMI, some USB DACs) have no software volume or mute at all,
 * the controller then reports itself unavailable.
 */
class Volume_coreaudio: public Volume
{
    Q_OBJECT

    AudioObjectID m_device = kAudioObjectUnknown;   //!< current default output device
    bool m_hasMute = false;

    //! Switch to the current default output device
    void bindDefaultDevice();
    void unbindDevice();

    //! Refresh m_volume / m_mute from the device, emitting the changes
    void refresh();

    static OSStatus defaultDeviceListener(AudioObjectID object, UInt32 count,
                                          const AudioObjectPropertyAddress *addresses, void *data);
    static OSStatus deviceListener(AudioObjectID object, UInt32 count,
                                   const AudioObjectPropertyAddress *addresses, void *data);

public:
    Volume_coreaudio(QObject *parent = nullptr);
    virtual ~Volume_coreaudio();

    virtual void setup() override;

    virtual void setVolume(float volume) override;
    virtual void setMute(bool mute) override;
};

/* ************************************************************************** */
#endif // VOLUME_COREAUDIO_H
