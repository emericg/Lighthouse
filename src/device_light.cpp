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

#include "device_light.h"
#include "DatabaseManager.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

DeviceLight::DeviceLight(QString &deviceAddr, QString &deviceName, QObject *parent) :
    Device(deviceAddr, deviceName, parent)
{
    // Database
    DatabaseManager *db = DatabaseManager::getInstance();
    if (db)
    {
        m_dbInternal = db->hasDatabaseInternal();
        m_dbExternal = db->hasDatabaseExternal();
    }

    // Load device infos, bias, limits and initial data
    if (m_dbInternal || m_dbExternal)
    {
        getSqlDeviceInfos();
        m_colorSaved = getSetting("colorSaved").toString();
    }
}

DeviceLight::DeviceLight(const QBluetoothDeviceInfo &d, QObject *parent) :
    Device(d, parent)
{
    // Database
    DatabaseManager *db = DatabaseManager::getInstance();
    if (db)
    {
        m_dbInternal = db->hasDatabaseInternal();
        m_dbExternal = db->hasDatabaseExternal();
    }

    // Load device infos, bias, limits and initial data
    if (m_dbInternal || m_dbExternal)
    {
        getSqlDeviceInfos();
        m_colorSaved = getSetting("colorSaved").toString();
    }
}

DeviceLight::~DeviceLight()
{
    //
}

/* ************************************************************************** */

void DeviceLight::setMode(unsigned)
{
    //
}

void DeviceLight::setEffect(unsigned)
{
    //
}

/* ************************************************************************** */

void DeviceLight::setLuminosity(unsigned)
{
    //
}

void DeviceLight::setColor(unsigned)
{
    //
}

void DeviceLight::setColorAndBrightness(unsigned, unsigned, unsigned, unsigned)
{
    //
}

/* ************************************************************************** */

void DeviceLight::setColorSaved(const QString &value)
{
    //qDebug() << "setColorSaved(" << value << ")";

    if (m_colorSaved != value)
    {
        m_colorSaved = value;
        Q_EMIT colorSavedUpdated();
        setSetting("colorSaved", value);
    }
}

/* ************************************************************************** */
