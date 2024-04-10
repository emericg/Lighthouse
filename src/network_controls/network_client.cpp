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

#include <QtNetwork>

/* ************************************************************************** */

NetworkClient::NetworkClient(QObject *parent): QObject(parent)
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
    m_host = SettingsManager::getInstance()->getNetCtrlHost();
    m_port = SettingsManager::getInstance()->getNetCtrlPort();

    //qDebug() << "NetworkClient::connectToServer(" << m_host << "/" << m_port << ")";

    m_tcpSocket->abort();
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
    Q_EMIT connectionEvent();
}

void NetworkClient::disconnected()
{
    //qDebug() << "NetworkClient::disconnected()";
    m_connected = false;
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
    m_dataInput.startTransaction();

    QString metadata;
    m_dataInput >> metadata;

    if (m_dataInput.commitTransaction())
    {
        qDebug() << "NetworkClient::readMetadata() >" << metadata;
    }
    else
    {
        qWarning() << "NetworkClient::readMetadata() m_dataInput commit failed";
    }
}

void NetworkClient::sendPress(int btn)
{
    //qDebug() << "NetworkClient::sendPress()";

    QByteArray block;
    QDataStream dataOutput(&block, QIODevice::WriteOnly);
    dataOutput.setVersion(QDataStream::Qt_6_0);

    if (btn == 1) dataOutput << QString("press:playpause");
    if (btn == 2) dataOutput << QString("press:stop");
    if (btn == 3) dataOutput << QString("press:next");
    if (btn == 4) dataOutput << QString("press:prev");
    if (btn == 5) dataOutput << QString("press:mute");
    if (btn == 6) dataOutput << QString("press:volumeup");
    if (btn == 7) dataOutput << QString("press:volumedown");

    m_tcpSocket->write(block);
}

void NetworkClient::media_prev()
{
    sendPress(4);
}
void NetworkClient::media_playpause()
{
    sendPress(1);
}
void NetworkClient::media_next()
{
    sendPress(3);
}
void NetworkClient::volume_mute()
{
    sendPress(5);
}
void NetworkClient::volume_down()
{
    sendPress(7);
}
void NetworkClient::volume_up()
{
    sendPress(6);
}

/* ************************************************************************** */
