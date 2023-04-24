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
 * \date      2020
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef DEVICE_LIGHT_H
#define DEVICE_LIGHT_H
/* ************************************************************************** */

#include <QObject>
#include <QString>
#include <QColor>

#include "device.h"

/* ************************************************************************** */

/*!
 * \brief The DeviceLight class
 */
class DeviceLight: public Device
{
    Q_OBJECT

    Q_PROPERTY(unsigned mode READ getMode WRITE setMode NOTIFY modeUpdated)
    Q_PROPERTY(unsigned effect READ getEffect WRITE setEffect NOTIFY effectUpdated)

    Q_PROPERTY(unsigned luminosity READ getLuminosity WRITE setLuminosity NOTIFY dataUpdated)
    Q_PROPERTY(unsigned brightness READ getLuminosity WRITE setLuminosity NOTIFY dataUpdated)
    Q_PROPERTY(unsigned color READ getColor WRITE setColor NOTIFY dataUpdated)

    Q_PROPERTY(QString colorSaved READ getColorSaved WRITE setColorSaved NOTIFY colorSavedUpdated)

Q_SIGNALS:
    void modeUpdated();
    void effectUpdated();
    void colorsUpdated();
    void colorSavedUpdated();

protected:
    // specific data
    unsigned m_mode = 0;
    unsigned m_effect = 0;

    unsigned m_brightness = 0;
    unsigned m_colors = 0;

    QString m_colorSaved;

public:
    DeviceLight(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceLight(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    virtual ~DeviceLight();

public slots:
    unsigned getMode() const { return m_mode; }
    virtual void setMode(unsigned value);

    unsigned getEffect() const { return m_effect; }
    virtual void setEffect(unsigned value);

    unsigned getLuminosity() const { return m_brightness; }
    virtual void setLuminosity(unsigned value);

    unsigned getColor() const { return m_colors; }
    virtual void setColor(unsigned value);

    QString getColorSaved() const { return m_colorSaved; }
    virtual void setColorSaved(const QString &value);

    virtual void setColorAndBrightness(unsigned bright, unsigned r, unsigned g, unsigned b);

    unsigned getColor_a() const { return (m_colors & 0xff000000) >> 24; }
    unsigned getColor_r() const { return (m_colors & 0x00ff0000) >> 16; }
    unsigned getColor_g() const { return (m_colors & 0x0000ff00) >>  8; }
    unsigned getColor_b() const { return (m_colors & 0x000000ff); }
};

/* ************************************************************************** */
#endif // DEVICE_LIGHT_H
