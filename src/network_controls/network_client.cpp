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

    m_tcpSocket->abort();

    //qDebug() << "NetworkClient::connectToServer(" << m_host << "/" << m_port << ")";
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

void NetworkClient::sendAction(int action)
{
    //qDebug() << "NetworkClient::sendAction()";

    if (m_tcpSocket->isOpen())
    {
        QByteArray block;
        QDataStream dataOutput(&block, QIODevice::WriteOnly);
        dataOutput.setVersion(QDataStream::Qt_6_0);

        if (action == LocalActions::ACTION_KEYBOARD_media_playpause) dataOutput << QString("press:playpause");
        if (action == LocalActions::ACTION_KEYBOARD_media_stop) dataOutput << QString("press:stop");
        if (action == LocalActions::ACTION_KEYBOARD_media_next) dataOutput << QString("press:next");
        if (action == LocalActions::ACTION_KEYBOARD_media_prev) dataOutput << QString("press:prev");

        if (action == LocalActions::ACTION_KEYBOARD_volume_mute) dataOutput << QString("press:mute");
        if (action == LocalActions::ACTION_KEYBOARD_volume_up) dataOutput << QString("press:volumeup");
        if (action == LocalActions::ACTION_KEYBOARD_volume_down) dataOutput << QString("press:volumedown");

        if (action == LocalActions::ACTION_KEYBOARD_up) dataOutput << QString("press:up");
        if (action == LocalActions::ACTION_KEYBOARD_down) dataOutput << QString("press:down");
        if (action == LocalActions::ACTION_KEYBOARD_left) dataOutput << QString("press:left");
        if (action == LocalActions::ACTION_KEYBOARD_right) dataOutput << QString("press:right");
        if (action == LocalActions::ACTION_KEYBOARD_enter) dataOutput << QString("press:enter");
        if (action == LocalActions::ACTION_KEYBOARD_escape) dataOutput << QString("press:escape");

        m_tcpSocket->write(block);
    }
}

void NetworkClient::sendKey(QChar key)
{
    //qDebug() << "NetworkClient::sendKey(" << key << ")";

    if (m_tcpSocket->isOpen())
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

    if (m_tcpSocket->isOpen())
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

void NetworkClient::volume_mute()
{
    sendAction(LocalActions::ACTION_KEYBOARD_volume_mute);
}
void NetworkClient::volume_down()
{
    sendAction(LocalActions::ACTION_KEYBOARD_volume_down);
}
void NetworkClient::volume_up()
{
    sendAction(LocalActions::ACTION_KEYBOARD_volume_up);
}

/* ************************************************************************** */
