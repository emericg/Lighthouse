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

#ifndef DEVICE_POKEBALLPLUS_H
#define DEVICE_POKEBALLPLUS_H
/* ************************************************************************** */

#include "../device_beacon.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

class Gamepad;

/* ************************************************************************** */

/*!
 * Nintendo "Pokeball Plus" (PBP)
 * Advertising name is 'Pokemon PBP'
 *
 * Protocol infos:
 * - https://www.reddit.com/r/Unity3D/comments/10pjw91/update_poke_ball_plus_in_unity/
 */
class DevicePokeballPlus: public DeviceBeacon
{
    Q_OBJECT

    Q_PROPERTY(bool autoConnect READ getAutoConnect WRITE setAutoConnect NOTIFY autoconnectChanged)
    Q_PROPERTY(QString deviceMode READ getDeviceMode WRITE setDeviceMode NOTIFY devicemodeChanged)

    Q_PROPERTY(float axis_x READ getXf NOTIFY axisChanged)
    Q_PROPERTY(float axis_y READ getYf NOTIFY axisChanged)
    Q_PROPERTY(float gyroX READ getGyroXf NOTIFY gyroChanged)
    Q_PROPERTY(float gyroY READ getGyroYf NOTIFY gyroChanged)
    Q_PROPERTY(float gyroZ READ getGyroZf NOTIFY gyroChanged)
    Q_PROPERTY(float acclX READ getAcclXf NOTIFY acclChanged)
    Q_PROPERTY(float acclY READ getAcclYf NOTIFY acclChanged)
    Q_PROPERTY(float acclZ READ getAcclZf NOTIFY acclChanged)

    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_gamepad(QLowEnergyService::ServiceState newState);

    QLowEnergyService *m_serviceBattery = nullptr;
    QLowEnergyService *m_serviceGamepad = nullptr;
    QLowEnergyDescriptor m_gamepadDesc;

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);

    bool m_autoConnect = true;

    QString m_deviceMode = "gamepad";

    // virtual gamepad
    Gamepad *m_gamepad = nullptr;

    float m_axis_x = 0.f;
    float m_axis_y = 0.f;
    float getXf() { return m_axis_x; };
    float getYf() { return m_axis_y; };

    float m_gyro_x = 0.f;
    float m_gyro_y = 0.f;
    float m_gyro_z = 0.f;
    float getGyroXf() { return m_gyro_x; };
    float getGyroYf() { return m_gyro_y; };
    float getGyroZf() { return m_gyro_z; };

    float m_accl_x = 0.f;
    float m_accl_y = 0.f;
    float m_accl_z = 0.f;
    float getAcclXf() { return m_accl_x; };
    float getAcclYf() { return m_accl_y; };
    float getAcclZf() { return m_accl_z; };

Q_SIGNALS:
    void autoconnectChanged();
    void devicemodeChanged();
    void btnChanged();
    void axisChanged();
    void gyroChanged();
    void acclChanged();

protected:
    int getButtonCount() { return 2; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return false; }
    bool hasLongPress() { return false; }

public:
    DevicePokeballPlus(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DevicePokeballPlus(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DevicePokeballPlus();

    bool getAutoConnect() { return m_autoConnect; }
    void setAutoConnect(const bool value);

    QString getDeviceMode() { return m_deviceMode; }
    void setDeviceMode(const QString &value);
};

/* ************************************************************************** */
#endif // DEVICE_POKEBALLPLUS_H
