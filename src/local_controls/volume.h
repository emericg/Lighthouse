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

#ifndef VOLUME_H
#define VOLUME_H
/* ************************************************************************** */

#include <QObject>

/* ************************************************************************** */

/*!
 * Minimal API to create virtual volume controllers.
 *
 * Volume levels are expressed as a normalized float in the [0.0 ; 1.0] range.
 * A negative volume (-1.0) means "unknown / not available yet".
 */
class Volume: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ isAvailable NOTIFY availabilityChanged)
    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ getMute WRITE setMute NOTIFY muteChanged)

protected:
    float m_volume = -1.f;   //!< normalized [0.0 ; 1.0], -1.0 if unknown
    bool m_mute = false;
    bool m_available = false; //!< true once a sink has been found and is controllable
    float m_volumeLimit = 1.f; //!< hard cap [0.0 ; 1.0]: setVolume() never exceeds this

public:
    Volume(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~Volume() = default;

    virtual void setup() = 0;

    //virtual QStringList listSinks() = 0;
    //virtual void selectSink(const QString &sink) = 0;

    bool isAvailable() const { return m_available; }

    virtual float getVolume() const { return m_volume; }
    virtual void setVolume(float volume) = 0;

    virtual bool getMute() const { return m_mute; }
    virtual void setMute(bool mute) = 0;

    Q_INVOKABLE void mute() { setMute(true); }
    Q_INVOKABLE void unmute() { setMute(false); }
    Q_INVOKABLE virtual void toggleMute() { setMute(!m_mute); }

    Q_INVOKABLE virtual void volumeUp(float step = 0.05f) {
        float v = (m_volume < 0.f) ? 0.f : m_volume;
        setVolume(qBound(0.f, v + step, m_volumeLimit));
    }
    Q_INVOKABLE virtual void volumeDown(float step = 0.05f) {
        float v = (m_volume < 0.f) ? 0.f : m_volume;
        setVolume(qBound(0.f, v - step, m_volumeLimit));
    }

    //! Set the hard cap [0.0 ; 1.0]. If the current level is above it, pull it down.
    void setVolumeLimit(float limit) {
        m_volumeLimit = qBound(0.f, limit, 1.f);
        if (m_available && m_volume > m_volumeLimit) setVolume(m_volumeLimit);
    }
    float getVolumeLimit() const { return m_volumeLimit; }

signals:
    void availabilityChanged();
    void volumeChanged();
    void muteChanged();
};

/* ************************************************************************** */
#endif // VOLUME_H
