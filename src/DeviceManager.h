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

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H
/* ************************************************************************** */

#include "DeviceFilter.h"

#include <QObject>
#include <QVariant>
#include <QList>
#include <QTimer>

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

class QBluetoothDeviceInfo;
class QLowEnergyController;
class QPermission;

/* ************************************************************************** */

/*!
 * \brief The DeviceManager class
 */
class DeviceManager: public QObject
{
    Q_OBJECT

    ////////

    Q_PROPERTY(bool hasDevices READ areDevicesAvailable NOTIFY devicesListUpdated)
    Q_PROPERTY(int deviceCount READ getDeviceCount NOTIFY devicesListUpdated)
    Q_PROPERTY(DeviceFilter *devicesList READ getDevicesFiltered NOTIFY devicesListUpdated)
    Q_PROPERTY(DeviceFilter *devicesNearby READ getDevicesNearby NOTIFY devicesNearbyUpdated)

    ////////

    Q_PROPERTY(bool advertising READ isAdvertising NOTIFY advertisingChanged)
    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool scanningNearby READ isScanningNearby NOTIFY scanningNearbyChanged)
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    Q_PROPERTY(bool syncing READ isSyncing NOTIFY syncingChanged)

    Q_PROPERTY(bool bluetooth READ hasBluetooth NOTIFY bluetoothChanged)
    Q_PROPERTY(bool bluetoothAdapter READ hasBluetoothAdapter NOTIFY bluetoothChanged)
    Q_PROPERTY(bool bluetoothEnabled READ hasBluetoothEnabled NOTIFY bluetoothChanged)
    Q_PROPERTY(bool bluetoothPermissions READ hasBluetoothPermissions NOTIFY permissionsChanged)

    Q_PROPERTY(bool permissionBluetooth READ hasPermissionBluetooth NOTIFY permissionsChanged)
    Q_PROPERTY(bool permissionLocationForeground READ hasPermissionLocationForeground NOTIFY permissionsChanged)
    Q_PROPERTY(bool permissionLocationBackground READ hasPermissionLocationBackground NOTIFY permissionsChanged)
    Q_PROPERTY(bool gpsEnabled READ hasGpsEnabled NOTIFY permissionsChanged)

    Q_PROPERTY(int bluetoothHostMode READ getBluetoothHostMode NOTIFY hostModeChanged)

    ////

    static const int ble_scanning_duration = 60;
    static const int ble_listening_duration = 0;
    static const int ble_listening_duration_nearby = 0;
    static const int ble_listening_duration_background = 60;

    bool m_dbInternal = false;  //!< do we have an internal SQLite database?
    bool m_dbExternal = false;  //!< do we have a remote MySQL database?

    bool m_daemonMode = false;  //!< did we start without an UI?

    ////

    bool m_bleAdapter = false;      //!< do we have a BLE adapter?
    bool m_bleEnabled = false;      //!< is the BLE adapter enabled?
    bool m_blePermission = false;   //!< do we have necessary BLE permission(s)?

    bool m_locPermission_foreground = false;    //!< do we have location permission? (Android)
    bool m_locPermission_background = false;    //!< do we have background location permission? (Android)
    bool m_gpsEnabled = false;                  //!< is the GPS enabled? (Android)

    QBluetoothLocalDevice *m_bluetoothAdapter = nullptr;
    QBluetoothDeviceDiscoveryAgent *m_bluetoothDiscoveryAgent = nullptr;
    QBluetoothLocalDevice::HostMode m_bluetoothHostMode = QBluetoothLocalDevice::HostPoweredOff;

    QList <QObject *> m_bluetoothAdapters;

    ////

    QList <QString> m_devices_blacklist;

    DeviceModel *m_devices_nearby_model = nullptr;
    DeviceFilter *m_devices_nearby_filter = nullptr;

    DeviceModel *m_devices_model = nullptr;
    DeviceFilter *m_devices_filter = nullptr;

    QList <QObject *> m_devices_updating_queue;
    QList <QObject *> m_devices_updating;

    QList <QObject *> m_devices_syncing_queue;
    QList <QObject *> m_devices_syncing;

    ////

    bool m_advertising = false;
    bool isAdvertising() const { return m_advertising; }

    bool m_listening = false;
    bool isListening() const { return m_listening; }

    bool m_scanning = false;
    bool isScanning() const { return m_scanning; }

    bool m_scanning_nearby = false;
    bool isScanningNearby() const { return m_scanning_nearby; }

    bool m_updating = false;
    bool isUpdating() const { return m_updating; }

    bool m_syncing = false;
    bool isSyncing() const { return m_syncing; }

    bool hasBluetoothAdapter() const { return m_bleAdapter; }
    bool hasBluetoothEnabled() const { return m_bleEnabled; }
    bool hasBluetoothPermissions() const;
    bool hasBluetooth() const;

    bool hasPermissionBluetooth() const { return m_blePermission; }
    bool hasPermissionLocationForeground() const { return m_locPermission_foreground; }
    bool hasPermissionLocationBackground() const { return m_locPermission_background; }
    bool hasGpsEnabled() const { return m_gpsEnabled; }

    void setBluetoothPermission(bool perm);
    void setLocationPermission_foreground(bool perm);
    void setLocationPermission_background(bool perm);

    int getBluetoothHostMode() const { return m_bluetoothHostMode; }

    void startBleAgent();

    void checkBluetoothIOS();
    bool m_checking_ios_ble = false;
    QTimer m_checking_ios_timer;

    void setLastRun();

    QTimer m_updateTimer;
    void setUpdateTimer(int updateIntervalMin = 0);

Q_SIGNALS:
    void bluetoothChanged();
    void hostModeChanged();
    void permissionsChanged();

    void adaptersListUpdated();

    void devicesListUpdated();
    void devicesNearbyUpdated();
    void devicesBlacklistUpdated();

    void advertisingChanged();
    void listeningChanged();
    void scanningChanged();
    void scanningNearbyChanged();
    void updatingChanged();
    void syncingChanged();

private slots:
    // QBluetoothLocalDevice related
    void bluetoothHostModeStateChanged(QBluetoothLocalDevice::HostMode);
    void bluetoothStatusChanged();
    void bluetoothPermissionsChanged();

    // QBluetoothDeviceDiscoveryAgent related
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error);
    void deviceDiscoveryErrorIOS();
    void deviceDiscoveryFinished();
    void deviceDiscoveryStopped();

    //
    void addBleDeviceNearby(const QBluetoothDeviceInfo &info);
    void bleDeviceNearby_updated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);
    void bleDeviceNearby_discovered(const QBluetoothDeviceInfo &info);

    //
    void addBleDevice(const QBluetoothDeviceInfo &info);
    void bleDevice_discovered(const QBluetoothDeviceInfo &info);
    void bleDevice_updated(const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields updatedFields);

public:
    DeviceManager(bool daemon = false);
    ~DeviceManager();

    bool isDaemon() const { return m_daemonMode; }

    // Adapters management
    Q_INVOKABLE bool areAdaptersAvailable() const { return m_bluetoothAdapters.size(); }
    QVariant getAdapters() const { return QVariant::fromValue(m_bluetoothAdapters); }
    int getAdaptersCount() const { return m_bluetoothAdapters.size(); }

    // Bluetooth management
    Q_INVOKABLE bool checkBluetooth();
    Q_INVOKABLE bool checkBluetoothPermissions();
    Q_INVOKABLE bool requestBluetoothPermissions();
    Q_INVOKABLE bool enableBluetooth(bool enforceUserPermissionCheck = false);

    Q_INVOKABLE bool checkBluetoothPermission();
    Q_INVOKABLE bool requestBluetoothPermission();
    void requestBluetoothPermission_results(const QPermission &permission);

    Q_INVOKABLE bool checkLocationPermission();
    Q_INVOKABLE bool requestLocationPermission();
    void requestLocationPermission_results(const QPermission &permission);

    // Scanning management
    static int getLastRun();

    Q_INVOKABLE void scanNearby_start();
    Q_INVOKABLE void scanNearby_stop();

    Q_INVOKABLE void scanDevices_start();
    Q_INVOKABLE void scanDevices_stop();

    Q_INVOKABLE void refreshDevices_start();
    Q_INVOKABLE void refreshDevices_stop();

    Q_INVOKABLE void listenDevices_start();
    Q_INVOKABLE void listenDevices_stop();

    // Device management
    Q_INVOKABLE void removeDevice(const QString &address);
    Q_INVOKABLE void removeDeviceData(const QString &address);

    Q_INVOKABLE void blacklistBleDevice(const QString &addr);
    Q_INVOKABLE void whitelistBleDevice(const QString &addr);
    Q_INVOKABLE bool isBleDeviceBlacklisted(const QString &addr);

    // Devices list management
    Q_INVOKABLE bool areDevicesAvailable() const { return m_devices_model->hasDevices(); }
    Q_INVOKABLE bool areDevicesConnected() const;
    Q_INVOKABLE void disconnectDevices() const;
    Q_INVOKABLE void disconnectAndExit() const;

    int getDeviceCount() const { return m_devices_model->getDeviceCount(); }
    DeviceFilter *getDevicesFiltered() const { return m_devices_filter; }
    DeviceFilter *getDevicesNearby() const { return m_devices_nearby_filter; }

    // Sorting and filtering
    Q_INVOKABLE void orderby_manual();
    Q_INVOKABLE void orderby_model();
    Q_INVOKABLE void orderby_name();
    Q_INVOKABLE void orderby_location();
    Q_INVOKABLE void orderby_waterlevel();
    Q_INVOKABLE void orderby_plant();
    Q_INVOKABLE void orderby_insideoutside();
    void orderby(int role, Qt::SortOrder order);

    Q_INVOKABLE QVariant getDeviceByProxyIndex(const int index, const int deviceType = 0) const {
        Q_UNUSED(deviceType)
        QModelIndex proxyIndex = m_devices_filter->index(index, 0);
        return QVariant::fromValue(m_devices_filter->data(proxyIndex, DeviceModel::PointerRole));
    }

    void invalidate();
    void invalidateFilter();
};

/* ************************************************************************** */
#endif // DEVICE_MANAGER_H
