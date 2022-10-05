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

#include "network_server.h"
#include "SettingsManager.h"
#include "local_controls/local_controls.h"
#include "local_controls/local_actions.h"

#include <QtCore>
#include <QtNetwork>

/* ************************************************************************** */

NetworkServer::NetworkServer(QObject *parent) : QObject(parent)
{
    startServer();

    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkServer::newClientConnection);
}

/* ************************************************************************** */

void NetworkServer::startServer()
{
    stopServer();

    // check if we should
    SettingsManager *sm = SettingsManager::getInstance();
    if (!sm || (sm && !sm->getNetCtrl()))
    {
        return;
    }

    // start the tcp server
    m_tcpServer = new QTcpServer(this);
    if (m_tcpServer->listen(QHostAddress::Any, m_tcpServerPort))
    {
        m_serverAddress.clear();
        QList <QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

        // use the first non-localhost IPv4 address
        for (int i = 0; i < ipAddressesList.size(); ++i)
        {
            if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address())
            {
                m_serverAddress = ipAddressesList.at(i).toString();
                break;
            }
        }
        // if we did not find one, use IPv4 localhost
        if (m_serverAddress.isEmpty())
        {
            m_serverAddress = QHostAddress(QHostAddress::LocalHost).toString();
        }

        m_serverRunning = true;
        qDebug() << "NetworkServer::startServer( IP:" << m_serverAddress << "port:" << m_tcpServer->serverPort() << ")";
    }
    else
    {
        qWarning() << "Unable to start the NetworkServer:" << m_tcpServer->errorString();
        m_serverRunning = false;
    }

    Q_EMIT serverEvent();
}

void NetworkServer::stopServer()
{
    qDebug() << "NetworkServer::stopServer()";

    if (m_tcpServer)
    {
        m_tcpServer->close();

        delete m_tcpServer;
        m_tcpServer = nullptr;

        m_serverRunning = false;
        Q_EMIT serverEvent();
    }
}

/* ************************************************************************** */

void NetworkServer::newClientConnection()
{
    qDebug() << "NetworkServer::newClientConnection()";

    QTcpSocket *conn = m_tcpServer->nextPendingConnection();
    if (conn && conn->peerAddress().toString() == "127.0.0.1") return;
    if (conn && conn->localPort() != m_tcpServerPort) return;
    if (!conn) return;

    if (m_clientConnection)
    {
        m_clientConnection->close();
        m_clientConnection->deleteLater();
    }

    m_clientConnection = conn;
    if (m_clientConnection)
    {
        //qDebug() << "NetworkServer::newClientConnection(ACCEPTED)";
        connect(m_clientConnection, &QIODevice::readyRead, this, &NetworkServer::readPress);
        connect(m_clientConnection, &QAbstractSocket::disconnected, this, &NetworkServer::closeClientConnection);

        m_clientConnected = true;
        Q_EMIT connectionEvent();

        m_clientDataStream.abortTransaction();
        m_clientDataStream.setDevice(m_clientConnection);
        m_clientDataStream.setVersion(QDataStream::Qt_6_0);

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << QString("WELCOME CLIENT");

        m_clientConnection->write(block);
    }
}

void NetworkServer::closeClientConnection()
{
    //qDebug() << "NetworkServer::closeClientConnection()";

    if (m_clientConnection)
    {
        m_clientConnection->close();
        m_clientConnection->deleteLater();
        m_clientConnection = nullptr;
    }

    m_clientConnected = false;
    Q_EMIT connectionEvent();
}

void NetworkServer::readPress()
{
    m_clientDataStream.startTransaction();

    QString nextPress;
    m_clientDataStream >> nextPress;
    //qDebug() << "NetworkServer::readPress() >" << nextPress;

    if (nextPress.startsWith("press:"))
    {
        LocalControls *ctrls = LocalControls::getInstance();
        if (nextPress == "press:playpause") ctrls->action(LocalActions::ACTION_KEYBOARD_media_playpause);
        else if (nextPress == "press:next") ctrls->action(LocalActions::ACTION_KEYBOARD_media_next);
        else if (nextPress == "press:prev") ctrls->action(LocalActions::ACTION_KEYBOARD_media_prev);
        else if (nextPress == "press:mute") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_mute);
        else if (nextPress == "press:volumeup") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_up);
        else if (nextPress == "press:volumedown") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_down);
    }

    m_clientDataStream.commitTransaction();
}

/* ************************************************************************** */
