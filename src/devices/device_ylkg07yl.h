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

#ifndef DEVICE_YLKG07YL_H
#define DEVICE_YLKG07YL_H
/* ************************************************************************** */

#include "../device_beacon.h"

#include <QObject>
#include <QString>
#include <QByteArray>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Yeelight Wireless Smart Dimmer (YLKG07YL and YLKG08YL)
 * Advertising name is 'yee-rc'
 *
 * Support the following actions: rotate, press and rotate, press, double press
 *
 * Get beacon key:
 * - https://custom-components.github.io/ble_monitor/faq#how-to-get-the-mibeacon-v2v3-encryption-key
 * - https://github.com/custom-components/ble_monitor/blob/master/custom_components/ble_monitor/ble_parser/get_beacon_key.py
 *
 * Protocol infos:
 * - todo
 */
class DeviceYLKG07YL: public DeviceBeacon
{
    Q_OBJECT

    Q_PROPERTY(QString beaconkey READ getBeaconKey NOTIFY beaconkeyChanged)

    QString m_beaconkey;
    QByteArray m_previousdata_dimmer;

Q_SIGNALS:
    void beaconkeyChanged();

protected:
    int getButtonCount() { return 3; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return true; }
    bool hasLongPress() { return false; }

public:
    DeviceYLKG07YL(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceYLKG07YL(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DeviceYLKG07YL();

    void parseAdvertisementData(const uint16_t type, const uint16_t identifier, const QByteArray &data);

    QString getBeaconKey() { return m_beaconkey; };
    Q_INVOKABLE void setBeaconKey(const QString &key);
};

/* ************************************************************************** */
#endif // DEVICE_YLKG07YL_H
