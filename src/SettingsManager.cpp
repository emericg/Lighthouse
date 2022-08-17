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

#include "SettingsManager.h"
#include "SystrayManager.h"

#include <QCoreApplication>
#include <QSettings>
#include <QLocale>
#include <QDebug>

/* ************************************************************************** */

SettingsManager *SettingsManager::instance = nullptr;

SettingsManager *SettingsManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new SettingsManager();
    }

    return instance;
}

SettingsManager::SettingsManager()
{
    readSettings();
}

SettingsManager::~SettingsManager()
{
    //
}

/* ************************************************************************** */
/* ************************************************************************** */

void SettingsManager::reloadSettings()
{
    readSettings();
}

bool SettingsManager::readSettings()
{
    bool status = false;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if (settings.status() == QSettings::NoError)
    {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
        if (settings.contains("ApplicationWindow/x"))
            m_appPosition.setWidth(settings.value("ApplicationWindow/x").toInt());
        if (settings.contains("ApplicationWindow/y"))
            m_appPosition.setHeight(settings.value("ApplicationWindow/y").toInt());
        if (settings.contains("ApplicationWindow/width"))
            m_appSize.setWidth(settings.value("ApplicationWindow/width").toInt());
        if (settings.contains("ApplicationWindow/height"))
            m_appSize.setHeight(settings.value("ApplicationWindow/height").toInt());
        if (settings.contains("ApplicationWindow/visibility"))
            m_appVisibility = settings.value("ApplicationWindow/visibility").toUInt();

        if (m_appPosition.width() > 8192) m_appPosition.setWidth(100);
        if (m_appPosition.height() > 8192) m_appPosition.setHeight(100);
        if (m_appSize.width() > 8192) m_appSize.setWidth(1920);
        if (m_appSize.height() > 8192) m_appSize.setHeight(1080);
        if (m_appVisibility < 1 || m_appVisibility > 5) m_appVisibility = 1;
#endif

        if (settings.contains("settings/appTheme"))
            m_appTheme = settings.value("settings/appTheme").toString();

        if (settings.contains("settings/appThemeAuto"))
            m_appThemeAuto = settings.value("settings/appThemeAuto").toBool();

        if (settings.contains("settings/appLanguage"))
            m_appLanguage = settings.value("settings/appLanguage").toString();

        if (settings.contains("settings/bluetoothControl"))
            m_bluetoothControl = settings.value("settings/bluetoothControl").toBool();

        if (settings.contains("settings/bluetoothLimitScanningRange"))
            m_bluetoothLimitScanningRange = settings.value("settings/bluetoothLimitScanningRange").toBool();

        {
#if defined(Q_OS_ANDROID)
            // too many weak devices on Android...
            m_bluetoothSimUpdates = 2;
#elif defined(Q_OS_IOS)
            // iOS is better
            m_bluetoothSimUpdates = 3;
#else
            // desktops are usually good with simultaneous updates
            m_bluetoothSimUpdates = 4;
#endif
        }

        if (settings.contains("settings/startMinimized"))
            m_startMinimized = settings.value("settings/startMinimized").toBool();

        if (settings.contains("settings/trayEnabled"))
            m_systrayEnabled = settings.value("settings/trayEnabled").toBool();

        if (settings.contains("settings/notifsEnabled"))
            m_notificationsEnabled = settings.value("settings/notifsEnabled").toBool();

        if (settings.contains("settings/updateIntervalBackground"))
            m_updateIntervalBackground = settings.value("settings/updateIntervalBackground").toInt();

        if (settings.contains("settings/updateIntervalPlant"))
            m_updateIntervalPlant = settings.value("settings/updateIntervalPlant").toInt();

        if (settings.contains("settings/updateIntervalThermo"))
            m_updateIntervalThermometer = settings.value("settings/updateIntervalThermo").toInt();

        if (settings.contains("settings/updateIntervalEnv"))
            m_updateIntervalEnvironmental = settings.value("settings/updateIntervalEnv").toInt();

        if (settings.contains("settings/tempUnit"))
            m_tempUnit = settings.value("settings/tempUnit").toString();
        else
        {
            // If we have no measurement system saved, use system's one
            // TODO: i18n may not have been set yet?
            QLocale lo;
            if (lo.measurementSystem() == QLocale::MetricSystem)
                m_tempUnit = "C";
            else
                m_tempUnit = "F";
        }

        if (settings.contains("settings/graphHistory"))
            m_graphHistogram = settings.value("settings/graphHistory").toString();

        if (settings.contains("settings/graphThermometer"))
            m_graphThermometer = settings.value("settings/graphThermometer").toString();

        if (settings.contains("settings/graphShowDots"))
            m_graphShowDots = settings.value("settings/graphShowDots").toBool();

        if (settings.contains("settings/compactView"))
            m_compactView = settings.value("settings/compactView").toBool();
        else
        {
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_MACOS)
            // these platforms have HiDPI screens
            m_compactView = true;
#else
            m_compactView = false;
#endif
        }

        if (settings.contains("settings/bigIndicator"))
            m_bigIndicator = settings.value("settings/bigIndicator").toBool();

        if (settings.contains("settings/dynaScale"))
            m_dynaScale = settings.value("settings/dynaScale").toBool();

        if (settings.contains("settings/orderBy"))
            m_orderBy = settings.value("settings/orderBy").toString();

        if (settings.contains("database/enabled"))
            m_mysql = settings.value("database/enabled").toBool();
        if (settings.contains("database/host"))
            m_mysqlHost = settings.value("database/host").toString();
        if (settings.contains("database/port"))
            m_mysqlPort = settings.value("database/port").toInt();
        if (settings.contains("database/name"))
            m_mysqlName = settings.value("database/name").toString();
        if (settings.contains("database/user"))
            m_mysqlUser = settings.value("database/user").toString();
        if (settings.contains("database/password"))
            m_mysqlPassword = settings.value("database/password").toString();

        if (settings.contains("mqtt/enabled"))
            m_mqtt = settings.value("mqtt/enabled").toBool();
        if (settings.contains("mqtt/host"))
            m_mqttHost = settings.value("mqtt/host").toString();
        if (settings.contains("mqtt/port"))
            m_mqttPort = settings.value("mqtt/port").toInt();
        if (settings.contains("mqtt/name"))
            m_mqttName = settings.value("mqtt/name").toString();
        if (settings.contains("mqtt/user"))
            m_mqttUser = settings.value("mqtt/user").toString();
        if (settings.contains("mqtt/password"))
            m_mqttPassword = settings.value("mqtt/password").toString();

        if (settings.contains("netctrl/enabled"))
            m_netctrl = settings.value("netctrl/enabled").toBool();
        if (settings.contains("netctrl/host"))
            m_netctrlHost = settings.value("netctrl/host").toString();
        if (settings.contains("netctrl/port"))
            m_netctrlPort = settings.value("netctrl/port").toInt();

        status = true;
    }
    else
    {
        qWarning() << "SettingsManager::readSettings() error:" << settings.status();
    }

    return status;
}

/* ************************************************************************** */

bool SettingsManager::writeSettings()
{
    bool status = false;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if (settings.isWritable())
    {
        settings.setValue("settings/appTheme", m_appTheme);
        settings.setValue("settings/appThemeAuto", m_appThemeAuto);
        settings.setValue("settings/appLanguage", m_appLanguage);
        settings.setValue("settings/bluetoothControl", m_bluetoothControl);
        settings.setValue("settings/bluetoothLimitScanningRange", m_bluetoothLimitScanningRange);
        settings.setValue("settings/bluetoothSimUpdates", m_bluetoothSimUpdates);
        settings.setValue("settings/startMinimized", m_startMinimized);
        settings.setValue("settings/trayEnabled", m_systrayEnabled);
        settings.setValue("settings/notifsEnabled", m_notificationsEnabled);
        //settings.setValue("settings/updateIntervalBackground", m_updateIntervalBackground);
        settings.setValue("settings/updateIntervalPlant", m_updateIntervalPlant);
        settings.setValue("settings/updateIntervalThermo", m_updateIntervalThermometer);
        //settings.setValue("settings/updateIntervalEnv", m_updateIntervalEnvironmental);
        settings.setValue("settings/graphHistory", m_graphHistogram);
        settings.setValue("settings/graphThermometer", m_graphThermometer);
        settings.setValue("settings/graphShowDots", m_graphShowDots);
        settings.setValue("settings/bigIndicator", m_bigIndicator);
        settings.setValue("settings/compactView", m_compactView);
        settings.setValue("settings/tempUnit", m_tempUnit);
        settings.setValue("settings/dynaScale", m_dynaScale);
        settings.setValue("settings/orderBy", m_orderBy);

        settings.setValue("database/enabled", m_mysql);
        settings.setValue("database/host", m_mysqlHost);
        settings.setValue("database/port", m_mysqlPort);
        settings.setValue("database/name", m_mysqlName);
        settings.setValue("database/user", m_mysqlUser);
        settings.setValue("database/password", m_mysqlPassword);

        settings.setValue("mqtt/enabled", m_mqtt);
        settings.setValue("mqtt/host", m_mqttHost);
        settings.setValue("mqtt/port", m_mqttPort);
        settings.setValue("mqtt/name", m_mqttName);
        settings.setValue("mqtt/user", m_mqttUser);
        settings.setValue("mqtt/password", m_mqttPassword);

        settings.setValue("netctrl/enabled", m_netctrl);
        settings.setValue("netctrl/host", m_netctrlHost);
        settings.setValue("netctrl/port", m_netctrlPort);

        if (settings.status() == QSettings::NoError)
        {
            status = true;
        }
        else
        {
            qWarning() << "SettingsManager::writeSettings() error:" << settings.status();
        }
    }
    else
    {
        qWarning() << "SettingsManager::writeSettings() error: read only file?";
    }

    return status;
}

/* ************************************************************************** */

void SettingsManager::resetSettings()
{
    m_appTheme = "THEME_PLANT";
    Q_EMIT appThemeChanged();
    m_appThemeAuto = false;
    Q_EMIT appThemeAutoChanged();
    m_appThemeCSD = false;
    Q_EMIT appThemeCSDChanged();
    m_appUnits = 0;
    Q_EMIT appUnitsChanged();
    m_appLanguage = "auto";
    Q_EMIT appLanguageChanged();

    m_startMinimized = false;
    Q_EMIT minimizedChanged();
    m_systrayEnabled = true;
    Q_EMIT systrayChanged();
    m_notificationsEnabled = true;
    Q_EMIT notifsChanged();
    m_updateIntervalBackground = s_intervalBackgroundUpdate;
    Q_EMIT updateIntervalBackgroundChanged();
    m_updateIntervalPlant = s_intervalPlantUpdate;
    Q_EMIT updateIntervalPlantChanged();
    m_updateIntervalThermometer = s_intervalThermometerUpdate;
    Q_EMIT updateIntervalThermoChanged();
    m_updateIntervalEnvironmental = s_intervalEnvironmentalUpdate;
    Q_EMIT updateIntervalEnvChanged();

    m_bluetoothControl = false;
    Q_EMIT bluetoothControlChanged();
    m_bluetoothLimitScanningRange = false;
    Q_EMIT bluetoothLimitScanningRangeChanged();
    m_bluetoothSimUpdates = 2;
    Q_EMIT bluetoothSimUpdatesChanged();

    QLocale lo;
    if (lo.measurementSystem() == QLocale::MetricSystem)
        m_tempUnit = "C";
    else
        m_tempUnit = "F";
    Q_EMIT tempUnitChanged();

    m_graphHistogram = "monthly";
    Q_EMIT graphHistogramChanged();
    m_graphThermometer = "minmax";
    Q_EMIT graphThermometerChanged();
    m_graphShowDots = false;
    Q_EMIT graphShowDotsChanged();

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_MACOS)
    // these platforms have HiDPI screens
    m_compactView = true;
#else
    m_compactView = false;
#endif
    Q_EMIT compactViewChanged();

    m_bigIndicator = false;
    Q_EMIT bigIndicatorChanged();
    m_dynaScale = false;
    Q_EMIT dynaScaleChanged();
    m_orderBy = "model";
    Q_EMIT orderByChanged();

    m_mysql = false;
    m_mysqlHost = "";
    m_mysqlPort = 3306;
    m_mysqlName = "lighthouse";
    m_mysqlUser = "lighthouse";
    m_mysqlPassword = "lighthouse";
    Q_EMIT mysqlChanged();

    m_mqtt = false;
    m_mqttHost = "";
    m_mqttPort = 1883;
    m_mqttName = "lighthouse";
    m_mqttUser = "lighthouse";
    m_mqttPassword = "lighthouse";
    m_mqttTopics = "lighthouse";
    Q_EMIT mqttChanged();
}

/* ************************************************************************** */
/* ************************************************************************** */

void SettingsManager::setSysTray(const bool value)
{
    if (m_systrayEnabled != value)
    {
        bool trayEnable_saved = m_systrayEnabled;
        m_systrayEnabled = value;
        writeSettings();

        SystrayManager *st = SystrayManager::getInstance();
        if (st)
        {
            if (trayEnable_saved == true && m_systrayEnabled == false)
            {
                st->removeSystray();
                Q_EMIT systrayChanged();
            }
            else if (trayEnable_saved == false && m_systrayEnabled == true)
            {
                st->installSystray();
                Q_EMIT systrayChanged();
            }
        }
    }
}

void SettingsManager::setAppTheme(const QString &value)
{
    if (m_appTheme != value)
    {
        m_appTheme = value;
        writeSettings();
        Q_EMIT appThemeChanged();
    }
}

void SettingsManager::setAppThemeAuto(const bool value)
{
    if (m_appThemeAuto != value)
    {
        m_appThemeAuto = value;
        writeSettings();
        Q_EMIT appThemeAutoChanged();
    }
}

void SettingsManager::setAppThemeCSD(const bool value)
{
    if (m_appThemeCSD != value)
    {
        m_appThemeCSD = value;
        writeSettings();
        Q_EMIT appThemeCSDChanged();
    }
}

void SettingsManager::setAppUnits(const unsigned value)
{
    if (m_appUnits != value)
    {
        m_appUnits = value;
        writeSettings();
        Q_EMIT appUnitsChanged();
    }
}

void SettingsManager::setAppLanguage(const QString &value)
{
    if (m_appLanguage != value)
    {
        m_appLanguage = value;
        writeSettings();
        Q_EMIT appLanguageChanged();
    }
}

void SettingsManager::setMinimized(const bool value)
{
    if (m_startMinimized != value)
    {
        m_startMinimized = value;
        writeSettings();
        Q_EMIT minimizedChanged();
    }
}

void SettingsManager::setNotifs(const bool value)
{
    if (m_notificationsEnabled != value)
    {
        m_notificationsEnabled = value;
        writeSettings();
        Q_EMIT notifsChanged();
    }
}

void SettingsManager::setBluetoothControl(const bool value)
{
    if (m_bluetoothControl != value)
    {
        m_bluetoothControl = value;
        writeSettings();
        Q_EMIT bluetoothControlChanged();
    }
}
void SettingsManager::setBluetoothLimitScanningRange(const bool value)
{
    if (m_bluetoothLimitScanningRange != value)
    {
        m_bluetoothLimitScanningRange = value;
        writeSettings();
        Q_EMIT bluetoothLimitScanningRangeChanged();
    }
}

void SettingsManager::setBluetoothSimUpdates(const unsigned value)
{
    if (m_bluetoothSimUpdates != value)
    {
        m_bluetoothSimUpdates = value;
        writeSettings();
        Q_EMIT bluetoothSimUpdatesChanged();
    }
}

void SettingsManager::setUpdateIntervalBackground(const unsigned value)
{
    if (m_updateIntervalBackground != value)
    {
        m_updateIntervalBackground = value;
        writeSettings();
        Q_EMIT updateIntervalBackgroundChanged();
    }
}

void SettingsManager::setUpdateIntervalPlant(const unsigned value)
{
    if (m_updateIntervalPlant != value)
    {
        m_updateIntervalPlant = value;
        writeSettings();
        Q_EMIT updateIntervalPlantChanged();
    }
}

void SettingsManager::setUpdateIntervalThermo(const unsigned value)
{
    if (m_updateIntervalThermometer!= value)
    {
        m_updateIntervalThermometer = value;
        writeSettings();
        Q_EMIT updateIntervalThermoChanged();
    }
}

void SettingsManager::setUpdateIntervalEnv(const unsigned value)
{
    if (m_updateIntervalEnvironmental!= value)
    {
        m_updateIntervalEnvironmental = value;
        writeSettings();
        Q_EMIT updateIntervalEnvChanged();
    }
}

void SettingsManager::setTempUnit(const QString &value)
{
    if (m_tempUnit != value)
    {
        m_tempUnit = value;
        writeSettings();
        Q_EMIT tempUnitChanged();
    }
}

void SettingsManager::setGraphHistogram(const QString &value)
{
    if (m_graphHistogram != value)
    {
        m_graphHistogram = value;
        writeSettings();
        Q_EMIT graphHistogramChanged();
    }
}

void SettingsManager::setGraphThermometer(const QString &value)
{
    if (m_graphThermometer != value)
    {
        m_graphThermometer = value;
        writeSettings();
        Q_EMIT graphThermometerChanged();
    }
}

void SettingsManager::setGraphShowDots(const bool value)
{
    if (m_graphShowDots != value)
    {
        m_graphShowDots = value;
        writeSettings();
        Q_EMIT graphShowDotsChanged();
    }
}

void SettingsManager::setCompactView(const bool value)
{
    if (m_compactView != value)
    {
        m_compactView = value;
        writeSettings();
        Q_EMIT compactViewChanged();
    }
}

void SettingsManager::setBigIndicator(const bool value)
{
    if (m_bigIndicator != value)
    {
        m_bigIndicator = value;
        writeSettings();
        Q_EMIT bigIndicatorChanged();
    }
}

void SettingsManager::setDynaScale(const bool value)
{
    if (m_dynaScale != value)
    {
        m_dynaScale = value;
        writeSettings();
        Q_EMIT dynaScaleChanged();
    }
}

void SettingsManager::setOrderBy(const QString &value)
{
    if (m_orderBy != value)
    {
        m_orderBy = value;
        writeSettings();
        Q_EMIT orderByChanged();
    }
}

/* ************************************************************************** */

void SettingsManager::setMySQL(const bool value)
{
    if (m_mysql != value)
    {
        m_mysql = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlHost(const QString &value)
{
    if (m_mysqlHost != value)
    {
        m_mysqlHost = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlPort(const int value)
{
    if (m_mysqlPort != value)
    {
        m_mysqlPort = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlUser(const QString &value)
{
    if (m_mysqlUser != value)
    {
        m_mysqlUser = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMysqlPassword(const QString &value)
{
    if (m_mysqlPassword != value)
    {
        m_mysqlPassword = value;
        writeSettings();
        Q_EMIT mysqlChanged();
    }
}

void SettingsManager::setMQTT(const bool value)
{
    if (m_mqtt != value)
    {
        m_mqtt = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttHost(const QString &value)
{
    if (m_mqttHost != value)
    {
        m_mqttHost = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttPort(const int value)
{
    if (m_mqttPort != value)
    {
        m_mqttPort = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttUser(const QString &value)
{
    if (m_mqttUser != value)
    {
        m_mqttUser = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttPassword(const QString &value)
{
    if (m_mqttPassword != value)
    {
        m_mqttPassword = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

void SettingsManager::setMqttTopics(const QString &value)
{
    if (m_mqttTopics != value)
    {
        m_mqttTopics = value;
        writeSettings();
        Q_EMIT mqttChanged();
    }
}

/* ************************************************************************** */

void SettingsManager::setNetCtrl(const bool value)
{
    if (m_netctrl != value)
    {
        m_netctrl = value;
        writeSettings();
        Q_EMIT netctrlChanged();
    }
}

void SettingsManager::setNetCtrlHost(const QString &value)
{
    if (m_netctrlHost != value)
    {
        m_netctrlHost = value;
        writeSettings();
        Q_EMIT netctrlChanged();
    }
}

void SettingsManager::setNetCtrlPort(const int value)
{
    if (m_netctrlPort != value)
    {
        m_netctrlPort = value;
        writeSettings();
        Q_EMIT netctrlChanged();
    }
}

/* ************************************************************************** */
