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

#include "device_ylai003.h"

#include <cstdint>
#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QProcess>

#include <QDebug>

/* ************************************************************************** */

DeviceYLAI003::DeviceYLAI003(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;

    m_deviceModel = "YLAI003";
    m_deviceModelID = "YLAI003";
}

DeviceYLAI003::DeviceYLAI003(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
}

DeviceYLAI003::~DeviceYLAI003()
{
    delete serviceBattery;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceYLAI003::serviceScanDone()
{
    //qDebug() << "DeviceYLAI003::serviceScanDone(" << m_deviceAddress << ")";

    //QProcess::startDetached("lighthouse");

    if (serviceBattery)
    {
        if (serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceBattery, &QLowEnergyService::stateChanged, this, &DeviceYLAI003::serviceDetailsDiscovered_battery);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceBattery->discoverDetails(); });
        }
    }
}

void DeviceYLAI003::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DeviceYLAI003::addLowEnergyService(" << uuid.toString() << ")";
    Q_UNUSED(uuid)

    if (uuid.toString() == "{0000180f-0000-1000-8000-00805f9b34fb}") // Battery service
    {
        delete serviceBattery;
        serviceBattery = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            serviceBattery = m_bleController->createServiceObject(uuid);
            if (!serviceBattery)
                qWarning() << "Cannot create service (battery) for uuid:" << uuid.toString();
        }
    }
}

void DeviceYLAI003::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DeviceYLAI003::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        QBluetoothUuid bat(QString("00002a19-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cbat = serviceBattery->characteristic(bat);
        if (cbat.value().size() > 0)
        {
            m_deviceBattery = static_cast<uint8_t>(cbat.value().constData()[0]);
            Q_EMIT sensorUpdated();

            qDebug() << "BATTERY : " << m_deviceBattery;
        }
    }
}

/* ************************************************************************** */

void DeviceYLAI003::bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    qDebug() << "DeviceYLAI003::bleWriteDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();

    if (c.uuid().toString() == "{x}")
    {
        //
    }
}

void DeviceYLAI003::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DeviceYLAI003::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "WE HAVE DATA: 0x" \
             << Qt::hex << data[0]  << Qt::hex << data[1]  << Qt::hex << data[2] << Qt::hex << data[3] \
             << Qt::hex << data[4]  << Qt::hex << data[5]  << Qt::hex << data[6] << Qt::hex << data[7] \
             << Qt::hex << data[8]  << Qt::hex << data[9]  << Qt::hex << data[10] << Qt::hex << data[10] \
             << Qt::hex << data[12]  << Qt::hex << data[13]  << Qt::hex << data[14] << Qt::hex << data[15];

    if (c.uuid().toString() == "{x}")
    {
        //
    }

    if (c.uuid().toString() == "{x}")
    {
        if (value.size() > 0)
        {
            //
        }
    }
}

void DeviceYLAI003::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DeviceYLAI003::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
/*
    qDebug() << "WE HAVE DATA: 0x" \
             << Qt::hex << data[0]  << Qt::hex << data[1]  << Qt::hex << data[2] << Qt::hex << data[3] \
             << Qt::hex << data[4]  << Qt::hex << data[5]  << Qt::hex << data[6] << Qt::hex << data[7] \
             << Qt::hex << data[8]  << Qt::hex << data[9]  << Qt::hex << data[10] << Qt::hex << data[10] \
             << Qt::hex << data[12]  << Qt::hex << data[13]  << Qt::hex << data[14] << Qt::hex << data[15];
*/

    if (c.uuid().toString() == "{x}")
    {
        //
    }
}

/* ************************************************************************** */

void DeviceYLAI003::parseAdvertisementData(const uint16_t type,
                                           const uint16_t identifier,
                                           const QByteArray &ba)
{
    if (type == DeviceUtils::BLE_ADV_SERVICEDATA && identifier == 0xFE95 && ba.size() == 12)
    {
        //setModel("YLAI003");
        //setModelID("YLAI003");

        if (prev_data.isEmpty()) { prev_data = ba; return; } // init
        if (prev_data.compare(ba) == 0) return; // duplicate
/*
        qDebug() << "DeviceYLAI003::parseAdvertisementData(" << m_deviceAddress
                 << " - " << type << " - 0x" << identifier << ")";
        qDebug() << "DATA (" << ba.size() << "bytes)   >  0x" << ba.toHex();
*/
        const quint8 *data = reinterpret_cast<const quint8 *>(ba.constData());

        if (data[01] == 0x5a) // simple click?
        {
            triggerAction(1, 0);
            triggerEvent(1, 0);
        }
        if (data[01] == 0x58) // double click?
        {
            triggerAction(1, 1);
            triggerEvent(1, 1);
        }

        prev_data = ba;
    }
}

/* ************************************************************************** */
