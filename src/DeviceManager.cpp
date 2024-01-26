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
#include "devices/device_pokeballplus.h"
#include "devices/device_pokemongoplus.h"
#include "devices/device_ylai003.h"
#include "devices/device_ylkg07yl.h"
#include "devices/device_ylyk01yl.h"

#include "utils_app.h"
#include "utils_log.h"

#include <QList>
#include <QDateTime>
#include <QDebug>

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
#if QT_CONFIG(permissions)
#include <QGuiApplication>
#include <QPermissions>
#endif
#endif

/* ************************************************************************** */

DeviceManager::DeviceManager(bool daemon)
{
    m_daemonMode = daemon;

    // Data model init (unified)
    m_devices_model = new DeviceModel(this);
    m_devices_filter = new DeviceFilter(this);
    m_devices_filter->setSourceModel(m_devices_model);
    m_devices_filter->setDynamicSortFilter(true);

    // Data model filtering
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
    enableBluetooth(true); // Enables adapter // ONLY if off and permission given
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
        // Load device blacklist
        if (!m_daemonMode)
        {
            QSqlQuery queryBlacklist;
            queryBlacklist.exec("SELECT deviceAddr FROM devicesBlacklist");
            while (queryBlacklist.next())
            {
                m_devices_blacklist.push_back(queryBlacklist.value(0).toString());
            }
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
            else if (deviceName == "Pokemon PBP")
            {
                d = new DevicePokeballPlus(deviceAddr, deviceName, this);
            }
            else if (deviceName == "Pokemon GO Plus")
            {
                d = new DevicePokemonGoPlus(deviceAddr, deviceName, this);
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
                m_devices_model->addDevice(d);
                //qDebug() << "* Device added (from database): " << deviceName << "/" << deviceAddr;
            }
        }
    }
}

DeviceManager::~DeviceManager()
{
    qDeleteAll(m_bluetoothAdapters);
    m_bluetoothAdapters.clear();

    delete m_bluetoothAdapter;
    delete m_discoveryAgent;

    delete m_devices_nearby_filter;
    delete m_devices_nearby_model;

    delete m_devices_filter;
    delete m_devices_model;
}

/* ************************************************************************** */
/* ************************************************************************** */

bool DeviceManager::checkBluetooth()
{
    //qDebug() << "DeviceManager::checkBluetooth()";

#if defined(Q_OS_IOS)
    // at this point we don't actually try to use checkBluetoothIOS() or we will
    // be caugth in a loop with the OS notifying the user that BLE wants to start
    // but is off, then giving back the focus to the app, thus calling checkBluetooth()...
    return m_bleEnabled;
#endif

    bool btA_was = m_bleAdapter;
    bool btE_was = m_bleEnabled;
    bool btP_was = m_blePermissions;

    // Check adapter availability
    if (m_bluetoothAdapter && m_bluetoothAdapter->isValid())
    {
        m_bleAdapter = true;

        if (m_bluetoothAdapter->hostMode() > QBluetoothLocalDevice::HostMode::HostPoweredOff)
        {
            m_bleEnabled = true;
        }
        else
        {
            m_bleEnabled = false;
            qDebug() << "Bluetooth adapter host mode:" << m_bluetoothAdapter->hostMode();
        }
    }
    else
    {
        m_bleAdapter = false;
        m_bleEnabled = false;
    }

    // Check OS permissions
    checkBluetoothPermissions();

    if (btA_was != m_bleAdapter || btE_was != m_bleEnabled || btP_was != m_blePermissions)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    return (m_bleAdapter && m_bleEnabled && m_blePermissions);
}

bool DeviceManager::enableBluetooth(bool enforceUserPermissionCheck)
{
    //qDebug() << "DeviceManager::enableBluetooth() enforce:" << enforceUserPermissionCheck;

#if defined(Q_OS_IOS)
    checkBluetoothIOS();
    return false;
#endif

    bool btA_was = m_bleAdapter;
    bool btE_was = m_bleEnabled;
    bool btP_was = m_blePermissions;

    // Invalid adapter? (ex: plugged off)
    if (m_bluetoothAdapter && !m_bluetoothAdapter->isValid())
    {
        qDebug() << "DeviceManager::enableBluetooth() deleting current adapter";

        disconnect(m_bluetoothAdapter, &QBluetoothLocalDevice::hostModeStateChanged,
                   this, &DeviceManager::bluetoothHostModeStateChanged);

        delete m_bluetoothAdapter;
        m_bluetoothAdapter = nullptr;
    }

    // Select an adapter (if none currently selected)
    if (!m_bluetoothAdapter)
    {
        // Correspond to the "first available" or "default" Bluetooth adapter
        m_bluetoothAdapter = new QBluetoothLocalDevice();
        if (m_bluetoothAdapter)
        {
            // Keep us informed of Bluetooth adapter state change
            // On some platform, this can only inform us about disconnection, not reconnection
            connect(m_bluetoothAdapter, &QBluetoothLocalDevice::hostModeStateChanged,
                    this, &DeviceManager::bluetoothHostModeStateChanged);
        }
    }

    // Check adapter availability
    if (m_bluetoothAdapter && m_bluetoothAdapter->isValid())
    {
        m_bleAdapter = true;

        if (m_bluetoothAdapter->hostMode() > QBluetoothLocalDevice::HostMode::HostPoweredOff)
        {
            // Was already activated
            m_bleEnabled = true;
        }
        else
        {
            // Try to activate the adapter

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
        qWarning() << "DeviceManager::enableBluetooth() we have an invalid adapter";
        m_bleAdapter = false;
        m_bleEnabled = false;
    }

    //qDebug() << "DeviceManager::enableBluetooth() recap";
    //qDebug() << " - bluetooth" << hasBluetooth();
    //qDebug() << " - bleAdapter" << m_bleAdapter;
    //qDebug() << " - bleEnabled" << m_bleEnabled;
    //qDebug() << " - blePermissions" << m_blePermissions;

    // Check OS permissions
    checkBluetoothPermissions();

    if (btA_was != m_bleAdapter || btE_was != m_bleEnabled || btP_was != m_blePermissions)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    return (m_bleAdapter && m_bleEnabled && m_blePermissions);
}

bool DeviceManager::checkBluetoothPermissions()
{
    //qDebug() << "DeviceManager::checkBluetoothPermissions()";

#if defined(Q_OS_ANDROID)
    //
#elif defined(Q_OS_LINUX) || defined(Q_OS_WINDOWS)
    // These OS don't ask for any particular permissions
    m_permOS = true;
    m_blePermissions = true;
#endif

#if !defined(Q_OS_ANDROID)
    // The location permission(s) debacle is Android only
    m_permLocationBLE = true;
    m_permLocationBKG = true;
    m_permGPS = true;
#endif

    bool os_was = m_permOS;
    bool loc_was = m_permLocationBLE;
    bool loc_bg_was = m_permLocationBKG;
    bool gps_was = m_permGPS;
    bool btP_was = m_blePermissions;

#if defined(Q_OS_ANDROID)
    m_permOS = UtilsApp::checkMobileBluetoothPermission();
    m_permLocationBLE = UtilsApp::checkMobileBleLocationPermission();
    m_permLocationBKG = UtilsApp::checkMobileBackgroundLocationPermission();
    m_permGPS = UtilsApp::isMobileGpsEnabled();

    m_blePermissions = m_permOS && m_permLocationBLE;
#endif

    // Android 12 ?
    //QLocationPermission::setAccuracy(QLocationPermission::Precise);
    //QBluetoothPermission::setCommunicationModes(QBluetoothPermission::Default);

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#if QT_CONFIG(permissions)
    if (qApp)
    {
        switch (qApp->checkPermission(QBluetoothPermission{}))
        {
        case Qt::PermissionStatus::Undetermined:
        case Qt::PermissionStatus::Denied:
            m_permOS = false;
            m_blePermissions = false;
            break;
        case Qt::PermissionStatus::Granted:
            m_permOS = true;
            m_blePermissions = true;
            break;
        }
    }
#endif // QT_CONFIG(permissions)
#endif // defined(Q_OS_MACOS) || defined(Q_OS_IOS)

    if (os_was != m_permOS || gps_was != m_permGPS ||
        loc_was != m_permLocationBLE || loc_bg_was != m_permLocationBKG)
    {
        // this function did change the Bluetooth permission
        Q_EMIT permissionsChanged();
    }
    if (btP_was != m_blePermissions)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    //qDebug() << "DeviceManager::checkBluetoothPermissions() recap";
    //qDebug() << " - bluetooth" << hasBluetooth();
    //qDebug() << " - bleAdapter" << m_bleAdapter;
    //qDebug() << " - bleEnabled" << m_bleEnabled;
    //qDebug() << " - blePermissions" << m_blePermissions;
    //qDebug() << " - permOS" << m_permOS;
    //qDebug() << " - permLocationBLE" << m_permLocationBLE;
    //qDebug() << " - permLocationBKG" << m_permLocationBKG;
    //qDebug() << " - permGPS" << m_permGPS;

    return m_blePermissions;
}

bool DeviceManager::requestBluetoothPermissions()
{
    //qDebug() << "DeviceManager::requestBluetoothPermissions()";

#if defined(Q_OS_ANDROID)
#if QT_CONFIG(permissions) && QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)

    // qApp->checkPermission(QBluetoothPermission{}) doesn't work on Android
    // so we do it ourselves, the old fashioned way...

    bool permLocationBLE = UtilsApp::checkMobileBleLocationPermission();
    bool permOS = UtilsApp::checkMobileBluetoothPermission();

    if (!permLocationBLE || !permOS)
    {
        if (qApp)
        {
            qApp->requestPermission(QBluetoothPermission{}, this, &DeviceManager::requestBluetoothPermissions_results);
        }
    }

#else // QT_CONFIG(permissions)

    m_permOS = UtilsApp::getMobileBluetoothPermission();
    m_permLocationBLE = UtilsApp::getMobileBleLocationPermission();
    m_blePermissions = m_permOS && m_permLocationBLE;

#endif // QT_CONFIG(permissions)
#endif // defined(Q_OS_ANDROID)

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#if QT_CONFIG(permissions)

    if (qApp)
    {
        switch (qApp->checkPermission(QBluetoothPermission{}))
        {
        case Qt::PermissionStatus::Undetermined:
            qDebug() << "Qt::PermissionStatus::Undetermined";
            qApp->requestPermission(QBluetoothPermission{}, this, &DeviceManager::requestBluetoothPermissions_results);
            break;
        case Qt::PermissionStatus::Granted:
            qDebug() << "Qt::PermissionStatus::Granted";
            m_permOS = true;
            m_blePermissions = true;
            break;
        case Qt::PermissionStatus::Denied:
            qDebug() << "Qt::PermissionStatus::Denied";
            m_permOS = false;
            m_blePermissions = false;
            break;
        }
    }

#endif // QT_CONFIG(permissions)
#endif // defined(Q_OS_MACOS) || defined(Q_OS_IOS)

    //qDebug() << "DeviceManager::requestBluetoothPermissions() recap";
    //qDebug() << " - bluetooth" << hasBluetooth();
    //qDebug() << " - bleAdapter" << m_bleAdapter;
    //qDebug() << " - bleEnabled" << m_bleEnabled;
    //qDebug() << " - blePermissions" << m_blePermissions;
    //qDebug() << " - permOS" << m_permOS;
    //qDebug() << " - permLocationBLE" << m_permLocationBLE;
    //qDebug() << " - permLocationBKG" << m_permLocationBKG;
    //qDebug() << " - permGPS" << m_permGPS;

    return m_blePermissions;
}

void DeviceManager::requestBluetoothPermissions_results()
{
    // evaluate the results
#if defined(Q_OS_IOS)
    checkBluetoothPermissions();
#else
    checkBluetooth();
#endif

    if (m_blePermissions)
    {
        // try enabling the adapter
        if (!m_bleAdapter || !m_bleEnabled)
        {
            enableBluetooth();
        }
    }
    else
    {
        // try again?
        //requestBluetoothPermissions();
    }
}

/* ************************************************************************** */

void DeviceManager::bluetoothHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
    //qDebug() << "DeviceManager::bluetoothHostModeStateChanged() host mode now:" << state;

    if (state != m_ble_hostmode)
    {
        m_ble_hostmode = state;
        Q_EMIT hostModeChanged();
    }

    if (state > QBluetoothLocalDevice::HostPoweredOff)
    {
        m_bleEnabled = true;
    }
    else
    {
        m_bleEnabled = false;
    }

    Q_EMIT bluetoothChanged();
}

void DeviceManager::bluetoothStatusChanged()
{
    //qDebug() << "DeviceManager::bluetoothStatusChanged() bt adapter:" << m_bleAdapter << " /  bt enabled:" << m_bleEnabled;

    if (m_bleAdapter && m_bleEnabled)
    {
        // Bluetooth enabled, re/start listening
        if (m_devices_model->hasDevices())
        {
            listenDevices_start();
        }
        else
        {
            // start scanning?
            //scanDevices_start();
        }
    }
    else
    {
        // Bluetooth disabled, force disconnection
        refreshDevices_stop();
    }
}

void DeviceManager::bluetoothPermissionsChanged()
{
    //qDebug() << "DeviceManager::bluetoothPermissionsChanged()";

    if (m_bleAdapter && m_bleEnabled)
    {
        checkBluetooth();
    }
    else
    {
        enableBluetooth();
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
            //qDebug() << "Scanning method supported:" << m_discoveryAgent->supportedDiscoveryMethods();

            connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                    this, &DeviceManager::deviceDiscoveryError);
        }
        else
        {
            qWarning() << "Unable to create BLE discovery agent...";
        }
    }
}

void DeviceManager::checkBluetoothIOS()
{
    //qDebug() << "DeviceManager::checkBluetoothIOS()";

    // iOS behave differently than all other platforms; there is no way to check
    // adapter status, we can only to start a device discovery and check if it fails

    // the thing is, when the discovery is started with the BLE adapter turned off,
    // it will actually take up to 30s to report that fact... so after a short while,
    // we check on our own if the discovery agent is still running or not using a timer

    // when the BLE adapter is turned off while the discovery is already running,
    // the error is reported instantly though

    m_bleAdapter = true; // there is no iOS device without a BLE adapter

    m_permOS = true; // TODO
    m_blePermissions = m_permOS;

    // not necessary on iOS // set everything to true
    m_permLocationBLE = true;
    m_permLocationBKG = true;
    m_permGPS = true;

    if (!m_discoveryAgent)
    {
        startBleAgent();
    }
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

        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &DeviceManager::deviceDiscoveryFinished, Qt::UniqueConnection);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                this, &DeviceManager::deviceDiscoveryStopped, Qt::UniqueConnection);

        m_discoveryAgent->setLowEnergyDiscoveryTimeout(8); // 8ms
        m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

        if (m_discoveryAgent->isActive())
        {
            qDebug() << "Checking iOS Bluetooth...";

            // this ensure no other function will be able to use the discovery agent while this check is running
            m_checking_ios_ble = true;
            m_bleEnabled = false;

            // this ensure that we catch error as soon as possible (~333ms) and not ~30s later when the OS think we should know
            connect(&m_checking_ios_timer, &QTimer::timeout, this,
                    &DeviceManager::deviceDiscoveryErrorIOS, Qt::UniqueConnection);
            m_checking_ios_timer.setSingleShot(true);
            m_checking_ios_timer.start(333);
        }
    }
}

void DeviceManager::deviceDiscoveryErrorIOS()
{
    //qDebug() << "DeviceManager::deviceDiscoveryErrorIOS()";

    if (m_discoveryAgent) m_discoveryAgent->stop();
    m_checking_ios_ble = false;

    if (m_bleEnabled)
    {
        m_bleEnabled = false;
        Q_EMIT bluetoothChanged();
    }
}

void DeviceManager::deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error <= QBluetoothDeviceDiscoveryAgent::NoError) return;

    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
    {
        qWarning() << "The Bluetooth adaptor is powered off, power it on before doing discovery.";

        if (m_bleEnabled)
        {
            m_bleEnabled = false;
            Q_EMIT bluetoothChanged();
        }
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
    {
        qWarning() << "deviceDiscoveryError() Writing or reading from the device resulted in an error.";

        m_bleAdapter = false;
        m_bleEnabled = false;
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError)
    {
        qWarning() << "deviceDiscoveryError() Invalid Bluetooth adapter.";

        m_bleAdapter = false;

        if (m_bleEnabled)
        {
            m_bleEnabled = false;
            Q_EMIT bluetoothChanged();
        }
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError)
    {
        qWarning() << "deviceDiscoveryError() Unsupported Platform.";

        m_bleAdapter = false;
        m_bleEnabled = false;
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod)
    {
        qWarning() << "deviceDiscoveryError() Unsupported Discovery Method.";

        m_bleEnabled = false;
        m_blePermissions = false;
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::LocationServiceTurnedOffError)
    {
        qWarning() << "deviceDiscoveryError() Location Service Turned Off Error.";

        m_bleEnabled = false;
        m_blePermissions = false;
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::MissingPermissionsError)
    {
        qWarning() << "deviceDiscoveryError() Missing Permissions Error.";

        m_bleEnabled = false;
        m_blePermissions = false;
        Q_EMIT bluetoothChanged();
    }
    else
    {
        qWarning() << "An unknown error has occurred.";

        m_bleAdapter = false;
        m_bleEnabled = false;
        Q_EMIT bluetoothChanged();
    }

    listenDevices_stop();
    refreshDevices_stop();
    scanDevices_stop();

    if (m_scanning)
    {
        m_scanning = false;
        Q_EMIT scanningChanged();
    }
    if (m_listening)
    {
        m_listening = false;
        Q_EMIT listeningChanged();
    }
}

void DeviceManager::deviceDiscoveryFinished()
{
    //qDebug() << "DeviceManager::deviceDiscoveryFinished()";

#if defined(Q_OS_IOS)
    if (m_checking_ios_ble)
    {
        m_checking_ios_ble = false;
        m_checking_ios_timer.stop();

        if (!m_bleEnabled)
        {
            m_bleEnabled = true;
            Q_EMIT bluetoothChanged();
        }
    }
#endif

    if (m_scanning)
    {
        m_scanning = false;
        Q_EMIT scanningChanged();
    }
    if (m_listening)
    {
        m_listening = false;
        Q_EMIT listeningChanged();
    }
}

void DeviceManager::deviceDiscoveryStopped()
{
    qDebug() << "DeviceManager::deviceDiscoveryStopped()";
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::scanDevices_start()
{
    //qDebug() << "DeviceManager::scanDevices_start()";

    if (hasBluetooth())
    {
        if (!m_discoveryAgent)
        {
            startBleAgent();
        }
        if (m_discoveryAgent)
        {
            if (m_discoveryAgent->isActive() && m_scanning)
            {
                qWarning() << "DeviceManager::scanDevices_start() already scanning?";
            }
            else
            {
                disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                           this, &DeviceManager::addNearbyBleDevice);
                disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                           this, &DeviceManager::updateNearbyBleDevice);

                connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                        this, &DeviceManager::deviceDiscoveryFinished, Qt::UniqueConnection);
                connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                        this, &DeviceManager::deviceDiscoveryStopped, Qt::UniqueConnection);

                connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                        this, &DeviceManager::addBleDevice, Qt::UniqueConnection);
                connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                        this, &DeviceManager::updateBleDevice, Qt::UniqueConnection);

                m_discoveryAgent->setLowEnergyDiscoveryTimeout(ble_scanning_duration*1000);
                m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

                if (m_discoveryAgent->isActive())
                {
                    m_scanning = true;
                    Q_EMIT scanningChanged();
                    qDebug() << "Scanning for new devices...";
                }
                else
                {
                    qWarning() << "DeviceManager::scanDevices_start() DID NOT START";
                }
            }
        }
    }
    else
    {
        qWarning() << "Cannot scan or listen without BLE or BLE permissions";
    }
}

void DeviceManager::scanDevices_stop()
{
    //qDebug() << "DeviceManager::scanDevices_stop()";

    if (m_discoveryAgent)
    {
        if (m_discoveryAgent->isActive())
        {
            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                       this, &DeviceManager::addBleDevice);
            disconnect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                       this, &DeviceManager::updateBleDevice);

            m_discoveryAgent->stop();

            if (m_scanning)
            {
                m_scanning = false;
                Q_EMIT scanningChanged();
            }
        }
    }
}

/* ************************************************************************** */

void DeviceManager::refreshDevices_start()
{
    //qDebug() << "DeviceManager::refreshDevices_start()";
}

void DeviceManager::refreshDevices_stop()
{
    //qDebug() << "DeviceManager::refreshDevices_stop()";
}

/* ************************************************************************** */

void DeviceManager::listenDevices_start()
{
    //qDebug() << "DeviceManager::listenDevices_start()";

#if defined(Q_OS_ANDROID)
    // An Android service won't be able to scan/listen in the background without the associated permission
    if (m_daemonMode && !m_permLocationBKG) return;
#endif

    if (hasBluetooth())
    {
        if (!m_discoveryAgent)
        {
            startBleAgent();
        }
        if (m_discoveryAgent)
        {
            if (m_discoveryAgent->isActive() && m_scanning)
            {
                m_discoveryAgent->stop();
                m_scanning = false;
                Q_EMIT scanningChanged();
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

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
            m_discoveryAgent->setLowEnergyDiscoveryTimeout(ble_listening_duration_mobile*1000);
#else
            m_discoveryAgent->setLowEnergyDiscoveryTimeout(ble_listening_duration_desktop*1000);
#endif

            if (hasBluetoothPermissions())
            {
#if defined(Q_OS_ANDROID) && defined(QT_CONNECTIVITY_PATCHED)
                // Build and apply Android BLE scan filter, otherwise we can't scan while the screen is off
                // Needs a patched QtConnectivity (from https://github.com/emericg/qtconnectivity/tree/blescanfiltering_v1)
                if (m_daemonMode)
                {
                    QStringList filteredAddr;
                    for (auto d: std::as_const(m_devices_model->m_devices))
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
        //if ((info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) == false) return; // not a BLE device
        if (m_devices_blacklist.contains(info.address().toString())) return; // device is blacklisted
        if (m_devices_blacklist.contains(info.deviceUuid().toString())) return; // device is blacklisted

        SettingsManager *sm = SettingsManager::getInstance();
        if (sm && sm->getBluetoothLimitScanningRange() && info.rssi() < -70) return; // device is too far away

        if ((info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) == false) return; // not a BLE device

        for (auto ed: std::as_const(m_devices_model->m_devices)) // device is already in the UI
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
        for (auto ed: std::as_const(m_devices_model->m_devices))
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
        else if (info.name() == "Pokemon PBP")
        {
            d = new DevicePokeballPlus(info, this);
        }
        else if (info.name() == "Pokemon GO Plus")
        {
            d = new DevicePokemonGoPlus(info, this);
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

                if (addDevice.exec() == false)
                {
                    qWarning() << "> addDevice.exec() ERROR"
                               << addDevice.lastError().type() << ":" << addDevice.lastError().text();
                }
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

    for (auto d: std::as_const(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);
        dd->deviceDisconnect();
    }
}

void DeviceManager::removeDevice(const QString &address)
{
    for (auto d: std::as_const(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);

        if (dd->getAddress() == address)
        {
            qDebug() << "- Removing device: " << dd->getName() << "/" << dd->getAddress() << "from local database";
/*
            // Make sure its not being used
            disconnect(dd, &Device::deviceUpdated, this, &DeviceManager::refreshDevices_finished);
            disconnect(dd, &Device::deviceSynced, this, &DeviceManager::syncDevices_finished);
            dd->refreshStop();
            refreshDevices_finished(dd);
*/
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
            m_devices_model->removeDevice(dd, true);
            Q_EMIT devicesListUpdated();

            break;
        }
    }
}

void DeviceManager::removeDeviceData(const QString &address)
{
    for (auto d: std::as_const(m_devices_model->m_devices))
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

void DeviceManager::invalidateFilter()
{
    m_devices_filter->invalidatefilter();
}

void DeviceManager::orderby(int role, Qt::SortOrder order)
{
    m_devices_filter->setSortRole(role);
    m_devices_filter->sort(0, order);
    m_devices_filter->invalidate();
}

/* ************************************************************************** */

void DeviceManager::orderby_manual()
{
    orderby(DeviceModel::ManualIndexRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_model()
{
    orderby(DeviceModel::DeviceModelRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_name()
{
    orderby(DeviceModel::DeviceNameRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_location()
{
    orderby(DeviceModel::AssociatedLocationRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_waterlevel()
{
    orderby(DeviceModel::SoilMoistureRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_plant()
{
    orderby(DeviceModel::PlantNameRole, Qt::AscendingOrder);
}

void DeviceManager::orderby_insideoutside()
{
    orderby(DeviceModel::InsideOutsideRole, Qt::AscendingOrder);
}

/* ************************************************************************** */
