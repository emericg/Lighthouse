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
 * \date      2022
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "network_client.h"
#include "SettingsManager.h"
#include "local_controls/local_actions.h"
#include "utils_wifi.h"

#include <QtNetwork>
#include <QSysInfo>

/* ************************************************************************** */

NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    m_tcpSocket = new QTcpSocket(this);

    m_dataInput.setDevice(m_tcpSocket);
    m_dataInput.setVersion(QDataStream::Qt_6_0);

    connect(m_tcpSocket, &QIODevice::readyRead, this, &NetworkClient::readMetadata);
    connect(m_tcpSocket, &QAbstractSocket::connected, this, &NetworkClient::connected);
    connect(m_tcpSocket, &QAbstractSocket::disconnected, this, &NetworkClient::disconnected);
    connect(m_tcpSocket, &QAbstractSocket::errorOccurred, this, &NetworkClient::displayError);
}

/* ************************************************************************** */

void NetworkClient::connectToServer()
{
    m_ssid = SettingsManager::getInstance()->getNetCtrlSSID();
    m_host = SettingsManager::getInstance()->getNetCtrlHost();
    m_port = SettingsManager::getInstance()->getNetCtrlPort();

    m_tcpSocket->abort();

    //qDebug() << "NetworkClient::connectToServer(" << m_host << "/" << m_port << ")";

    if (!m_ssid.isEmpty())
    {
        UtilsWiFi *wf = UtilsWiFi::getInstance();
        wf->refreshWiFi();
        QString ssid_current = wf->getCurrentSSID();

        if (m_ssid == ssid_current)
        {
            m_wifi = true;
            Q_EMIT wifiEvent();
        }
        else
        {
            // this is not our WiFi, no need to attempt a connection
            m_wifi = false;
            Q_EMIT wifiEvent();
            return;
        }
    }

    m_tcpSocket->connectToHost(m_host, m_port);
}

void NetworkClient::disconnectFromServer()
{
    //qDebug() << "NetworkClient::disconnectFromServer()";
    m_tcpSocket->abort();
}

void NetworkClient::connected()
{
    m_connected = true;
    m_authenticated = false;

    SettingsManager *sm = SettingsManager::getInstance();

    // Send greetings + name + token + password every time and wait for the "auth:ok" reply
    const QString hello = QStringLiteral("hello:")
                           + sm->getNetClientName() + " (" + QSysInfo::productType() + ")"
                           + ":" + sm->getNetClientToken()
                           + ":" + sm->getNetCtrlPassword();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << hello;
    m_tcpSocket->write(block);

    Q_EMIT connectionEvent();
}

void NetworkClient::disconnected()
{
    //qDebug() << "NetworkClient::disconnected()";
    m_connected = false;
    m_authenticated = false;
    Q_EMIT connectionEvent();
}

void NetworkClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        qWarning() << "RemoteHostClosedError";
        break;
    case QAbstractSocket::HostNotFoundError:
        qWarning() << "The host was not found. Please check the host name and port settings";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qWarning() << "The connection was refused by the peer. Make sure settings are correct.";
        break;
    default:
        qWarning() << "The following error occurred:" << m_tcpSocket->errorString();
        break;
    }
}

/* ************************************************************************** */

void NetworkClient::readMetadata()
{
    // Drain every complete message currently buffered:
    // A single readyRead can cover several messages, and reading just one would leave
    // the rest queued until the next message arrives, lagging the UI one update behind...
    while (true)
    {
        m_dataInput.startTransaction();

        QString metadata;
        m_dataInput >> metadata;

        if (!m_dataInput.commitTransaction()) break;

        qDebug() << "NetworkClient::readMetadata() >" << metadata;

        if (metadata.startsWith("auth:ok:"))
        {
            // Enrolled: the server issued us a token, persist it for next time
            SettingsManager::getInstance()->setNetClientToken(metadata.mid(8));
            m_authenticated = true;
            Q_EMIT connectionEvent();
        }
        else if (metadata == "auth:ok")
        {
            m_authenticated = true;
            Q_EMIT connectionEvent();
        }
        else if (metadata == "auth:denied")
        {
            // We were denied (token disabled by the server, or a wrong password).
            // Keep our token: if the server re-enables us, the same token works again.
            // Clearing it here would let a disabled client re-enroll as a brand-new entry, defeating the revocation.
            m_authenticated = false;
            Q_EMIT authError();
            Q_EMIT connectionEvent();
        }
        else if (metadata.startsWith("volume:state:"))
        {
            const QStringList p = metadata.mid(13).split(';');
            if (p.size() >= 2)
            {
                m_volume = qBound(0, p.at(0).toInt(), 100) / 100.f;
                m_volumeMuted = (p.at(1).toInt() != 0);
                Q_EMIT volumeStateChanged();
            }
        }
    }
}

void NetworkClient::sendCommand(const QString &cmd)
{
    if (m_tcpSocket->isOpen() && m_authenticated)
    {
        QByteArray block;
        QDataStream dataOutput(&block, QIODevice::WriteOnly);
        dataOutput.setVersion(QDataStream::Qt_6_0);

        dataOutput << cmd;

        m_tcpSocket->write(block);
    }
}

void NetworkClient::sendAction(int action)
{
    //qDebug() << "NetworkClient::sendAction()";

    if (m_tcpSocket->isOpen() && m_authenticated)
    {
        QByteArray block;
        QDataStream dataOutput(&block, QIODevice::WriteOnly);
        dataOutput.setVersion(QDataStream::Qt_6_0);

        if (action == LocalActions::ACTION_KEYBOARD_computer_lock) dataOutput << QString("press:lock");
        else if (action == LocalActions::ACTION_KEYBOARD_computer_sleep) dataOutput << QString("press:sleep");
        else if (action == LocalActions::ACTION_KEYBOARD_computer_poweroff) dataOutput << QString("press:poweroff");

        else if (action == LocalActions::ACTION_KEYBOARD_media_playpause) dataOutput << QString("press:playpause");
        else if (action == LocalActions::ACTION_KEYBOARD_media_stop) dataOutput << QString("press:stop");
        else if (action == LocalActions::ACTION_KEYBOARD_media_next) dataOutput << QString("press:next");
        else if (action == LocalActions::ACTION_KEYBOARD_media_prev) dataOutput << QString("press:prev");

        else if (action == LocalActions::ACTION_KEYBOARD_volume_mute) dataOutput << QString("press:mute");
        else if (action == LocalActions::ACTION_KEYBOARD_volume_up) dataOutput << QString("press:volumeup");
        else if (action == LocalActions::ACTION_KEYBOARD_volume_down) dataOutput << QString("press:volumedown");

        else if (action == LocalActions::ACTION_KEYBOARD_up) dataOutput << QString("press:up");
        else if (action == LocalActions::ACTION_KEYBOARD_down) dataOutput << QString("press:down");
        else if (action == LocalActions::ACTION_KEYBOARD_left) dataOutput << QString("press:left");
        else if (action == LocalActions::ACTION_KEYBOARD_right) dataOutput << QString("press:right");
        else if (action == LocalActions::ACTION_KEYBOARD_enter) dataOutput << QString("press:enter");
        else if (action == LocalActions::ACTION_KEYBOARD_escape) dataOutput << QString("press:escape");

        else
        {
            qWarning() << "Unknown action code:" << action;
            return;
        }

        m_tcpSocket->write(block);
    }
}

void NetworkClient::sendKey(QChar key)
{
    //qDebug() << "NetworkClient::sendKey(" << key << ")";

    if (m_tcpSocket->isOpen() && m_authenticated)
    {
        QByteArray block;
        QDataStream dataOutput(&block, QIODevice::WriteOnly);
        dataOutput.setVersion(QDataStream::Qt_6_0);

        dataOutput << QString("key:") + key;

        m_tcpSocket->write(block);
    }
}

void NetworkClient::sendGamepad(float x1, float y1, float x2, float y2,
                                int a, int b, int x, int y)
{
    //qDebug() << "NetworkClient::sendGamepad(" << key << ")";

    if (m_tcpSocket->isOpen() && m_authenticated)
    {
        QByteArray block;
        QDataStream dataOutput(&block, QIODevice::WriteOnly);
        dataOutput.setVersion(QDataStream::Qt_6_0);

        dataOutput << QString("pad:") +
                      QString::number(x1, 'g') + ";" +
                      QString::number(y1, 'g') + ";" +
                      QString::number(x2, 'g') + ";" +
                      QString::number(y2, 'g') + ";";

        m_tcpSocket->write(block);
    }
}

/* ************************************************************************** */

void NetworkClient::key_up()
{
    sendAction(LocalActions::ACTION_KEYBOARD_up);
}
void NetworkClient::key_down()
{
    sendAction(LocalActions::ACTION_KEYBOARD_down);
}
void NetworkClient::key_left()
{
    sendAction(LocalActions::ACTION_KEYBOARD_left);
}
void NetworkClient::key_right()
{
    sendAction(LocalActions::ACTION_KEYBOARD_right);
}
void NetworkClient::key_enter()
{
    sendAction(LocalActions::ACTION_KEYBOARD_enter);
}
void NetworkClient::key_escape()
{
    sendAction(LocalActions::ACTION_KEYBOARD_escape);
}

/* ************************************************************************** */

void NetworkClient::media_prev()
{
    sendAction(LocalActions::ACTION_KEYBOARD_media_prev);
}
void NetworkClient::media_playpause()
{
    sendAction(LocalActions::ACTION_KEYBOARD_media_playpause);
}
void NetworkClient::media_stop()
{
    sendAction(LocalActions::ACTION_KEYBOARD_media_stop);
}
void NetworkClient::media_next()
{
    sendAction(LocalActions::ACTION_KEYBOARD_media_next);
}

/* ************************************************************************** */

// Volume goes through the real desktop Volume backend (not the keyboard fake),
// so the desktop pushes its actual level/mute state back to us via "volume:state:".

void NetworkClient::volume_mute()
{
    sendCommand(QStringLiteral("volume:toggle"));
}
void NetworkClient::volume_down()
{
    sendCommand(QStringLiteral("volume:down"));
}
void NetworkClient::volume_up()
{
    sendCommand(QStringLiteral("volume:up"));
}
void NetworkClient::volume_set(int pct)
{
    sendCommand(QStringLiteral("volume:set:") + QString::number(qBound(0, pct, 100)));
}
void NetworkClient::volume_get()
{
    sendCommand(QStringLiteral("volume:get"));
}

/* ************************************************************************** */
