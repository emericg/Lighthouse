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

#include "../local_controls/local_controls.h"
#include "../local_controls/local_actions.h"

#include "../local_controls/gamepad.h"
#include "../local_controls/gamepad_uinput.h"

#include <cmath>
#include <cstdint>
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

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }
    if (hasSetting("deviceMode"))
    {
        m_deviceMode = getSetting("deviceMode").toString();
    }
}

DevicePokeballPlus::DevicePokeballPlus(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PBP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }
    if (hasSetting("deviceMode"))
    {
        m_deviceMode = getSetting("deviceMode").toString();
    }
}

DevicePokeballPlus::~DevicePokeballPlus()
{
    delete m_serviceBattery;
    delete m_serviceGamepad;
    delete m_gamepad;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokeballPlus::setAutoConnect(const bool value)
{
    if (m_autoConnect != value)
    {
        m_autoConnect = value;
        Q_EMIT autoconnectChanged();

        setSetting("autoConnect", m_autoConnect);
    }
}

void DevicePokeballPlus::setDeviceMode(const QString &value)
{
    if (m_deviceMode != value)
    {
        m_deviceMode = value;
        Q_EMIT devicemodeChanged();

        setSetting("deviceMode", m_deviceMode);
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

// axis
//    (y)
// (x)   (x)
//    (y)

// ranges
//    36
// 32   193 (but 176 advisable?)
//   180

// deadzones
//    109
// 106 + 117
//    112

float mapAxisValue(float value,
                   const float a1, const float a2,
                   const float b1, const float b2)
{
    // clamp
    if (value < a1) value = a1;
    if (value > a2) value = a2;
    // map
    return (b1 + ((value - a1) * (b2 - b1)) / (a2 - a1));
}

float getAnalogX(uint8_t byte1, uint8_t byte2)
{
    // map value
    uint8_t value = ((byte1 & 0x0F) << 4) | ((byte2 >> 4) & 0x0F);
    float analog_value = mapAxisValue(value, 32.f, 192.f, -1.f, 1.f);

    // handle deadzone
    if (analog_value >= -0.075f && analog_value <= 0.075f)
    {
        analog_value = 0.f;
    }

    // debug
    //qDebug() << "getAnalogX()" << QString::number(value).rightJustified(2, ' ') << " > " << analog_value;

    return analog_value;
}

float getAnalogY(uint8_t value)
{
    // map value
    float analog_value = mapAxisValue(value, 36.f, 180.f, -1.f, 1.f);

    // handle deadzone
    if (analog_value >= -0.075f && analog_value <= 0.075f)
    {
        analog_value = 0.f;
    }

    // debug
    //qDebug() << "getAnalogY()" << QString::number(value).rightJustified(2, ' ') << " > " << analog_value;

    return analog_value;
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
            //connect(m_serviceGamepad, &QLowEnergyService::characteristicWritten, this, &DevicePokeballPlus::bleWriteDone);
            //connect(m_serviceGamepad, &QLowEnergyService::characteristicRead, this, &DevicePokeballPlus::bleReadDone);
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
        //qDebug() << "DevicePokeballPlus::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

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
        //qDebug() << "DevicePokeballPlus::serviceDetailsDiscovered_gamepad(" << m_deviceAddress << ") > ServiceDiscovered";

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
    //qDebug() << "DevicePokeballPlus::bleWriteDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();
}

void DevicePokeballPlus::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DevicePokeballPlus::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();
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
        int btn_a = (btn == 1 || btn == 3);
        int btn_b = (btn == 2 || btn == 3);
        Q_EMIT btnChanged();

        // gamepad axis

        m_axis_x = getAnalogX(data[3], data[2]);
        m_axis_y = getAnalogY(data[4]);
        Q_EMIT axisChanged();

        // accl

        // gyro

        ////////////////

        if (btn_a) triggerEvent(1, 0);
        if (btn_b) triggerEvent(2, 0);

        if (m_deviceMode == "button")
        {
            if (btn_a) triggerAction(1, 0);
            if (btn_b) triggerAction(2, 0);
        }

        ////////////////

        if (m_deviceMode == "mouse")
        {
            LocalControls *ctrls = LocalControls::getInstance();
            ctrls->mouse_action(static_cast<int>(m_axis_x*15.f),
                                static_cast<int>(m_axis_y*15.f),
                                btn_a, btn_b, false);
        }

        ////////////////

        if (m_deviceMode == "keyboard")
        {
            if (btn_a) triggerDirectAction(LocalActions::ACTION_MOUSE_click_left);
            if (btn_b) triggerDirectAction(LocalActions::ACTION_MOUSE_click_right);

            if (m_axis_x > 0.5) triggerDirectAction(LocalActions::ACTION_KEYBOARD_right);
            else if (m_axis_x < -0.5) triggerDirectAction(LocalActions::ACTION_KEYBOARD_left);

            if (m_axis_y > 0.5) triggerDirectAction(LocalActions::ACTION_KEYBOARD_down);
            else if (m_axis_y < -0.5) triggerDirectAction(LocalActions::ACTION_KEYBOARD_up);
        }

        ////////////////

        if (m_deviceMode == "gamepad")
        {
#if defined(ENABLE_UINPUT)
            if (!m_gamepad)
            {
                m_gamepad = new Gamepad_uinput();
                dynamic_cast<Gamepad_uinput*>(m_gamepad)->setup_pbp();
            }
            if (m_gamepad)
            {
                dynamic_cast<Gamepad_uinput*>(m_gamepad)->action_pbp(m_axis_x*32767.f, m_axis_y*32767.f, btn_a, btn_b);
            }
#endif // defined(ENABLE_UINPUT)
        }
        else
        {
            delete m_gamepad;
            m_gamepad = nullptr;
        }

        ////////////////
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
