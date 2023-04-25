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

#include "device_mipow.h"

#include <cstdint>
#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QSqlQuery>
#include <QSqlError>

#include <QColor>
#include <QDateTime>
#include <QTimeZone>

#include <QDebug>

/* ************************************************************************** */

DeviceMiPow::DeviceMiPow(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceLight(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_LIGHT;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;
}

DeviceMiPow::DeviceMiPow(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceLight(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_LIGHT;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;
}

DeviceMiPow::~DeviceMiPow()
{
    delete serviceBattery;
    delete serviceData;
    delete serviceInfos;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceMiPow::serviceScanDone()
{
    //qDebug() << "DeviceMiPow::serviceScanDone(" << m_deviceAddress << ")";

    if (serviceInfos)
    {
        if (serviceInfos->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceInfos, &QLowEnergyService::stateChanged, this, &DeviceMiPow::serviceDetailsDiscovered_infos);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceInfos->discoverDetails(QLowEnergyService::SkipValueDiscovery); });
        }
    }

    if (serviceBattery)
    {
        if (serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceBattery, &QLowEnergyService::stateChanged, this, &DeviceMiPow::serviceDetailsDiscovered_battery);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceBattery->discoverDetails(QLowEnergyService::SkipValueDiscovery); });
        }
    }

    if (serviceData)
    {
        if (serviceData->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceData, &QLowEnergyService::stateChanged, this, &DeviceMiPow::serviceDetailsDiscovered_data);
            //connect(serviceData, &QLowEnergyService::error, this, &DeviceMiPow::serviceError);
            connect(serviceData, &QLowEnergyService::characteristicWritten, this, &DeviceMiPow::bleWriteDone);
            connect(serviceData, &QLowEnergyService::characteristicRead, this, &DeviceMiPow::bleReadDone);
            connect(serviceData, &QLowEnergyService::characteristicChanged, this, &DeviceMiPow::bleReadNotify);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceData->discoverDetails(QLowEnergyService::SkipValueDiscovery); });
        }
    }
}

void DeviceMiPow::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DeviceMiPow::addLowEnergyService(" << uuid.toString() << ")";
/*
    if (uuid.toString() == "{0000180a-0000-1000-8000-00805f9b34fb}") // device information
    {
        delete serviceInfos;
        serviceInfos = nullptr;

        if (m_deviceFirmware.isEmpty() || m_deviceFirmware == "UNKN")
        {
            serviceInfos = m_bleController->createServiceObject(uuid);
            if (!serviceInfos)
                qWarning() << "Cannot create service (infos) for uuid:" << uuid.toString();
        }
    }

    if (uuid.toString() == "{0000180f-0000-1000-8000-00805f9b34fb}") // battery service
    {
        delete serviceBattery;
        serviceBattery = nullptr;

        serviceBattery = m_bleController->createServiceObject(uuid);
        if (!serviceBattery)
            qWarning() << "Cannot create service (battery) for uuid:" << uuid.toString();
    }
*/
    if (uuid.toString() == "{0000ff0d-0000-1000-8000-00805f9b34fb}") // (unknown service) // data
    {
        delete serviceData;
        serviceData = nullptr;

        serviceData = m_bleController->createServiceObject(uuid);
        if (!serviceData)
            qWarning() << "Cannot create service (data) for uuid:" << uuid.toString();
    }
}

void DeviceMiPow::serviceDetailsDiscovered_infos(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceMiPow::serviceDetailsDiscovered_infos(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceInfos)
        {
            // Characteristic "Firmware Revision String"
            QBluetoothUuid f(QStringLiteral("00002a26-0000-1000-8000-00805f9b34fb"));
            QLowEnergyCharacteristic chf = serviceInfos->characteristic(f);
            if (chf.value().size() > 0)
            {
               m_deviceFirmware = chf.value();
               Q_EMIT sensorUpdated();

               //qDebug() << "firmware: " << m_deviceFirmware;
            }
        }
    }
}

void DeviceMiPow::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceMiPow::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceBattery)
        {
            // Characteristic "Battery Level"
            QBluetoothUuid bat(QStringLiteral("00002a19-0000-1000-8000-00805f9b34fb"));
            QLowEnergyCharacteristic cbat = serviceBattery->characteristic(bat);
            if (cbat.value().size() > 0)
            {
               int lvl = static_cast<uint8_t>(cbat.value().constData()[0]);
               setBattery(lvl);
            }
        }
    }
}

void DeviceMiPow::serviceDetailsDiscovered_data(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceMiPow::serviceDetailsDiscovered_data(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceData)
        {
            // read initial brightness and colors
            QBluetoothUuid clr(QStringLiteral("0000fffc-0000-1000-8000-00805f9b34fb"));
            QLowEnergyCharacteristic cclr = serviceData->characteristic(clr);
            serviceData->readCharacteristic(cclr);

            // effects
            //QBluetoothUuid eff(QStringLiteral("0000fffb-0000-1000-8000-00805f9b34fb"));
            //QLowEnergyCharacteristic ceff = serviceData->characteristic(eff);
            //m_notificationDesc = chth.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            //serviceData->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
        }
    }
}

/* ************************************************************************** */

void DeviceMiPow::serviceError(QLowEnergyService::ServiceError error)
{
    //qDebug() << "DeviceMiPow::serviceError(err:" << error;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceMiPow::setMode(unsigned value)
{
    //qDebug() << "DeviceMiPow::setMode(" << value << ")";
}

void DeviceMiPow::setEffect(unsigned value)
{
    //qDebug() << "DeviceMiPow::setEffect(" << value << ")";
}

/* ************************************************************************** */

void DeviceMiPow::setOff()
{
    //qDebug() << "DeviceMiPow::setOff()";

    if (serviceData && m_ble_status >= DeviceUtils::DEVICE_CONNECTED)
    {
        QByteArray v;
        v.push_back(char(0));
        v.push_back(char(0));
        v.push_back(char(0));
        v.push_back(char(0));

        QBluetoothUuid ic(QStringLiteral("0000fffc-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cic = serviceData->characteristic(ic);
        serviceData->writeCharacteristic(cic, v, QLowEnergyService::WriteWithoutResponse);
    }
}

void DeviceMiPow::setLuminosity(unsigned brightness)
{
    //qDebug() << "DeviceMiPow::setLuminosity(" << brightness << ")";

    if (serviceData && m_ble_status >= DeviceUtils::DEVICE_CONNECTED)
    {
        QByteArray v;
        v.push_back(char(brightness));
        v.push_back(char(0));
        v.push_back(char(0));
        v.push_back(char(0));

        m_brightness = brightness;

        QBluetoothUuid ic(QStringLiteral("0000fffc-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cic = serviceData->characteristic(ic);
        serviceData->writeCharacteristic(cic, v, QLowEnergyService::WriteWithoutResponse);
    }
}

void DeviceMiPow::setColors(unsigned brightness, unsigned r, unsigned g, unsigned b)
{
    //qDebug() << "DeviceMiPow::setColors(" << brightness << r << g << b << ")";

    if (serviceData && m_ble_status >= DeviceUtils::DEVICE_CONNECTED)
    {
        QByteArray v;
        v.push_back(char(brightness));
        v.push_back(char(r));
        v.push_back(char(g));
        v.push_back(char(b));

        m_brightness = brightness;

        QBluetoothUuid ic(QStringLiteral("0000fffc-0000-1000-8000-00805f9b34fb"));
        QLowEnergyCharacteristic cic = serviceData->characteristic(ic);
        serviceData->writeCharacteristic(cic, v, QLowEnergyService::WriteWithoutResponse);
    }
}

/* ************************************************************************** */

void DeviceMiPow::setLuminosity_float(float value)
{
    int br = value * 255;
    setLuminosity(br);
}

void DeviceMiPow::setColors_float(float bright, float r, float g, float b)
{
    //qDebug() << "DeviceMiPow::setColors_float(" << bright << r << g << b << ")";

    int br = bright * 255;
    int rr = r * 255;
    int gg = g * 255;
    int bb = b * 255;

    setColors(br, rr, gg, bb);
}

void DeviceMiPow::setColors_rgb_hex(const QString &rgb_hex)
{
    //qDebug() << "DeviceMiPow::setColors_rgb_hex(" << rgb_hex << ")";

    QColor clr;
    clr.setNamedColor(rgb_hex);

    int br = m_brightness;
    int rr = clr.red();
    int gg = clr.green();
    int bb = clr.blue();
/*
    int rr = (rgb & 0xff0000) >> 16;
    int gg = (rgb & 0x00ff00) >> 8;
    int bb = (rgb & 0x0000ff);
*/
    setColors(br, rr, gg, bb);
}

/* ************************************************************************** */

void DeviceMiPow::bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DeviceMiPow::bleWriteDone(" << m_deviceAddress << ")";

    Q_UNUSED(c)
    Q_UNUSED(value)
}

void DeviceMiPow::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DeviceMiPow::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid().toString() == "{0000fffc-0000-1000-8000-00805f9b34fb}")
    {
        const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

        unsigned argb = static_cast<unsigned>(data[3] +
                                             (data[2] << 8) +
                                             (data[1] << 16) +
                                             (data[0] << 24));

        m_brightness = (argb & 0xff000000) >> 24;
        m_colors = (argb & 0x00ffffff);
        Q_EMIT dataUpdated();

        //qDebug() << "initial read / argb:   " << argb;
        //qDebug() << "initial read / brightness:   " << m_brightness << "   /   m_colors:   " << m_colors;
    }
}

void DeviceMiPow::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DeviceMiPow::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    Q_UNUSED(c)
    Q_UNUSED(value)

    if (c.uuid().toString() == "{x}")
    {
        //const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    }
}

void DeviceMiPow::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    //qDebug() << "DeviceMiPow::confirmedDescriptorWrite!";

    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000"))
    {
        //
    }
}

/* ************************************************************************** */
