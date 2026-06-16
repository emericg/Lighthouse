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
#include <QList>
#include <QDateTime>

#include "SettingsManager.h"

/* ************************************************************************** */

/*!
 * \brief A single remote connection handled by the NetworkServer.
 *
 * Owns its socket and the QDataStream used to (de)serialize messages. Identity
 * (name/token) is filled in once the client authenticates; in non-secure mode
 * the connection is treated as authenticated right away.
 */
class Client
{
public:
    QTcpSocket *m_connection = nullptr;
    QDataStream m_dataStream;
    bool m_authenticated = false;
    bool m_secure = false; //!< authenticated while the server was in secure mode
    QString m_name;
    QString m_token;
    QString m_peer;

    explicit Client(QTcpSocket *connection) : m_connection(connection)
    {
        m_dataStream.setDevice(m_connection);
        m_dataStream.setVersion(QDataStream::Qt_6_0);
        if (m_connection) m_peer = m_connection->peerAddress().toString();
    }

    ~Client()
    {
        if (m_connection)
        {
            m_connection->close();
            m_connection->deleteLater();
        }
    }

    void write(const QString &msg)
    {
        if (!m_connection || !m_connection->isOpen()) return;

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << msg;
        m_connection->write(block);
    }
};

/* ************************************************************************** */

/*!
 * \brief A network control client as exposed to the QML layer.
 *
 * Wraps the persisted identity (name/token/enabled/firstSeen/lastSeen) with the
 * runtime state (connected/secure). NetworkServer owns these objects and keeps
 * them in sync with the SettingsManager-persisted list.
 */
class NetworkClientModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString token READ getToken CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QDateTime firstSeen READ getFirstSeen NOTIFY firstSeenChanged)
    Q_PROPERTY(QDateTime lastSeen READ getLastSeen NOTIFY lastSeenChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool secure READ isSecure NOTIFY secureChanged)

    QString m_name;
    QString m_token;
    bool m_enabled = true;
    QDateTime m_firstSeen;
    QDateTime m_lastSeen;
    bool m_connected = false;
    bool m_secure = false;

public:
    explicit NetworkClientModel(QObject *parent = nullptr) : QObject(parent) {}

    NetworkClientModel(const NetworkClientSettings &s, QObject *parent = nullptr)
        : QObject(parent), m_name(s.name), m_token(s.token), m_enabled(s.enabled),
          m_firstSeen(s.firstSeen), m_lastSeen(s.lastSeen) {}

    NetworkClientSettings toSettings() const
    {
        NetworkClientSettings s;
        s.name = m_name;
        s.token = m_token;
        s.enabled = m_enabled;
        s.firstSeen = m_firstSeen;
        s.lastSeen = m_lastSeen;
        return s;
    }

    QString getName() const { return m_name; }
    void setName(const QString &v) { if (m_name != v) { m_name = v; Q_EMIT nameChanged(); } }

    QString getToken() const { return m_token; }
    void setToken(const QString &v) { m_token = v; }   //!< set once at creation (no NOTIFY)

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool v) { if (m_enabled != v) { m_enabled = v; Q_EMIT enabledChanged(); } }

    QDateTime getFirstSeen() const { return m_firstSeen; }
    void setFirstSeen(const QDateTime &v) { if (m_firstSeen != v) { m_firstSeen = v; Q_EMIT firstSeenChanged(); } }

    QDateTime getLastSeen() const { return m_lastSeen; }
    void setLastSeen(const QDateTime &v) { if (m_lastSeen != v) { m_lastSeen = v; Q_EMIT lastSeenChanged(); } }

    bool isConnected() const { return m_connected; }
    void setConnected(bool v) { if (m_connected != v) { m_connected = v; Q_EMIT connectedChanged(); } }

    bool isSecure() const { return m_secure; }
    void setSecure(bool v) { if (m_secure != v) { m_secure = v; Q_EMIT secureChanged(); } }

signals:
    void nameChanged();
    void enabledChanged();
    void firstSeenChanged();
    void lastSeenChanged();
    void connectedChanged();
    void secureChanged();
};

/* ************************************************************************** */

class NetworkServer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY serverEvent)

    Q_PROPERTY(bool clientConnected READ areClientsConnected NOTIFY connectionEvent)
    Q_PROPERTY(QList<QObject *> clients READ getClients NOTIFY clientsChanged)

    Q_PROPERTY(QString serverAddress READ getServerAddress NOTIFY serverEvent)
    Q_PROPERTY(int serverPort READ getServerPort NOTIFY serverEvent)

    QTcpServer *m_tcpServer = nullptr;
    bool m_serverRunning = false;
    QString m_serverAddress;
    unsigned m_tcpServerPort = 5555;

    QList <Client *> m_clients;                     //!< live clients
    QList <NetworkClientModel *> m_knownClients;    //!< known clients, exposed to QML

    bool isRunning() const { return m_serverRunning; }

    bool areClientsConnected() const { return !m_clients.isEmpty(); }

    QList <QObject *> getClients() const;

    QString getServerAddress() const { return m_serverAddress; }
    int getServerPort() const { return m_tcpServerPort; }

    Client *clientForSocket(QObject *socket) const;
    NetworkClientModel *knownClientForToken(const QString &token) const;

    void loadClients();
    void persistClients();
    void enforceRevocations();

    void handleClientHello(Client *client, const QString &cData);
    void processClientMessage(Client *client, const QString &cData);

    void sendVolumeStateTo(Client *client);

signals:
    void serverEvent();
    void connectionEvent();
    void clientsChanged();

private slots:
    void newClientConnection();
    void closeClientConnection();

    void readClientData();

    void onServerChanged();
    void onSecureModeChanged();
    void onKnownClientChanged();

    void sendVolumeState();

public:
    explicit NetworkServer(QObject *parent = nullptr);

    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();

    Q_INVOKABLE void forgetClient(QObject *client);
};

/* ************************************************************************** */
#endif // NETWORK_SERVER_H
