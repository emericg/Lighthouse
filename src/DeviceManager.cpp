/*!
 * This file is part of Lighthouse.
 * Copyright (c) 2022 Emeric Grange - All Rights Reserved
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
 * \date      2018
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "DeviceManager.h"
#include "DatabaseManager.h"
#include "SettingsManager.h"

#include "device.h"
#include "devices/device_mipow.h"
//#include "devices/device_pokeballplus.h"
#include "devices/device_ylai003.h"
#include "devices/device_ylkg07yl.h"
#include "devices/device_ylyk01yl.h"

#include "utils_app.h"

#include <QList>
#include <QDateTime>
#include <QDebug>

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyConnectionParameters>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

/* ************************************************************************** */

DeviceManager::DeviceManager(bool daemon)
{
    m_daemonMode = daemon;

    // Data model init
    m_devices_model = new DeviceModel(this);
    m_devices_filter = new DeviceFilter(this);
    m_devices_filter->setSourceModel(m_devices_model);
    SettingsManager *sm = SettingsManager::getInstance();
    if (sm)
    {
        //if (sm->getOrderBy() == "manual") orderby_manual();
        if (sm->getOrderBy() == "location") orderby_location();
        if (sm->getOrderBy() == "plant") orderby_plant();
        if (sm->getOrderBy() == "waterlevel") orderby_waterlevel();
        if (sm->getOrderBy() == "model") orderby_model();
        if (sm->getOrderBy() == "insideoutside") orderby_insideoutside();
    }

    // BLE init
    startBleAgent();
    enableBluetooth(true); // Enables adapter // ONLY if off and permission given
    checkBluetooth();

    connect(this, &DeviceManager::bluetoothChanged, this, &DeviceManager::bluetoothStatusChanged);

    // Database
    DatabaseManager *db = DatabaseManager::getInstance();
    if (db)
    {
        m_dbInternal = db->hasDatabaseInternal();
        m_dbExternal = db->hasDatabaseExternal();
    }

    if (m_dbInternal || m_dbExternal)
    {
        // Load blacklist
        QSqlQuery queryBlacklist;
        queryBlacklist.exec("SELECT deviceAddr FROM devicesBlacklist");
        while (queryBlacklist.next())
        {
            m_devices_blacklist.push_back(queryBlacklist.value(0).toString());
        }

        // Load saved devices
        QSqlQuery queryDevices;
        queryDevices.exec("SELECT deviceName, deviceModel, deviceAddr FROM devices");
        while (queryDevices.next())
        {
            QString deviceName = queryDevices.value(0).toString();
            QString deviceModel = queryDevices.value(1).toString();
            QString deviceAddr = queryDevices.value(2).toString();

            Device *d = nullptr;

            if (deviceName == "PLAYBULB CANDLE" || deviceName == "MIPOW smart bulb")
            {
                d = new DeviceMiPow(deviceAddr, deviceName, this);
            }
            else if (deviceName == "Pokemon PBP" || deviceName == "Pokemon GO Plus")
            {
                //d = new DevicePokeballPlus(deviceAddr, deviceName, this);
            }
            else if (deviceName == "YLKG")
            {
                d = new DeviceYLAI003(deviceAddr, deviceName, this);
            }
            else if (deviceName == "yee-rc" && deviceModel == "YLYK01YL")
            {
                d = new DeviceYLYK01YL(deviceAddr, deviceName, this);
            }
            else if (deviceName == "yee-rc" && (deviceModel == "YLKG07YL" || deviceModel == "YLKG08YL"))
            {
                d = new DeviceYLKG07YL(deviceAddr, deviceName, this);
            }

            if (d)
            {
                //connect(d, &Device::deviceUpdated, this, &DeviceManager::refreshDevices_finished);
                //connect(d, &Device::deviceSynced, this, &DeviceManager::syncDevices_finished);

                m_devices_model->addDevice(d);
                //qDebug() << "* Device added (from database): " << deviceName << "/" << deviceAddr;
            }
        }
    }
}

DeviceManager::~DeviceManager()
{
    delete m_bluetoothAdapter;
    delete m_discoveryAgent;
    delete m_ble_params;

    delete m_devices_nearby_filter;
    delete m_devices_nearby_model;

    delete m_devices_filter;
    delete m_devices_model;
}

/* ************************************************************************** */
/* ************************************************************************** */

bool DeviceManager::hasBluetooth() const
{
    return (m_btA && m_btE);
}

bool DeviceManager::hasBluetoothAdapter() const
{
    return m_btA;
}

bool DeviceManager::hasBluetoothEnabled() const
{
    return m_btE;
}

bool DeviceManager::hasBluetoothPermissions() const
{
    return m_btP;
}

bool DeviceManager::isListening() const
{
    return m_listening;
}

bool DeviceManager::isScanning() const
{
    return m_scanning;
}

bool DeviceManager::isUpdating() const
{
    return false;
}

bool DeviceManager::isSyncing() const
{
    return false;
}

/* ************************************************************************** */

bool DeviceManager::checkBluetooth()
{
    //qDebug() << "DeviceManager::checkBluetooth()";

#if defined(Q_OS_IOS)
    checkBluetoothIos();
    return true;
#endif

    bool btA_was = m_btA;
    bool btE_was = m_btE;
    bool btP_was = m_btP;

    // Check availability
    if (m_bluetoothAdapter && m_bluetoothAdapter->isValid())
    {
        m_btA = true;

        if (m_bluetoothAdapter->hostMode() > QBluetoothLocalDevice::HostMode::HostPoweredOff)
        {
            m_btE = true;
        }
        else
        {
            m_btE = false;
            qDebug() << "Bluetooth adapter host mode:" << m_bluetoothAdapter->hostMode();
        }
    }
    else
    {
        m_btA = false;
        m_btE = false;
    }

    checkBluetoothPermissions();

    if (btA_was != m_btA || btE_was != m_btE || btP_was != m_btP)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    return (m_btA && m_btE);
}

void DeviceManager::enableBluetooth(bool enforceUserPermissionCheck)
{
    //qDebug() << "DeviceManager::enableBluetooth() enforce:" << enforceUserPermissionCheck;

#if defined(Q_OS_IOS)
    checkBluetoothIos();
    return;
#endif

    bool btA_was = m_btA;
    bool btE_was = m_btE;
    bool btP_was = m_btP;
/*
    // List Bluetooth adapters
    QList<QBluetoothHostInfo> adaptersList = QBluetoothLocalDevice::allDevices();
    if (adaptersList.size() > 0)
    {
        for (QBluetoothHostInfo a: adaptersList)
        {
            qDebug() << "- Bluetooth adapter:" << a.name();
        }
    }
    else
    {
        qDebug() << "> No Bluetooth adapter found...";
    }
*/
    // Invalid adapter? (ex: plugged off)
    if (m_bluetoothAdapter && !m_bluetoothAdapter->isValid())
    {
        delete m_bluetoothAdapter;
        m_bluetoothAdapter = nullptr;
    }

    // We only try the "first" available Bluetooth adapter
    // TODO // Handle multiple adapters?
    if (!m_bluetoothAdapter)
    {
        m_bluetoothAdapter = new QBluetoothLocalDevice();
        if (m_bluetoothAdapter)
        {
            // Keep us informed of Bluetooth adapter state change
            // On some platform, this can only inform us about disconnection, not reconnection
            connect(m_bluetoothAdapter, &QBluetoothLocalDevice::hostModeStateChanged,
                    this, &DeviceManager::bluetoothHostModeStateChanged);
        }
    }

    if (m_bluetoothAdapter && m_bluetoothAdapter->isValid())
    {
        m_btA = true;

        if (m_bluetoothAdapter->hostMode() > QBluetoothLocalDevice::HostMode::HostPoweredOff)
        {
            m_btE = true; // was already activated

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
            // Already powered on? Power on again anyway. It helps on android...
            //m_bluetoothAdapter->powerOn();
#endif
        }
        else // Try to activate the adapter
        {
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
            // mobile? check if we have the user's permission to do so
            if (enforceUserPermissionCheck)
            {
                SettingsManager *sm = SettingsManager::getInstance();
                if (sm && sm->getBluetoothControl())
                {
                    m_bluetoothAdapter->powerOn(); // Doesn't work on all platforms...
                }
            }
            else
#endif
            // desktop (or mobile but with user action)
            {
                Q_UNUSED(enforceUserPermissionCheck)
                m_bluetoothAdapter->powerOn(); // Doesn't work on all platforms...
            }
        }
    }
    else
    {
        m_btA = false;
        m_btE = false;
    }

    checkBluetoothPermissions();

    if (btA_was != m_btA || btE_was != m_btE || btP_was != m_btP)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }
}

bool DeviceManager::checkBluetoothPermissions()
{
    bool btP_was = m_btP;

    UtilsApp *utilsApp = UtilsApp::getInstance();
    m_btP = utilsApp->checkMobileBleLocationPermission();

    if (btP_was != m_btP)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    return m_btP;
}

/* ************************************************************************** */

void DeviceManager::bluetoothHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
    qDebug() << "DeviceManager::bluetoothHostModeStateChanged() host mode now:" << state;

    if (state > QBluetoothLocalDevice::HostPoweredOff)
    {
        m_btE = true;
    }
    else
    {
        m_btE = false;
    }

    Q_EMIT bluetoothChanged();
}

void DeviceManager::bluetoothStatusChanged()
{
    //qDebug() << "DeviceManager::bluetoothStatusChanged() bt adapter:" << m_btA << " /  bt enabled:" << m_btE;

    if (m_btA && m_btE)
    {
        listenDevices_start();
    }
    else
    {
        // Bluetooth disabled, force disconnection
        listenDevices_stop();
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::startBleAgent()
{
    //qDebug() << "DeviceManager::startBleAgent()";

    // BLE discovery agent
    if (!m_discoveryAgent)
    {
        m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
        if (m_discoveryAgent)
        {
            connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::errorOccurred),
                    this, &DeviceManager::deviceDiscoveryError, Qt::UniqueConnection);
        }
        else
        {
            qWarning() << "Unable to create BLE discovery agent...";
        }
    }
}

void DeviceManager::checkBluetoothIos()
{
    // iOS behave differently than all other platforms; there is no way to check
    // adapter status, only to start a device discovery and check for errors

    //qDebug() << "DeviceManager::checkBluetoothIos()";

    m_btA = true;

    if (m_discoveryAgent)
    {
        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   this, &DeviceManager::addBleDevice);
        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   this, &DeviceManager::updateBleDevice_simple);
        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                   this, &DeviceManager::updateBleDevice);

        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   this, &DeviceManager::addNearbyBleDevice);
        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                   this, &DeviceManager::updateNearbyBleDevice);

        disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                   this, &DeviceManager::deviceDiscoveryFinished);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &DeviceManager::bluetoothHostModeStateChangedIos, Qt::UniqueConnection);

        m_discoveryAgent->setLowEnergyDiscoveryTimeout(8); // 8ms
        m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

        if (m_discoveryAgent->isActive())
        {
            qDebug() << "Checking iOS Bluetooth...";
        }
    }
}

void DeviceManager::deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
    {
        qWarning() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";

        if (m_btE)
        {
            m_btE = false;
            //refreshDevices_stop();
            Q_EMIT bluetoothChanged();
        }
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
    {
        qWarning() << "deviceDiscoveryError() Writing or reading from the device resulted in an error.";

        m_btA = false;
        m_btE = false;
        //refreshDevices_stop();
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError)
    {
        qWarning() << "deviceDiscoveryError() Invalid Bluetooth adapter.";

        m_btA = false;

        if (m_btE)
        {
            m_btE = false;
            //refreshDevices_stop();
            Q_EMIT bluetoothChanged();
        }
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError)
    {
        qWarning() << "deviceDiscoveryError() Unsupported platform.";

        m_btA = false;
        m_btE = false;
        //refreshDevices_stop();
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod)
    {
        qWarning() << "deviceDiscoveryError() Unsupported Discovery Method.";

        m_btE = false;
        m_btP = false;
        //refreshDevices_stop();
        Q_EMIT bluetoothChanged();
    }
    else
    {
        qWarning() << "An unknown error has occurred.";

        m_btA = false;
        m_btE = false;
        //refreshDevices_stop();
        Q_EMIT bluetoothChanged();
    }
}

void DeviceManager::deviceDiscoveryFinished()
{
    //qDebug() << "DeviceManager::deviceDiscoveryFinished()";

    if (m_listening)
    {
        m_listening = false;
        Q_EMIT listeningChanged();
    }
}

void DeviceManager::deviceDiscoveryStopped()
{
    //qDebug() << "DeviceManager::deviceDiscoveryStopped()";

    if (m_listening)
    {
        m_listening = false;
        Q_EMIT listeningChanged();
    }
}

void DeviceManager::bluetoothHostModeStateChangedIos()
{
    //qDebug() << "DeviceManager::bluetoothHostModeStateChangedIos()";

    if (!m_btE)
    {
        m_btA = true;
        m_btE = true;
        Q_EMIT bluetoothChanged();
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::scanDevices_start()
{
    //qDebug() << "DeviceManager::scanDevices_start()";
}

void DeviceManager::scanDevices_stop()
{
    //qDebug() << "DeviceManager::scanDevices_stop()";
}

/* ************************************************************************** */

void DeviceManager::listenDevices_start()
{
    //qDebug() << "DeviceManager::listenDevices_start()";

    if (hasBluetooth())
    {
        if (!m_discoveryAgent)
        {
            startBleAgent();
        }

        if (m_discoveryAgent)
        {
            if (m_discoveryAgent->isActive())
            {
                m_discoveryAgent->stop();
            }

            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                       this, &DeviceManager::addNearbyBleDevice);
            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                       this, &DeviceManager::updateNearbyBleDevice);

            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                       this, &DeviceManager::addBleDevice);
            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                       this, &DeviceManager::deviceDiscoveryFinished);

            connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                    this, &DeviceManager::deviceDiscoveryStopped, Qt::UniqueConnection);
            connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                    this, &DeviceManager::deviceDiscoveryStopped, Qt::UniqueConnection);

            connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                    this, &DeviceManager::updateBleDevice_simple, Qt::UniqueConnection);
            connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                    this, &DeviceManager::updateBleDevice, Qt::UniqueConnection);

            m_discoveryAgent->setLowEnergyDiscoveryTimeout(ble_listening_duration);

            if (hasBluetoothPermissions())
            {
#if defined(Q_OS_ANDROID) && defined(QT_CONNECTIVITY_PATCHED)
                // Build and apply Android BLE scan filter, otherwise we can't scan while the screen is off
                // Needs a patched QtConnectivity (from https://github.com/emericg/qtconnectivity/tree/blescanfiltering_v1)
                if (m_daemonMode)
                {
                    QStringList filteredAddr;
                    for (auto d: qAsConst(m_devices_model->m_devices))
                    {
                        Device *dd = qobject_cast<Device*>(d);
                        if (dd) filteredAddr += dd->getAddress();
                    }
                    m_discoveryAgent->setAndroidScanFilter(filteredAddr); // WIP
                }
#endif // Q_OS_ANDROID

                m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

                if (m_discoveryAgent->isActive())
                {
                    m_listening = true;
                    Q_EMIT listeningChanged();
                    //qDebug() << "Listening for BLE advertisement devices...";
                }
            }
            else
            {
                qWarning() << "Cannot scan or listen without related Android permissions";
            }
        }
    }
}

void DeviceManager::listenDevices_stop()
{
    //qDebug() << "DeviceManager::listenDevices_stop()";
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::blacklistBleDevice(const QString &addr)
{
    qDebug() << "DeviceManager::blacklistBleDevice(" << addr << ")";

    if (m_dbInternal || m_dbExternal)
    {
        // if
        QSqlQuery queryDevice;
        queryDevice.prepare("SELECT deviceAddr FROM devicesBlacklist WHERE deviceAddr = :deviceAddr");
        queryDevice.bindValue(":deviceAddr", addr);
        queryDevice.exec();

        // then
        if (queryDevice.last() == false)
        {
            qDebug() << "+ Blacklisting device: " << addr;

            QSqlQuery blacklistDevice;
            blacklistDevice.prepare("INSERT INTO devicesBlacklist (deviceAddr) VALUES (:deviceAddr)");
            blacklistDevice.bindValue(":deviceAddr", addr);

            if (blacklistDevice.exec() == true)
            {
                m_devices_blacklist.push_back(addr);
                Q_EMIT devicesBlacklistUpdated();
            }
        }
    }
}

void DeviceManager::whitelistBleDevice(const QString &addr)
{
    qDebug() << "DeviceManager::whitelistBleDevice(" << addr << ")";

    if (m_dbInternal || m_dbExternal)
    {
        QSqlQuery whitelistDevice;
        whitelistDevice.prepare("DELETE FROM devicesBlacklist WHERE deviceAddr = :deviceAddr");
        whitelistDevice.bindValue(":deviceAddr", addr);

        if (whitelistDevice.exec() == true)
        {
            m_devices_blacklist.removeAll(addr);
            Q_EMIT devicesBlacklistUpdated();
        }
    }
}

bool DeviceManager::isBleDeviceBlacklisted(const QString &addr)
{
    if (m_dbInternal || m_dbExternal)
    {
        // if
        QSqlQuery queryDevice;
        queryDevice.prepare("SELECT deviceAddr FROM devicesBlacklist WHERE deviceAddr = :deviceAddr");
        queryDevice.bindValue(":deviceAddr", addr);
        queryDevice.exec();

        // then
        return queryDevice.last();
    }

    return false;
}

/* ************************************************************************** */

void DeviceManager::addBleDevice(const QBluetoothDeviceInfo &info)
{
    //qDebug() << "DeviceManager::addBleDevice()" << " > NAME" << info.name() << " > RSSI" << info.rssi();

    // Various sanity checks
    {
        if (info.rssi() >= 0) return; // we probably just hit the device cache

        if (m_devices_blacklist.contains(info.address().toString())) return; // device is blacklisted

        SettingsManager *sm = SettingsManager::getInstance();
        if (sm && sm->getBluetoothLimitScanningRange() && info.rssi() < -70) return; // device is too far away

        if ((info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) == false) return; // not a BLE device

        for (auto ed: qAsConst(m_devices_model->m_devices)) // device is already in the UI
        {
            Device *edd = qobject_cast<Device*>(ed);
            if (edd && (edd->getAddress() == info.address().toString() ||
                        edd->getAddress() == info.deviceUuid().toString()))
            {
                return;
            }
        }
    }

    Device *d = nullptr;

    if (info.name() == "PLAYBULB CANDLE" || info.name() == "MIPOW smart bulb" ||
        info.name() == "Pokemon PBP" || info.name() == "Pokemon GO Plus" ||
        info.name() == "YLKG" ||
        info.name() == "yee-rc")
    {
        // Check if it's not already in the UI
        for (auto ed: qAsConst(m_devices_model->m_devices))
        {
            Device *edd = qobject_cast<Device*>(ed);
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
            if (edd && edd->getAddress() == info.deviceUuid().toString())
#else
            if (edd && edd->getAddress() == info.address().toString())
#endif
            {
                return;
            }
        }

        // Detect the model id
        QString modelID;
        if (info.name() == "yee-rc")
        {
            const QList<QBluetoothUuid> &serviceIds = info.serviceIds();
            for (const auto id: serviceIds)
            {
                if (id.toUInt16() == 0xFE95)
                {
                    const quint8 *data = reinterpret_cast<const quint8 *>(info.serviceData(id).constData());

                    int ProductID = data[2] + (data[3] << 8);
                    qDebug() << "HO YEAH" << ProductID;

                    if (ProductID == 339) modelID = "YLYK01YL";
                    if (ProductID == 950) modelID = "YLKG07YL";
                }
            }
        }

        // Create the device
        if (info.name() == "PLAYBULB CANDLE" || info.name() == "MIPOW smart bulb")
        {
            d = new DeviceMiPow(info, this);
        }
        else if (info.name() == "Pokemon PBP" || info.name() == "Pokemon GO Plus")
        {
            //d = new DevicePokeballPlus(info, this);
        }
        else if (info.name() == "YLKG")
        {
            d = new DeviceYLAI003(info, this);
        }
        else if (info.name() == "yee-rc" && modelID == "YLYK01YL")
        {
            d = new DeviceYLYK01YL(info, this);
        }
        else if (info.name() == "yee-rc" && (modelID == "YLKG07YL" || modelID == "YLKG08YL"))
        {
            d = new DeviceYLKG07YL(info, this);
        }
    }

    if (d)
    {
        // Add it to the database?
        if (m_dbInternal || m_dbExternal)
        {
            // if
            QSqlQuery queryDevice;
            queryDevice.prepare("SELECT deviceName FROM devices WHERE deviceAddr = :deviceAddr");
            queryDevice.bindValue(":deviceAddr", d->getAddress());
            queryDevice.exec();

            // then
            if (queryDevice.last() == false)
            {
                qDebug() << "+ Adding device: " << d->getName() << "/" << d->getAddress() << "to local database";

                QSqlQuery addDevice;
                addDevice.prepare("INSERT INTO devices (deviceAddr, deviceModel, deviceName) VALUES (:deviceAddr, :deviceModel, :deviceName)");
                addDevice.bindValue(":deviceAddr", d->getAddress());
                addDevice.bindValue(":deviceModel", d->getModel());
                addDevice.bindValue(":deviceName", d->getName());
                addDevice.exec();
            }
        }

        // Add it to the UI
        m_devices_model->addDevice(d);
        Q_EMIT devicesListUpdated();

        qDebug() << "Device added (from BLE discovery): " << d->getName() << "/" << d->getAddress();
    }
    else
    {
        //qDebug() << "Unsupported device: " << info.name() << "/" << info.address();
    }
}

void DeviceManager::disconnectDevices()
{
    //qDebug() << "DeviceManager::disconnectDevices()";

    for (auto d: qAsConst(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);
        dd->deviceDisconnect();
    }
}

void DeviceManager::removeDevice(const QString &address)
{
    for (auto d: qAsConst(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);

        if (dd->getAddress() == address)
        {
            qDebug() << "- Removing device: " << dd->getName() << "/" << dd->getAddress() << "from local database";

            // Make sure its not being used
            dd->refreshStop();

            // Remove from database // Don't remove the actual data, nor the limits
            if (m_dbInternal || m_dbExternal)
            {
                QSqlQuery removeDevice;
                removeDevice.prepare("DELETE FROM devices WHERE deviceAddr = :deviceAddr");
                removeDevice.bindValue(":deviceAddr", dd->getAddress());
                if (removeDevice.exec() == false)
                {
                    qWarning() << "> removeDevice.exec() ERROR"
                               << removeDevice.lastError().type() << ":" << removeDevice.lastError().text();
                }
            }

            // Remove device
            m_devices_model->removeDevice(dd);
            Q_EMIT devicesListUpdated();

            break;
        }
    }
}

void DeviceManager::removeDeviceData(const QString &address)
{
    for (auto d: qAsConst(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);

        if (dd->getAddress() == address)
        {
            qDebug() << "- Removing device data: " << dd->getName() << "/" << dd->getAddress() << "from local database";

            // Remove the actual data & limits
            if (m_dbInternal || m_dbExternal)
            {
                // TODO
            }

            break;
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::invalidate()
{
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_manual()
{
    m_devices_filter->setSortRole(DeviceModel::DeviceModelRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_model()
{
    m_devices_filter->setSortRole(DeviceModel::DeviceModelRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_name()
{
    m_devices_filter->setSortRole(DeviceModel::DeviceNameRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_location()
{
    m_devices_filter->setSortRole(DeviceModel::AssociatedLocationRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_waterlevel()
{
    m_devices_filter->setSortRole(DeviceModel::SoilMoistureRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_plant()
{
    m_devices_filter->setSortRole(DeviceModel::PlantNameRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

void DeviceManager::orderby_insideoutside()
{
    m_devices_filter->setSortRole(DeviceModel::InsideOutsideRole);
    m_devices_filter->sort(0, Qt::AscendingOrder);
    m_devices_filter->invalidate();
}

/* ************************************************************************** */
