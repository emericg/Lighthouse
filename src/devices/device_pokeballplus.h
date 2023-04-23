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
 * \date      2023
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef DEVICE_POKEBALLPLUS_H
#define DEVICE_POKEBALLPLUS_H
/* ************************************************************************** */

#include "../device_beacon.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Nintendo "Pokeball Plus" (PBP)
 *
 * Protocol infos:
 * - todo
 */
class DevicePokeballPlus: public DeviceBeacon
{
    Q_OBJECT

    Q_PROPERTY(float axis_x READ getXf NOTIFY axisChanged)
    Q_PROPERTY(float axis_y READ getYf NOTIFY axisChanged)

    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_gamepad(QLowEnergyService::ServiceState newState);

    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceGamepad = nullptr;
    QLowEnergyDescriptor m_gamepadDesc;

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);

    float m_axis_x = 0.f;
    float m_axis_y = 0.f;
    float getXf() { return m_axis_x; };
    float getYf() { return m_axis_y; };

Q_SIGNALS:
    void axisChanged();
    void acclChanged();
    void gyroChanged();

protected:
    int getButtonCount() { return 2; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return false; }
    bool hasLongPress() { return false; }

public:
    DevicePokeballPlus(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DevicePokeballPlus(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DevicePokeballPlus();
};

/* ************************************************************************** */
#endif // DEVICE_POKEBALLPLUS_H
