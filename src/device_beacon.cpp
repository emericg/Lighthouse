/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 20220 Emeric Grange - All Rights Reserved
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

#include "device_beacon.h"
#include "local_controls/local_actions.h"
#include "local_controls/local_controls.h"
#include "DatabaseManager.h"

#include <QDateTime>
#include <QTimer>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

/* ************************************************************************** */

DeviceBeacon::DeviceBeacon(QString &deviceAddr, QString &deviceName, QObject *parent) :
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
        getSqlButtons();
    }
}

DeviceBeacon::DeviceBeacon(const QBluetoothDeviceInfo &d, QObject *parent) :
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
        getSqlButtons();
    }
}

DeviceBeacon::~DeviceBeacon()
{
    //
}

/* ************************************************************************** */

bool DeviceBeacon::getSqlButtons()
{
    //qDebug() << "DeviceBeacon::getSqlButtons(" << m_deviceAddress << ")";
    bool status = false;

    if (m_dbInternal || m_dbExternal)
    {
        QSqlQuery getButtons;
        getButtons.prepare("SELECT buttonId," \
                           " single_action, single_params, " \
                           " double_action, double_params," \
                           " long_action, long_params " \
                           "FROM buttons WHERE deviceAddr = :deviceAddr");
        getButtons.bindValue(":deviceAddr", getAddress());

        if (getButtons.exec())
        {
            while (getButtons.next())
            {
                int button = getButtons.value(0).toInt();

                ButtonParameters *btn = new ButtonParameters(button,
                                            getButtons.value(1).toInt(), getButtons.value(2).toString(),
                                            getButtons.value(3).toInt(), getButtons.value(4).toString(),
                                            getButtons.value(5).toInt(), getButtons.value(6).toString(),
                                            this);

                if (button == 1) btn1 = btn;
                else if (button == 2) btn2 = btn;
                else if (button == 3) btn3 = btn;
                else if (button == 4) btn4 = btn;
                else if (button == 5) btn5 =  btn;
                else if (button == 6) btn6 = btn;
                else delete btn;

                status = true;
                //Q_EMIT actionUpdated();
            }
        }
        else
        {
            qWarning() << "> getButtons.exec(plant) ERROR"
                       << getButtons.lastError().type() << ":" << getButtons.lastError().text();
        }
    }

    if (!btn1) btn1 = new ButtonParameters(1, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);
    if (!btn2) btn2 = new ButtonParameters(2, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);
    if (!btn3) btn3 = new ButtonParameters(3, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);
    if (!btn4) btn4 = new ButtonParameters(4, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);
    if (!btn5) btn5 = new ButtonParameters(5, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);
    if (!btn6) btn6 = new ButtonParameters(6, LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", LocalActions::ACTION_NO_ACTION, "", this);

    return status;
}

bool DeviceBeacon::setSqlButtons(int button)
{
    qDebug() << "DeviceBeacon::setSqlButtons(" << m_deviceAddress << ")";
    bool status = false;

    ButtonParameters *btn = nullptr;
    if (button == 1) btn = btn1;
    else if (button == 2) btn = btn2;
    else if (button == 3) btn = btn3;
    else if (button == 4) btn = btn4;
    else if (button == 5) btn = btn5;
    else if (button == 6) btn = btn6;

    if (m_dbInternal || m_dbExternal)
    {
        if (btn)
        {
            QSqlQuery updateButtons;
            updateButtons.prepare("REPLACE INTO buttons (deviceAddr, buttonId," \
                                  " single_action, single_params, " \
                                  " double_action, double_params," \
                                  " long_action, long_params) " \
                                  "VALUES (:deviceAddr, :buttonId, " \
                                          ":single_action, :single_params, "
                                          ":double_action, :double_params, " \
                                          ":long_action, :long_params)");

            updateButtons.bindValue(":deviceAddr", getAddress());
            updateButtons.bindValue(":buttonId", btn->getId());
            updateButtons.bindValue(":single_action", btn->getSingleAction());
            updateButtons.bindValue(":single_params", btn->getSingleParameters());
            updateButtons.bindValue(":double_action", btn->getDoubleAction());
            updateButtons.bindValue(":double_params", btn->getDoubleParameters());
            updateButtons.bindValue(":long_action", btn->getLongAction());
            updateButtons.bindValue(":long_params", btn->getLongParameters());

            status = updateButtons.exec();
            if (status == false)
            {
                qWarning() << "> updateButtons.exec(plant) ERROR"
                           << updateButtons.lastError().type() << ":" << updateButtons.lastError().text();
            }
        }
    }

    //Q_EMIT biasUpdated();

    return status;
}

bool ButtonParameters::setSqlButton()
{
    qDebug() << "ButtonParameters::setSqlButton(" << m_id << ")";
    bool status = false;

    QSqlQuery updateButton;
    updateButton.prepare("REPLACE INTO buttons (deviceAddr, buttonId," \
                          " single_action, single_params, " \
                          " double_action, double_params," \
                          " long_action, long_params) " \
                          "VALUES (:deviceAddr, :buttonId, " \
                                  ":single_action, :single_params, "
                                  ":double_action, :double_params, " \
                                  ":long_action, :long_params)");

    updateButton.bindValue(":deviceAddr", static_cast<Device *>(parent())->getAddress());
    updateButton.bindValue(":buttonId", getId());
    updateButton.bindValue(":single_action", getSingleAction());
    updateButton.bindValue(":single_params", getSingleParameters());
    updateButton.bindValue(":double_action", getDoubleAction());
    updateButton.bindValue(":double_params", getDoubleParameters());
    updateButton.bindValue(":long_action", getLongAction());
    updateButton.bindValue(":long_params", getLongParameters());

    status = updateButton.exec();
    if (status == false)
    {
        qWarning() << "> updateButton.exec(plant) ERROR"
                   << updateButton.lastError().type() << ":" << updateButton.lastError().text() << updateButton.lastQuery();
    }

    return status;
}

/* ************************************************************************** */

void DeviceBeacon::triggerEvent(int button, int mode)
{
    Q_EMIT buttonPressed();

    if (mode == 0)
    {
        if (button == 1) Q_EMIT button1Pressed();
        else if (button == 2) Q_EMIT button2Pressed();
        else if (button == 3) Q_EMIT button3Pressed();
        else if (button == 4) Q_EMIT button4Pressed();
        else if (button == 5) Q_EMIT button5Pressed();
        else if (button == 6) Q_EMIT button6Pressed();
    }
    else if (mode == 1)
    {
        if (button == 1) Q_EMIT button1DoublePressed();
        else if (button == 2) Q_EMIT button2DoublePressed();
        else if (button == 3) Q_EMIT button3DoublePressed();
        else if (button == 4) Q_EMIT button4DoublePressed();
        else if (button == 5) Q_EMIT button5DoublePressed();
        else if (button == 6) Q_EMIT button6DoublePressed();
    }
    else if (mode == 2)
    {
        if (button == 1) Q_EMIT button1LongPressed();
        else if (button == 2) Q_EMIT button2LongPressed();
        else if (button == 3) Q_EMIT button3LongPressed();
        else if (button == 4) Q_EMIT button4LongPressed();
        else if (button == 5) Q_EMIT button5LongPressed();
        else if (button == 6) Q_EMIT button6LongPressed();
    }
}

void DeviceBeacon::triggerAction(int button, int mode)
{
    int action = 0;
    QString action_parameters;

    ButtonParameters *btn = nullptr;
    if (button == 1) btn = btn1;
    else if (button == 2) btn = btn2;
    else if (button == 3) btn = btn3;
    else if (button == 4) btn = btn4;
    else if (button == 5) btn = btn5;
    else if (button == 6) btn = btn6;

    if (btn)
    {
        if (mode == 0)
        {
            action = btn->getSingleAction();
            action_parameters = btn->getSingleParameters();
        }
        else if (mode == 1)
        {
            action = btn->getDoubleAction();
            action_parameters = btn->getDoubleParameters();
        }
        else if (mode == 2)
        {
            action = btn->getLongAction();
            action_parameters = btn->getLongParameters();
        }
    }

    LocalControls *ctrls = LocalControls::getInstance();
    if (ctrls)
    {
        ctrls->action(action, action_parameters);
    }
}

bool DeviceBeacon::getAction(int button, int mode,
                             int &action, QString &params)
{
    return false;
}

void DeviceBeacon::triggerDirectAction(int action,
                                       const QString &action_parameters)
{
    LocalControls *ctrls = LocalControls::getInstance();
    if (ctrls)
    {
        ctrls->action(action, action_parameters);
    }
}

/* ************************************************************************** */
