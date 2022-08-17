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

#include "device_ylyk01yl.h"

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QDebug>

#include <cstdint>
#include <cmath>

/* ************************************************************************** */

DeviceYLYK01YL::DeviceYLYK01YL(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;

    m_deviceModel = "YLYK01YL";
    m_deviceModelID = "YLYK01YL";
}

DeviceYLYK01YL::DeviceYLYK01YL(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;

    m_deviceModel = "YLYK01YL";
    m_deviceModelID = "YLYK01YL";
}

DeviceYLYK01YL::~DeviceYLYK01YL()
{
    //
}

/* ************************************************************************** */

void DeviceYLYK01YL::parseAdvertisementData(const uint16_t type,
                                            const uint16_t identifier,
                                            const QByteArray &ba)
{
/*
    qDebug() << "DeviceYLYK01YL::parseAdvertisementData(" << m_deviceAddress
             << " - " << type << " - 0x" << identifier << ")";
    qDebug() << "DATA (" << ba.size() << "bytes)   >  0x" << ba.toHex();
*/

    if (type == DeviceUtils::BLE_ADV_SERVICEDATA && identifier == 0xFE95 && ba.size() == 17)
    {
        //setModel("YLYK01YL");
        //setModelID("YLYK01YL");

        if (prev_data_remote.isEmpty()) { prev_data_remote = ba; return; } // init
        if (prev_data_remote.compare(ba) == 0) return; // duplicate

        const quint8 *data = reinterpret_cast<const quint8 *>(ba.constData());

        int FrameControl = data[0] + (data[1] << 8);
        int ProductID = data[2] + (data[3] << 8);
        int FrameSequence = data[4];
        int btn = data[14] + 1;
        int mode = data[16];

        // Save mac address (for macOS and iOS)
        if (!hasAddressMAC())
        {
            QString mac;

            mac += ba.mid(10,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(9,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(8,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(7,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(6,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(5,1).toHex().toUpper();

            setAddressMAC(mac);
        }
/*
        qDebug() << "- ----------------";
        qDebug() << "DATA (" << ba.size() << "bytes)   >  0x" << ba.toHex();

        qDebug() << "- FrameControl:" << FrameControl;
        qDebug() << "- ProductID:" << ProductID;
        qDebug() << "- mac:" << m_deviceAddress;
        qDebug() << "- FrameSequence:" << FrameSequence;

        qDebug() << "- btn:" << btn;
        qDebug() << "- mode:" << mode;
*/
        triggerAction(btn, mode);
        triggerEvent(btn, mode);

        prev_data_remote = ba;
    }
}

/* ************************************************************************** */
