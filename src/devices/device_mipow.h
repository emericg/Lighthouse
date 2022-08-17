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

#ifndef DEVICE_MIPOW_H
#define DEVICE_MIPOW_H
/* ************************************************************************** */

#include "../device_light.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * MiPow "Smart Bulb"
 *
 * Protocol infos:
 * - TODO
 */
class DeviceMiPow: public DeviceLight
{
    Q_OBJECT

public:
    DeviceMiPow(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceMiPow(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DeviceMiPow();

public slots:
    void setMode(unsigned value);
    void setEffect(unsigned value);

    void setOff();

    void setLuminosity(unsigned value);
    void setLuminosity_float(float value);

    void setColors(unsigned bright, unsigned r, unsigned g, unsigned b);
    void setColors_float(float bright, float r, float g, float b);
    void setColors_rgb_hex(QString rgb_hex);

private:
    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_data(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_infos(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceError(QLowEnergyService::ServiceError error);

    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceData = nullptr;
    QLowEnergyService *serviceInfos = nullptr;
    QLowEnergyDescriptor m_notificationDesc;
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);
};

/* ************************************************************************** */
#endif // DEVICE_MIPOW_H
