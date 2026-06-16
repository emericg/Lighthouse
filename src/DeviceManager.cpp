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

#include <thread>
#include <chrono>

#include <QCoreApplication>
#include <QDateTime>
#include <QList>
#include <QDebug>

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>

#if QT_CONFIG(permissions)
#include <QGuiApplication>
#include <QPermissions>
#endif

#if defined(Q_OS_ANDROID)
#include "utils_app.h"
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
    checkBluetoothPermissions();
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
    delete m_bluetoothDiscoveryAgent;

    delete m_devices_nearby_filter;
    delete m_devices_nearby_model;

    delete m_devices_filter;
    delete m_devices_model;
}

/* ************************************************************************** */
/* ************************************************************************** */

bool DeviceManager::areDevicesConnected() const
{
    for (auto d: std::as_const(m_devices_model->m_devices))
    {
        if (d && d->isConnected())
        {
            return true;
        }
    }

    qDebug() << "DeviceManager::areDevicesConnected() FALSE";

    return false;
}

void DeviceManager::disconnectDevices() const
{
    qDebug() << "DeviceManager::disconnectDevices()";

    for (auto d: std::as_const(m_devices_model->m_devices))
    {
        Device *dd = qobject_cast<Device*>(d);
        dd->actionDisconnect();
    }
}

void DeviceManager::disconnectAndExit() const
{
    if (areDevicesConnected())
    {
        qDebug() << "DeviceManager::disconnectAndExit()";

        disconnectDevices();

        int timeout = 60;

        while (areDevicesConnected() && timeout > 0)
        {
            qApp->processEvents();
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
            timeout--;
        }
    }
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

    bool btA_was = m_bleAdapter && m_bluetoothAdapter->isValid();
    bool btE_was = m_bleEnabled;
    bool btP_was = hasBluetoothPermissions();

    // Check permissions
    checkBluetoothPermissions();

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
            qWarning() << "Bluetooth adapter host mode:" << m_bluetoothAdapter->hostMode();
        }
    }
    else
    {
        m_bleAdapter = false;
        m_bleEnabled = false;
        qWarning() << "Bluetooth adapter INVALID";
    }

    if (btA_was != m_bleAdapter || btE_was != m_bleEnabled || btP_was != hasBluetoothPermissions())
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();

        // let's see if we can turn on the adapter now
        enableBluetooth(true);
    }

    return (m_bleAdapter && m_bleEnabled && hasBluetoothPermissions());
}

bool DeviceManager::enableBluetooth(bool enforceUserPermissionCheck)
{
    //qDebug() << "DeviceManager::enableBluetooth() enforce:" << enforceUserPermissionCheck;

#if defined(Q_OS_IOS)
    checkBluetoothIOS();
    return false;
#endif

    bool btA_was = m_bleAdapter && m_bluetoothAdapter->isValid();
    bool btE_was = m_bleEnabled;
    bool btP_was = hasBluetoothPermissions();

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
        qDebug() << "DeviceManager::enableBluetooth() creating new adapter";

        // Correspond to the "first available" or "default" Bluetooth adapter
        m_bluetoothAdapter = new QBluetoothLocalDevice();
        if (m_bluetoothAdapter)
        {
            // Keep us informed of Bluetooth adapter state change
            // On some platform, this can only inform us about disconnection, not reconnection
            connect(m_bluetoothAdapter, &QBluetoothLocalDevice::hostModeStateChanged,
                    this, &DeviceManager::bluetoothHostModeStateChanged);

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
            if (!m_bluetoothAdapter->name().isEmpty())
            {
                qDebug() << "Bluetooth adapter name:" << m_bluetoothAdapter->name();
                SettingsManager::getInstance()->setNetClientName(m_bluetoothAdapter->name());
            }
#endif
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

    if (btA_was != m_bleAdapter || btE_was != m_bleEnabled || btP_was != hasBluetoothPermissions())
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();
    }

    //qDebug() << "DeviceManager::enableBluetooth() >> RECAP";
    //qDebug() << " - bluetooth" << hasBluetooth();
    //qDebug() << " - bleAdapter" << m_bleAdapter;
    //qDebug() << " - bleEnabled" << m_bleEnabled;
    //qDebug() << " - blePermissions" << hasBluetoothPermissions();

    return (m_bleAdapter && m_bleEnabled && hasBluetoothPermissions());
}

bool DeviceManager::checkBluetoothPermissions()
{
    //qDebug() << "DeviceManager::checkBluetoothPermissions()";

#if defined(Q_OS_ANDROID)
    //
#elif defined(Q_OS_LINUX) || defined(Q_OS_WINDOWS)
    // These OS don't ask for any particular permissions
    m_blePermission = true;
#endif

    bool ble_was = m_blePermission;
    bool loc_fg_was = m_locPermission_foreground;
    bool loc_bg_was = m_locPermission_background;
    bool gps_was = m_gpsEnabled;

    m_blePermission = checkBluetoothPermission();

#if defined(Q_OS_ANDROID)
    if (UtilsApp::getInstance()->getAndroidSdkVersion() <= 30)
    {
        m_locPermission_foreground = UtilsApp::checkMobileBleLocationPermission();
        m_locPermission_background = UtilsApp::checkMobileBackgroundLocationPermission();
        m_gpsEnabled = UtilsApp::isMobileGpsEnabled();
    }
#endif

    if (ble_was != m_blePermission)
    {
        // this function did change the Bluetooth permission
        Q_EMIT permissionsChanged();

        // let's see if we can turn on the adapter now
        enableBluetooth(true);
    }
    if (loc_fg_was != m_locPermission_foreground || loc_bg_was != m_locPermission_background || gps_was != m_gpsEnabled)
    {
        // this function did change the Bluetooth permission
        Q_EMIT permissionsChanged();

        // let's see if we can turn on the adapter now
        enableBluetooth(true);
    }
    if (ble_was != m_blePermission)
    {
        // this function did changed the Bluetooth adapter status
        Q_EMIT bluetoothChanged();

        // let's see if we can turn on the adapter now
        enableBluetooth(true);
    }

    //qDebug() << "DeviceManager::checkBluetoothPermissions() >> RECAP";
    //qDebug() << " - bluetooth" << hasBluetooth();
    //qDebug() << " - bleAdapter" << m_bleAdapter;
    //qDebug() << " - bleEnabled" << m_bleEnabled;
    //qDebug() << " - permBluetooth" << m_blePermission;
#if defined(Q_OS_ANDROID)
    //qDebug() << " - permLocation(FG)" << m_locPermission_foreground;
    //qDebug() << " - permLocation(BG)" << m_locPermission_background;
    //qDebug() << " - gpsEnabled" << m_gpsEnabled;
#endif

    return m_blePermission;
}

/* ************************************************************************** */

bool DeviceManager::requestBluetoothPermissions()
{
    //qDebug() << "DeviceManager::requestBluetoothPermissions()";

    bool ble = checkBluetoothPermission();
    if (ble == false)
    {
        requestBluetoothPermission();
    }
    else // (ble == true)
    {
#if defined(Q_OS_ANDROID)
        if (UtilsApp::getInstance()->getAndroidSdkVersion() <= 30)
        {
            bool loc = checkLocationPermission();
            if (loc == false)
            {
                requestLocationPermission();
            }
        }
#endif
    }

    return false;
}

bool DeviceManager::hasBluetoothPermissions() const
{
#if defined(Q_OS_ANDROID)
    if (UtilsApp::getInstance()->getAndroidSdkVersion() <= 30)
    {
        return (m_bleAdapter && m_bleEnabled && m_blePermission && m_locPermission_foreground);
    }
#endif

    return (m_bleAdapter && m_bleEnabled && m_blePermission);
}

bool DeviceManager::hasBluetooth() const
{
    return (m_bleAdapter && m_bleEnabled && hasBluetoothPermissions());
}

/* ************************************************************************** */

bool DeviceManager::checkBluetoothPermission()
{
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    switch (qApp->checkPermission(bluetoothPermission))
    {
    case Qt::PermissionStatus::Granted:
        setBluetoothPermission(true);
        break;
    case Qt::PermissionStatus::Denied:
        setBluetoothPermission(false);
        break;
    case Qt::PermissionStatus::Undetermined:
        break;
    }

    return m_blePermission;
}

bool DeviceManager::requestBluetoothPermission()
{
    qDebug() << "DeviceManager::requestBluetoothPermission()";

    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    switch (qApp->checkPermission(bluetoothPermission))
    {
    case Qt::PermissionStatus::Granted:
        setBluetoothPermission(true);
        break;
    case Qt::PermissionStatus::Denied:
    case Qt::PermissionStatus::Undetermined:
        qDebug() << "Requesting BLUETOOTH permission...";
        qApp->requestPermission(bluetoothPermission, this, &DeviceManager::requestBluetoothPermission_results);
        break;
    }

    return m_blePermission;
}

void DeviceManager::requestBluetoothPermission_results(const QPermission &permission)
{
    // evaluate the results
    switch (permission.status())
    {
    case Qt::PermissionStatus::Granted: {
        setBluetoothPermission(true);

#if defined(Q_OS_ANDROID)
        if (UtilsApp::getAndroidSdkVersion() <= 30)
        {
            bool loc = checkLocationPermission();
            if (loc == false)
            {
                requestLocationPermission();
            }
        }
#endif
        enableBluetooth(true);
        break;
        }
    case Qt::PermissionStatus::Denied:
    case Qt::PermissionStatus::Undetermined:
        setBluetoothPermission(false);
        break;
    }
}

/* ************************************************************************** */

bool DeviceManager::checkLocationPermission()
{
    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    locationPermission.setAvailability(QLocationPermission::WhenInUse);

    switch (qApp->checkPermission(locationPermission))
    {
    case Qt::PermissionStatus::Granted:
        setLocationPermission_foreground(true);
        break;
    case Qt::PermissionStatus::Denied:
        setLocationPermission_foreground(false);
        break;
    case Qt::PermissionStatus::Undetermined:
        break;
    }

    return m_locPermission_foreground;
}

bool DeviceManager::requestLocationPermission()
{
    //qDebug() << "DeviceManager::requestLocationPermission()";

    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    locationPermission.setAvailability(QLocationPermission::WhenInUse);

    switch (qApp->checkPermission(locationPermission))
    {
    case Qt::PermissionStatus::Granted:
        setLocationPermission_foreground(true);
        break;
    case Qt::PermissionStatus::Denied:
    case Qt::PermissionStatus::Undetermined:
        qDebug() << "Requesting LOCATION permission...";
        qApp->requestPermission(locationPermission, this, &DeviceManager::requestLocationPermission_results);
        break;
    }

    return m_locPermission_foreground;
}

void DeviceManager::requestLocationPermission_results(const QPermission &permission)
{
    // evaluate the results
    switch (permission.status())
    {
    case Qt::PermissionStatus::Granted:
        setLocationPermission_foreground(true);
        enableBluetooth(true);
        break;
    case Qt::PermissionStatus::Denied:
    case Qt::PermissionStatus::Undetermined:
        setLocationPermission_foreground(false);
        break;
    }
}

void DeviceManager::setBluetoothPermission(bool perm)
{
    if (m_blePermission != perm)
    {
        m_blePermission = perm;
        Q_EMIT permissionsChanged();
    }
}
void DeviceManager::setLocationPermission_foreground(bool perm)
{
    if (m_locPermission_foreground != perm)
    {
        m_locPermission_foreground = perm;
        Q_EMIT permissionsChanged();
    }
}
void DeviceManager::setLocationPermission_background(bool perm)
{
    if (m_locPermission_background != perm)
    {
        m_locPermission_background = perm;
        Q_EMIT permissionsChanged();
    }
}

/* ************************************************************************** */

void DeviceManager::bluetoothHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
    //qDebug() << "DeviceManager::bluetoothHostModeStateChanged() host mode now:" << state;

    if (state != m_bluetoothHostMode)
    {
        m_bluetoothHostMode = state;
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
    qDebug() << "DeviceManager::bluetoothStatusChanged() bt adapter:" << m_bleAdapter << " /  bt enabled:" << m_bleEnabled;

    if (m_bleAdapter && m_bleEnabled)
    {
        // Bluetooth enabled, re/start listening
        if (m_devices_model->hasDevices())
        {
            refreshDevices_start();
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
    if (!m_bluetoothDiscoveryAgent)
    {
        m_bluetoothDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
        if (m_bluetoothDiscoveryAgent)
        {
            //qDebug() << "Scanning method supported:" << m_bluetoothDiscoveryAgent->supportedDiscoveryMethods();

            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                    this, &DeviceManager::deviceDiscoveryFinished, Qt::UniqueConnection);

            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                    this, &DeviceManager::deviceDiscoveryStopped, Qt::UniqueConnection);

            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
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
    m_blePermission = checkBluetoothPermission();

    if (!m_bluetoothDiscoveryAgent)
    {
        startBleAgent();
    }

    if (m_bluetoothDiscoveryAgent)
    {
        m_bluetoothDiscoveryAgent->setLowEnergyDiscoveryTimeout(12); // 12 ms
        m_bluetoothDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

        if (m_bluetoothDiscoveryAgent->isActive())
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
        else
        {
            qWarning() << "Cannot check iOS Bluetooth";
        }
    }
    else
    {
        qWarning() << "Cannot start BLE agent";
    }
}

void DeviceManager::deviceDiscoveryErrorIOS()
{
    //qDebug() << "DeviceManager::deviceDiscoveryErrorIOS()";

    if (m_bluetoothDiscoveryAgent) m_bluetoothDiscoveryAgent->stop();
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
        m_blePermission = false;
        Q_EMIT bluetoothChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::LocationServiceTurnedOffError)
    {
        qWarning() << "deviceDiscoveryError() Location Service Turned Off Error.";

        m_bleEnabled = false;
        m_blePermission = false;
        m_locPermission_foreground = false;
        Q_EMIT bluetoothChanged();
        Q_EMIT permissionsChanged();
    }
    else if (error == QBluetoothDeviceDiscoveryAgent::MissingPermissionsError)
    {
        qWarning() << "deviceDiscoveryError() Missing Permissions Error.";

        m_bleEnabled = false;
        m_blePermission = false;
        Q_EMIT bluetoothChanged();
        Q_EMIT permissionsChanged();
    }
    else
    {
        qWarning() << "An unknown error has occurred.";

        m_bleAdapter = false;
        m_bleEnabled = false;
        Q_EMIT bluetoothChanged();
    }

    scanDevices_stop();
    scanNearby_stop();
    listenDevices_stop();

    refreshDevices_stop();
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

    if (m_scanning_nearby)
    {
        m_scanning_nearby = false;
        Q_EMIT scanningNearbyChanged();
    }

    if (m_listening)
    {
        //m_listening = false;
        //Q_EMIT listeningChanged();
    }

    listenDevices_start();
}

void DeviceManager::deviceDiscoveryStopped()
{
    qDebug() << "DeviceManager::deviceDiscoveryStopped()";

    if (m_scanning)
    {
        m_scanning = false;
        Q_EMIT scanningChanged();
    }

    if (m_scanning_nearby)
    {
        m_scanning_nearby = false;
        Q_EMIT scanningNearbyChanged();
    }

    if (m_listening)
    {
        m_listening = false;
        Q_EMIT listeningChanged();
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceManager::scanDevices_start()
{
    //qDebug() << "DeviceManager::scanDevices_start()";

    if (hasBluetooth())
    {
        if (!m_bluetoothDiscoveryAgent)
        {
            startBleAgent();
        }

        if (m_bluetoothDiscoveryAgent)
        {
            if (m_bluetoothDiscoveryAgent->isActive())
            {
                refreshDevices_stop();
            }

            disconnect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                       this, &DeviceManager::bleDeviceNearby_discovered);
            disconnect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                       this, &DeviceManager::bleDeviceNearby_updated);

            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                    this, &DeviceManager::bleDevice_discovered, Qt::UniqueConnection);
            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                    this, &DeviceManager::bleDevice_updated, Qt::UniqueConnection);

            // start scanning
            m_bluetoothDiscoveryAgent->setLowEnergyDiscoveryTimeout(ble_scanning_duration*1000);
            m_bluetoothDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

            if (m_bluetoothDiscoveryAgent->isActive())
            {
                m_scanning = true;
                Q_EMIT scanningChanged();
                qDebug() << "Scanning for new BLE devices...";
            }
            else
            {
                qWarning() << "DeviceManager::scanDevices_start() DID NOT START";
            }
        }
        else
        {
            qWarning() << "Cannot start BLE agent";
        }
    }
    else
    {
        qWarning() << "Cannot scan or listen without BLE (or BLE permissions)";
    }
}

void DeviceManager::scanDevices_stop()
{
    //qDebug() << "DeviceManager::scanDevices_stop()";

    if (m_bluetoothDiscoveryAgent)
    {
        if (m_bluetoothDiscoveryAgent->isActive())
        {
            m_bluetoothDiscoveryAgent->stop();
        }

        if (m_scanning)
        {
            m_scanning = false;
            Q_EMIT scanningChanged();
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
    qDebug() << "DeviceManager::listenDevices_start()";

#if defined(Q_OS_ANDROID)
    // An Android service won't be able to scan/listen in the background without the associated permission
    if (m_daemonMode && !m_locPermission_background) return;
#endif

    if (hasBluetooth())
    {
        if (!m_bluetoothDiscoveryAgent)
        {
            startBleAgent();
        }

        if (m_bluetoothDiscoveryAgent)
        {
            if (m_bluetoothDiscoveryAgent->isActive() && m_scanning)
            {
                m_bluetoothDiscoveryAgent->stop();

                m_scanning = false;
                Q_EMIT scanningChanged();
            }

            disconnect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                       this, &DeviceManager::bleDeviceNearby_discovered);
            disconnect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                       this, &DeviceManager::bleDeviceNearby_updated);

            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                    this, &DeviceManager::bleDevice_discovered, Qt::UniqueConnection);
            connect(m_bluetoothDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                    this, &DeviceManager::bleDevice_updated, Qt::UniqueConnection);

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
                m_bluetoothDiscoveryAgent->setAndroidScanFilter(filteredAddr); // WIP
            }
#endif // Q_OS_ANDROID

            int duration = ble_listening_duration*1000;
            if (m_daemonMode) duration = ble_listening_duration_background*1000;

            // start listening
            m_bluetoothDiscoveryAgent->setLowEnergyDiscoveryTimeout(duration);
            m_bluetoothDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

            if (m_bluetoothDiscoveryAgent->isActive())
            {
                m_listening = true;
                Q_EMIT listeningChanged();
                //qDebug() << ">> Listening for BLE advertisement packets...";

                // Update lastRun
                //setLastRun();
            }
            else
            {
                qWarning() << "DeviceManager::listenDevices_start() DID NOT START";
            }
        }
        else
        {
            qWarning() << "Cannot start BLE agent";
        }
    }
    else
    {
        qWarning() << "Cannot scan or listen without BLE (or BLE permissions)";
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
    SettingsManager *sm = SettingsManager::getInstance();

    // Various sanity checks
    {
        if (info.rssi() >= 0) return; // we probably just hit the device cache
        if (sm && sm->getBluetoothLimitScanningRange() && info.rssi() < -70) return; // device is too far away
        if ((info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) == false) return; // not a BLE device

        if (m_devices_blacklist.contains(info.address().toString())) return; // device is blacklisted
        if (m_devices_blacklist.contains(info.deviceUuid().toString())) return; // device is blacklisted

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

    qDebug() << "DeviceManager::addBleDevice()" << " > NAME" << info.name() << " > RSSI" << info.rssi();

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

            // Remove from model
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
