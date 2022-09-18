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

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H
/* ************************************************************************** */

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

/* ************************************************************************** */

class NetworkServer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY serverEvent)
    Q_PROPERTY(bool clientConnected READ isClientConnected NOTIFY connectionEvent)

    Q_PROPERTY(QString serverAddress READ getServerAddress NOTIFY serverEvent)
    Q_PROPERTY(int serverPort READ getServerPort NOTIFY serverEvent)

    QTcpServer *m_tcpServer = nullptr;
    bool m_serverRunning = false;
    QString m_serverAddress;
    unsigned m_tcpServerPort = 5555;

    QTcpSocket *m_clientConnection = nullptr;
    QDataStream m_clientDataStream;
    bool m_clientConnected = false;

    bool isRunning() const { return m_serverRunning; }
    bool isClientConnected() const { return m_clientConnected; }
    QString getServerAddress() const { return m_serverAddress; }
    int getServerPort() const { return m_tcpServerPort; }

signals:
    void serverEvent();
    void connectionEvent();

private slots:
    void newClientConnection();
    void closeClientConnection();
    void readPress();

public:
    explicit NetworkServer(QObject *parent = nullptr);

    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();
};

/* ************************************************************************** */
#endif // NETWORK_SERVER_H
