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
 * \date      2022
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef DEVICE_YLAI003_H
#define DEVICE_YLAI003_H
/* ************************************************************************** */

#include "../device_beacon.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Yeelight square button (YLAI003)
 * Advertising name is "YLKG1"
 * Support the following actions: press, double press, long press
 *
 * Protocol infos:
 * - todo
 */
class DeviceYLAI003: public DeviceBeacon
{
    Q_OBJECT

    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);

    QLowEnergyService *serviceBattery = nullptr;

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);

    QByteArray prev_data;

protected:
    int getButtonCount() { return 1; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return true; }
    bool hasLongPress() { return false; }

public:
    DeviceYLAI003(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceYLAI003(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DeviceYLAI003();

    void parseAdvertisementData(const uint16_t type, const uint16_t identifier,
                                const QByteArray &data);
};

/* ************************************************************************** */
#endif // DEVICE_YLAI003_H
