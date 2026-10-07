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

#include "volume_coreaudio.h"

#include <AudioToolbox/AudioServices.h> // kAudioHardwareServiceDeviceProperty_VirtualMainVolume, also served by the HAL itself

#include <QMetaObject>
#include <QDebug>

/* ************************************************************************** */

static constexpr AudioObjectPropertyAddress s_defaultDeviceAddress = {
    kAudioHardwarePropertyDefaultOutputDevice,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMain,
};
static constexpr AudioObjectPropertyAddress s_volumeAddress = {
    kAudioHardwareServiceDeviceProperty_VirtualMainVolume,
    kAudioDevicePropertyScopeOutput,
    kAudioObjectPropertyElementMain,
};
static constexpr AudioObjectPropertyAddress s_muteAddress = {
    kAudioDevicePropertyMute,
    kAudioDevicePropertyScopeOutput,
    kAudioObjectPropertyElementMain,
};

/* ************************************************************************** */

Volume_coreaudio::Volume_coreaudio(QObject *parent) : Volume(parent)
{
    setup();
}

Volume_coreaudio::~Volume_coreaudio()
{
    AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &s_defaultDeviceAddress,
                                      &Volume_coreaudio::defaultDeviceListener, this);
    unbindDevice();
}

/* ************************************************************************** */

void Volume_coreaudio::setup()
{
    const OSStatus err = AudioObjectAddPropertyListener(kAudioObjectSystemObject, &s_defaultDeviceAddress,
                                                        &Volume_coreaudio::defaultDeviceListener, this);
    if (err != noErr) qWarning() << "Volume_coreaudio: unable to watch the default output device" << err;

    bindDefaultDevice();
}

void Volume_coreaudio::bindDefaultDevice()
{
    AudioObjectID device = kAudioObjectUnknown;
    UInt32 size = sizeof(device);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &s_defaultDeviceAddress, 0, nullptr, &size, &device);

    if (device == m_device && device != kAudioObjectUnknown) return;

    unbindDevice();

    const bool wasAvailable = m_available;
    m_available = false;

    if (device != kAudioObjectUnknown && AudioObjectHasProperty(device, &s_volumeAddress))
    {
        m_device = device;
        m_hasMute = AudioObjectHasProperty(device, &s_muteAddress);

        AudioObjectAddPropertyListener(device, &s_volumeAddress, &Volume_coreaudio::deviceListener, this);
        if (m_hasMute) AudioObjectAddPropertyListener(device, &s_muteAddress, &Volume_coreaudio::deviceListener, this);

        Boolean settable = false;
        AudioObjectIsPropertySettable(device, &s_volumeAddress, &settable);
        m_available = settable;
    }
    else
    {
        qWarning() << "Volume_coreaudio: the default output device has no volume control";
    }

    if (wasAvailable != m_available) Q_EMIT availabilityChanged();

    refresh();
}

void Volume_coreaudio::unbindDevice()
{
    if (m_device == kAudioObjectUnknown) return;

    AudioObjectRemovePropertyListener(m_device, &s_volumeAddress, &Volume_coreaudio::deviceListener, this);
    if (m_hasMute) AudioObjectRemovePropertyListener(m_device, &s_muteAddress, &Volume_coreaudio::deviceListener, this);

    m_device = kAudioObjectUnknown;
    m_hasMute = false;
}

/* ************************************************************************** */

void Volume_coreaudio::refresh()
{
    const float prevVol = m_volume;
    const bool prevMute = m_mute;

    m_volume = -1.f;
    m_mute = false;

    if (m_device != kAudioObjectUnknown)
    {
        Float32 vol = 0.f;
        UInt32 size = sizeof(vol);
        if (AudioObjectGetPropertyData(m_device, &s_volumeAddress, 0, nullptr, &size, &vol) == noErr)
        {
            m_volume = qBound(0.f, float(vol), 1.f);
        }

        if (m_hasMute)
        {
            UInt32 mute = 0;
            size = sizeof(mute);
            if (AudioObjectGetPropertyData(m_device, &s_muteAddress, 0, nullptr, &size, &mute) == noErr)
            {
                m_mute = (mute != 0);
            }
        }
    }

    if (!qFuzzyCompare(prevVol + 1.f, m_volume + 1.f)) Q_EMIT volumeChanged();
    if (prevMute != m_mute) Q_EMIT muteChanged();
}

/* ************************************************************************** */

void Volume_coreaudio::setVolume(float volume)
{
    if (!m_available) return;

    volume = qBound(0.f, volume, m_volumeLimit);

    const Float32 vol = volume;
    const OSStatus err = AudioObjectSetPropertyData(m_device, &s_volumeAddress, 0, nullptr, sizeof(vol), &vol);
    if (err != noErr)
    {
        qWarning() << "Volume_coreaudio: unable to set the volume" << err;
        return;
    }

    // the listener will confirm it, but keep the state coherent right away
    refresh();
}

void Volume_coreaudio::setMute(bool mute)
{
    if (!m_available || !m_hasMute) return;

    const UInt32 value = mute ? 1 : 0;
    const OSStatus err = AudioObjectSetPropertyData(m_device, &s_muteAddress, 0, nullptr, sizeof(value), &value);
    if (err != noErr)
    {
        qWarning() << "Volume_coreaudio: unable to set the mute state" << err;
        return;
    }

    refresh();
}

/* ************************************************************************** */

// Core Audio calls the listeners from its own threads, so hop back to ours

OSStatus Volume_coreaudio::defaultDeviceListener(AudioObjectID, UInt32,
                                                 const AudioObjectPropertyAddress *, void *data)
{
    auto *self = static_cast<Volume_coreaudio *>(data);
    QMetaObject::invokeMethod(self, [self]() { self->bindDefaultDevice(); }, Qt::QueuedConnection);
    return noErr;
}

OSStatus Volume_coreaudio::deviceListener(AudioObjectID object, UInt32,
                                          const AudioObjectPropertyAddress *, void *data)
{
    auto *self = static_cast<Volume_coreaudio *>(data);
    QMetaObject::invokeMethod(self, [self, object]() {
        // late notification from a device we already left
        if (object == self->m_device) self->refresh();
    }, Qt::QueuedConnection);
    return noErr;
}

/* ************************************************************************** */
