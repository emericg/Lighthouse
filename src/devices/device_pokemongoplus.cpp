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

#include "device_pokemongoplus.h"

#include <cstdint>
#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QDebug>

/* ************************************************************************** */

DevicePokemonGoPlus::DevicePokemonGoPlus(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PGP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }
}

DevicePokemonGoPlus::DevicePokemonGoPlus(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PGP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }
}

DevicePokemonGoPlus::~DevicePokemonGoPlus()
{
    delete m_serviceBattery;
    delete m_serviceCertificate;
    delete m_serviceControl;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokemonGoPlus::setAutoConnect(const bool value)
{
    if (m_autoConnect != value)
    {
        m_autoConnect = value;
        Q_EMIT autoconnectChanged();

        setSetting("autoConnect", m_autoConnect);
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokemonGoPlus::serviceScanDone()
{
    qDebug() << "DevicePokemonGoPlus::serviceScanDone(" << m_deviceAddress << ")";

    if (m_serviceBattery)
    {
        if (m_serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            connect(m_serviceBattery, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_battery);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { m_serviceBattery->discoverDetails(); });
        }
    }

    if (m_serviceCertificate)
    {
        if (m_serviceCertificate->state() == QLowEnergyService::RemoteService)
        {
            connect(m_serviceCertificate, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_certificate);
            connect(m_serviceCertificate, &QLowEnergyService::characteristicWritten, this, &DevicePokemonGoPlus::bleWriteDone);
            connect(m_serviceCertificate, &QLowEnergyService::characteristicRead, this, &DevicePokemonGoPlus::bleReadDone);
            connect(m_serviceCertificate, &QLowEnergyService::characteristicChanged, this, &DevicePokemonGoPlus::bleReadNotify);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { m_serviceCertificate->discoverDetails(); });
        }
    }

    if (m_serviceControl)
    {
        if (m_serviceControl->state() == QLowEnergyService::RemoteService)
        {
            connect(m_serviceControl, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_control);
            connect(m_serviceControl, &QLowEnergyService::characteristicWritten, this, &DevicePokemonGoPlus::bleWriteDone);
            connect(m_serviceControl, &QLowEnergyService::characteristicRead, this, &DevicePokemonGoPlus::bleReadDone);
            connect(m_serviceControl, &QLowEnergyService::characteristicChanged, this, &DevicePokemonGoPlus::bleReadNotify);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { m_serviceControl->discoverDetails(); });
        }
    }
}

void DevicePokemonGoPlus::addLowEnergyService(const QBluetoothUuid &uuid)
{
    qDebug() << "DevicePokemonGoPlus::addLowEnergyService(" << uuid.toString() << ")";
    Q_UNUSED(uuid)

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
/*
    if (uuid.toString() == "{bbe87709-5b89-4433-ab7f-8b8eef0d8e37}") // Certificate
    {
        delete serviceCertificate;
        serviceCertificate = nullptr;

        serviceCertificate = controller->createServiceObject(uuid);
        if (!serviceCertificate)
            qWarning() << "Cannot create service (certificate) for uuid:" << uuid.toString();
    }

    if (uuid.toString() == "{21c50462-67cb-63a3-5c4c-82b5b9939aeb}") // Device control
    {
        delete serviceControl;
        serviceControl = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            serviceControl = controller->createServiceObject(uuid);
            if (!serviceControl)
                qWarning() << "Cannot create service (control) for uuid:" << uuid.toString();
        }
    }
*/
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        QBluetoothUuid bat(QString("00002a19-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cbat = m_serviceBattery->characteristic(bat);
        if (cbat.value().size() > 0)
        {
            int lvl = static_cast<uint8_t>(cbat.value().constData()[0]);
            setBattery(lvl);
        }
    }
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_certificate(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_certificate(" << m_deviceAddress << ") > ServiceDiscovered";

        // SFIDA_COMMANDS    bbe87709-5b89-4433-ab7f-8b8eef0d8e39
        // CENTRAL_TO_SFIDA  bbe87709-5b89-4433-ab7f-8b8eef0d8e38
        // SFIDA_TO_CENTRAL  bbe87709-5b89-4433-ab7f-8b8eef0d8e3a

        QBluetoothUuid sfida(QString("bbe87709-5b89-4433-ab7f-8b8eef0d8e39"));
        QLowEnergyCharacteristic csfida = m_serviceCertificate->characteristic(sfida);

        // QT6
        //m_sifdaNotifDesc = csfida.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        //serviceCertificate->writeDescriptor(m_sifdaNotifDesc, QByteArray::fromHex("0100"));

        QBluetoothUuid sTOc(QString("bbe87709-5b89-4433-ab7f-8b8eef0d8e38"));
        QBluetoothUuid cTOs(QString("bbe87709-5b89-4433-ab7f-8b8eef0d8e3a"));
    }
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_control(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_control(" << m_deviceAddress << ") > ServiceDiscovered";

        // Set LED
        QBluetoothUuid l(QString("21c50462-67cb-63a3-5c4c-82b5b9939aec"));
        QLowEnergyCharacteristic ll = m_serviceControl->characteristic(l);
        m_serviceControl->writeCharacteristic(ll, QByteArray::fromHex("06000012"));

        // Subscribe to button
        QBluetoothUuid b(QString("21c50462-67cb-63a3-5c4c-82b5b9939aed"));
        QLowEnergyCharacteristic bb = m_serviceControl->characteristic(b);

        // QT6
        //m_buttonNotifDesc = bb.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        //serviceControl->writeDescriptor(m_buttonNotifDesc, QByteArray::fromHex("0100"));
    }
}

/* ************************************************************************** */

void DevicePokemonGoPlus::bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    qDebug() << "DevicePokemonGoPlus::bleWriteDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();

    if (c.uuid().toString() == "{x}")
    {
        //
    }
}

void DevicePokemonGoPlus::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DevicePokemonGoPlus::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

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

void DevicePokemonGoPlus::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DevicePokemonGoPlus::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid().toString() == "{x}")
    {
        //
    }
    if (c.uuid().toString() == "{bbe87709-5b89-4433-ab7f-8b8eef0d8e39}")
    {
        // response from PGP

        // response to initial sifda notify?
        qDebug() << "initial sifda notify: 0x" << Qt::hex << data[0]  << Qt::hex << data[1]  << Qt::hex << data[2] << Qt::hex << data[3] ;

        // reads the sfida_to_central characteristic (bbe87709-5b89-4433-ab7f-8b8eef0d8e3a
        QBluetoothUuid cTOs(QString("bbe87709-5b89-4433-ab7f-8b8eef0d8e3a"));
        QLowEnergyCharacteristic ccTOs = m_serviceCertificate->characteristic(cTOs);
        m_serviceCertificate->readCharacteristic(ccTOs);
    }
}
