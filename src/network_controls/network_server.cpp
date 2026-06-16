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

    // forward desktop volume/mute changes to the connected client
    // (routed through LocalControls so we never reference the platform Volume backend)
    LocalControls *ctrls = LocalControls::getInstance();
    connect(ctrls, &LocalControls::volumeChanged, this, &NetworkServer::sendVolumeState);
    connect(ctrls, &LocalControls::muteChanged, this, &NetworkServer::sendVolumeState);
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
    if (m_tcpServer)
    {
        qDebug() << "NetworkServer::stopServer()";

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
    QTcpSocket *conn = m_tcpServer->nextPendingConnection();
    if (conn && conn->peerAddress().toString() == "::ffff:127.0.0.1") return;
    if (conn && conn->peerAddress().toString() == "127.0.0.1") return;
    if (conn && conn->localPort() != m_tcpServerPort) return;
    if (!conn) return;

    qDebug() << "NetworkServer::newClientConnection()";
    qDebug() << "- " << conn->peerAddress();
    qDebug() << "- " << conn->peerName();
    qDebug() << "- " << conn->peerPort();
    qDebug() << "- " << conn->localPort();

    if (m_clientConnection)
    {
        m_clientConnection->close();
        m_clientConnection->deleteLater();
    }

    m_clientConnection = conn;
    if (m_clientConnection)
    {
        //qDebug() << "NetworkServer::newClientConnection(ACCEPTED)";
        connect(m_clientConnection, &QIODevice::readyRead, this, &NetworkServer::readClientData);
        connect(m_clientConnection, &QAbstractSocket::disconnected, this, &NetworkServer::closeClientConnection);

        m_clientConnected = true;
        Q_EMIT connectionEvent();

        //m_clientDataStream.abortTransaction();
        m_clientDataStream.setDevice(m_clientConnection);
        m_clientDataStream.setVersion(QDataStream::Qt_6_0);

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << QString("WELCOME CLIENT");

        m_clientConnection->write(block);

        // let the freshly connected client know the current volume state
        sendVolumeState();
    }
}

void NetworkServer::closeClientConnection()
{
    if (m_clientConnection)
    {
        qDebug() << "NetworkServer::closeClientConnection()";

        m_clientConnection->close();
        m_clientConnection->deleteLater();
        m_clientConnection = nullptr;
    }

    m_clientConnected = false;
    Q_EMIT connectionEvent();
}

void NetworkServer::readClientData()
{
    //qDebug() << "NetworkServer::readClientData()";
    // QAbstractSocket emits readyRead once even when several messages arrived at once,
    // so we must drain every complete message here, or risk processing message with some latency.
    while (true)
    {
        QString cData;
        m_clientDataStream.startTransaction();
        m_clientDataStream >> cData;
        if (!m_clientDataStream.commitTransaction()) break;

        processClientMessage(cData);
    }
}

void NetworkServer::processClientMessage(const QString &cData)
{
    qDebug() << "NetworkServer::processClientMessage() >" << cData;

    if (cData.startsWith("press:"))
    {
        LocalControls *ctrls = LocalControls::getInstance();
        if (cData == "press:lock") ctrls->action(LocalActions::ACTION_KEYBOARD_computer_lock);
        else if (cData == "press:sleep") ctrls->action(LocalActions::ACTION_KEYBOARD_computer_sleep);
        else if (cData == "press:poweroff") ctrls->action(LocalActions::ACTION_KEYBOARD_computer_poweroff);

        else if (cData == "press:playpause") ctrls->action(LocalActions::ACTION_KEYBOARD_media_playpause);
        else if (cData == "press:stop") ctrls->action(LocalActions::ACTION_KEYBOARD_media_stop);
        else if (cData == "press:next") ctrls->action(LocalActions::ACTION_KEYBOARD_media_next);
        else if (cData == "press:prev") ctrls->action(LocalActions::ACTION_KEYBOARD_media_prev);

        else if (cData == "press:mute") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_mute);
        else if (cData == "press:volumeup") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_up);
        else if (cData == "press:volumedown") ctrls->action(LocalActions::ACTION_KEYBOARD_volume_down);

        else if (cData == "press:up") ctrls->action(LocalActions::ACTION_KEYBOARD_up);
        else if (cData == "press:down") ctrls->action(LocalActions::ACTION_KEYBOARD_down);
        else if (cData == "press:left") ctrls->action(LocalActions::ACTION_KEYBOARD_left);
        else if (cData == "press:right") ctrls->action(LocalActions::ACTION_KEYBOARD_right);
        else if (cData == "press:enter") ctrls->action(LocalActions::ACTION_KEYBOARD_enter);
        else if (cData == "press:escape") ctrls->action(LocalActions::ACTION_KEYBOARD_escape);
    }
    else if (cData.startsWith("key:"))
    {
        LocalControls *ctrls = LocalControls::getInstance();
        ctrls->keyboard_key(cData.back());
    }
    else if (cData.startsWith("mouse:"))
    {
        LocalControls *ctrls = LocalControls::getInstance();
        //ctrls->mouse_action(int x, int y, int btn_left, int btn_right, int btn_middle);
    }
    else if (cData.startsWith("pad:"))
    {
        QStringList axis = cData.mid(4).split(';');

        float x1 = 0.f;
        float y1 = 0.f;
        float x2 = 0.f;
        float y2 = 0.f;

        int a = 0;
        int b = 0;
        int x = 0;
        int y = 0;

        if (axis.size() >= 4)
        {
            x1 = axis.at(0).toFloat();
            y1 = axis.at(1).toFloat();
            x2 = axis.at(2).toFloat();
            y2 = axis.at(3).toFloat();
        }
        if (axis.size() >= 8)
        {
            a = axis.at(4).toInt();
            b = axis.at(5).toInt();
            x = axis.at(6).toInt();
            y = axis.at(7).toInt();
        }

        LocalControls *ctrls = LocalControls::getInstance();
        ctrls->gamepad_action(x1*32767.f, y1*32767.f, x2*32767.f, y2*32767.f, a, b, x, y);
    }
    else if (cData.startsWith("volume:"))
    {
        LocalControls *ctrls = LocalControls::getInstance();

        if (cData == "volume:up") ctrls->volume_up();
        else if (cData == "volume:down") ctrls->volume_down();
        else if (cData == "volume:mute") ctrls->volume_mute();
        else if (cData == "volume:unmute") ctrls->volume_unmute();
        else if (cData == "volume:toggle") ctrls->volume_toggle_mute();
        else if (cData == "volume:get") sendVolumeState();
        else if (cData.startsWith("volume:set:"))
        {
            int pct = qBound(0, cData.mid(11).toInt(), 100);
            ctrls->volume_set(pct / 100.f);
            sendVolumeState();
        }
    }
}

/* ************************************************************************** */

void NetworkServer::sendVolumeState()
{
    if (!m_clientConnection || !m_clientConnected) return;

    LocalControls *ctrls = LocalControls::getInstance();
    const float level = ctrls->getVolumeLevel();
    const int pct = (level < 0.f) ? 0 : qRound(level * 100.f);
    const int muted = ctrls->isMuted() ? 1 : 0;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << QString("volume:state:%1;%2").arg(pct).arg(muted);

    m_clientConnection->write(block);
}

/* ************************************************************************** */
