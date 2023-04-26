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

#ifndef DEVICE_POKEMONGOPLUS_H
#define DEVICE_POKEMONGOPLUS_H
/* ************************************************************************** */

#include "../device_beacon.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Nintendo "Pokemon Go Plus" (PGP)
 * Advertising name is 'Pokemon GO Plus'
 *
 * Protocol infos:
 * - todo
 */
class DevicePokemonGoPlus: public DeviceBeacon
{
    Q_OBJECT

    Q_PROPERTY(bool autoConnect READ getAutoConnect WRITE setAutoConnect NOTIFY autoconnectChanged)

    bool m_autoConnect = true;

    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_certificate(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_control(QLowEnergyService::ServiceState newState);

    QLowEnergyService *m_serviceBattery = nullptr;
    QLowEnergyService *m_serviceCertificate = nullptr;
    QLowEnergyDescriptor m_sifdaNotifDesc;
    QLowEnergyService *m_serviceControl = nullptr;
    QLowEnergyDescriptor m_buttonNotifDesc;

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);

Q_SIGNALS:
    void autoconnectChanged();

protected:
    int getButtonCount() { return 1; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return false; }
    bool hasLongPress() { return false; }

public:
    DevicePokemonGoPlus(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DevicePokemonGoPlus(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DevicePokemonGoPlus();

    bool getAutoConnect() { return m_autoConnect; }
    void setAutoConnect(const bool value);
};

/* ************************************************************************** */
#endif // DEVICE_POKEMONGOPLUS_H
