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

#include "device_pokeballplus.h"
#include "utils_maths.h"

#include <cstdint>
#include <cmath>
#include <algorithm>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QDebug>

/* ************************************************************************** */

DevicePokeballPlus::DevicePokeballPlus(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PBP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;
}

DevicePokeballPlus::DevicePokeballPlus(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PBP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;
}

DevicePokeballPlus::~DevicePokeballPlus()
{
    delete m_serviceBattery;
    delete m_serviceGamepad;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokeballPlus::serviceScanDone()
{
    qDebug() << "DevicePokeballPlus::serviceScanDone(" << m_deviceAddress << ")";

    if (m_serviceBattery)
    {
        if (m_serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            connect(m_serviceBattery, &QLowEnergyService::stateChanged, this, &DevicePokeballPlus::serviceDetailsDiscovered_battery);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { m_serviceBattery->discoverDetails(); });
        }
    }

    if (m_serviceGamepad)
    {
        if (m_serviceGamepad->state() == QLowEnergyService::RemoteService)
        {
            connect(m_serviceGamepad, &QLowEnergyService::stateChanged, this, &DevicePokeballPlus::serviceDetailsDiscovered_gamepad);
            connect(m_serviceGamepad, &QLowEnergyService::characteristicWritten, this, &DevicePokeballPlus::bleWriteDone);
            connect(m_serviceGamepad, &QLowEnergyService::characteristicRead, this, &DevicePokeballPlus::bleReadDone);
            connect(m_serviceGamepad, &QLowEnergyService::characteristicChanged, this, &DevicePokeballPlus::bleReadNotify);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { m_serviceGamepad->discoverDetails(); });
        }
    }
}

void DevicePokeballPlus::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DevicePokeballPlus::addLowEnergyService(" << uuid.toString() << ")";

    if (uuid.toString() == "{0000180f-0000-1000-8000-00805f9b34fb}") // Battery service
    {
        delete m_serviceBattery;
        m_serviceBattery = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            m_serviceBattery = m_bleController->createServiceObject(uuid);
            if (!m_serviceBattery)
                qWarning() << "Cannot create service (battery) for uuid:" << uuid.toString();
        }
    }

    if (uuid.toString() == "{6675e16c-f36d-4567-bb55-6b51e27a23e5}") // Gamepad
    {
        delete m_serviceGamepad;
        m_serviceGamepad = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            m_serviceGamepad = m_bleController->createServiceObject(uuid);
            if (!m_serviceGamepad)
                qWarning() << "Cannot create service (gamepad) for uuid:" << uuid.toString();
        }
    }
}

void DevicePokeballPlus::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokeballPlus::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        QBluetoothUuid bat(QStringLiteral("00002a19-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cbat = m_serviceBattery->characteristic(bat);
        if (cbat.value().size() > 0)
        {
            int lvl = static_cast<uint8_t>(cbat.value().constData()[0]);
            setBattery(lvl);
        }
    }
}

void DevicePokeballPlus::serviceDetailsDiscovered_gamepad(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokeballPlus::serviceDetailsDiscovered_gamepad(" << m_deviceAddress << ") > ServiceDiscovered";

        // Enable notification
        QBluetoothUuid gp(QStringLiteral("6675e16c-f36d-4567-bb55-6b51e27a23e6"));
        QLowEnergyCharacteristic cgp = m_serviceGamepad->characteristic(gp);
        m_gamepadDesc = cgp.clientCharacteristicConfiguration();
        m_serviceGamepad->writeDescriptor(m_gamepadDesc, QByteArray::fromHex("0100"));
    }
}

/* ************************************************************************** */

void DevicePokeballPlus::bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    qDebug() << "DevicePokeballPlus::bleWriteDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();

    if (c.uuid().toString() == "{x}")
    {
        //
    }
}

void DevicePokeballPlus::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DevicePokeballPlus::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid().toString() == "{x}")
    {
        if (value.size() > 0)
        {
            //
        }
    }
}

void DevicePokeballPlus::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    //qDebug() << "DevicePokeballPlus::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid().toString() == "{6675e16c-f36d-4567-bb55-6b51e27a23e6}")
    {
        // gamepad buttons

        int btn = data[1];
        if (btn == 1 || btn == 2)
        {
            triggerEvent(btn, 0);
        }
        else if (btn == 3)
        {
            triggerEvent(1, 0);
            triggerEvent(2, 0);
        }

        // gamepad axis

        // accl

        // gyro
    }
}

/* ************************************************************************** */
