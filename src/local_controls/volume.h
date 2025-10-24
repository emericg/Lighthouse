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
 */
class Volume: public QObject
{
    Q_OBJECT

    float m_volume = -1.f;

    bool m_mute = false;

public:
    Volume(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~Volume() = default;

    virtual void setup() = 0;

    //virtual QStringList listSinks() = 0;
    //virtual void selectSink(const QString &sink) = 0;

    virtual float getVolume() const = 0;
    virtual void setVolume(float volume) = 0;

    virtual float getMute() const = 0;
    virtual void setMute(bool mute) = 0;
    virtual void mute() = 0;
    virtual void unmute() = 0;
};

/* ************************************************************************** */
#endif // VOLUME_H
