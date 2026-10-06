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
#include <QList>
#include <QDateTime>
#include <QByteArray>

#include "SettingsManager.h"

class QTcpServer;
class QNetworkAccessManager;
class ServerConnection;

/* ************************************************************************** */

/*!
 * \brief A network control client as exposed to the QML layer.
 *
 * Wraps the persisted identity (name/token/enabled/verified/firstSeen/lastSeen) with the
 * runtime state (connected). NetworkServer owns these objects and keeps
 * them in sync with the SettingsManager-persisted list.
 */
class NetworkClientModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString token READ getToken CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool verified READ isVerified NOTIFY verifiedChanged)
    Q_PROPERTY(QDateTime firstSeen READ getFirstSeen NOTIFY firstSeenChanged)
    Q_PROPERTY(QDateTime lastSeen READ getLastSeen NOTIFY lastSeenChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

    QString m_name;
    QString m_token;
    bool m_enabled = true;
    bool m_verified = false;    //!< has proven the server password at least once
    QDateTime m_firstSeen;
    QDateTime m_lastSeen;       //!< last authentication, or last disconnection
    bool m_connected = false;

public:
    explicit NetworkClientModel(QObject *parent = nullptr) : QObject(parent) {}

    NetworkClientModel(const NetworkClientSettings &s, QObject *parent = nullptr)
        : QObject(parent), m_name(s.name), m_token(s.token), m_enabled(s.enabled),
          m_verified(s.verified), m_firstSeen(s.firstSeen), m_lastSeen(s.lastSeen) {}

    NetworkClientSettings toSettings() const
    {
        NetworkClientSettings s;
        s.name = m_name;
        s.token = m_token;
        s.enabled = m_enabled;
        s.verified = m_verified;
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

    bool isVerified() const { return m_verified; }
    void setVerified(bool v) { if (m_verified != v) { m_verified = v; Q_EMIT verifiedChanged(); } }

    QDateTime getFirstSeen() const { return m_firstSeen; }
    void setFirstSeen(const QDateTime &v) { if (m_firstSeen != v) { m_firstSeen = v; Q_EMIT firstSeenChanged(); } }

    QDateTime getLastSeen() const { return m_lastSeen; }
    void setLastSeen(const QDateTime &v) { if (m_lastSeen != v) { m_lastSeen = v; Q_EMIT lastSeenChanged(); } }

    bool isConnected() const { return m_connected; }
    void setConnected(bool v) { if (m_connected != v) { m_connected = v; Q_EMIT connectedChanged(); } }

signals:
    void nameChanged();
    void enabledChanged();
    void verifiedChanged();
    void firstSeenChanged();
    void lastSeenChanged();
    void connectedChanged();
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
    quint16 m_tcpServerPort = 5555;                 //!< overridden by a valid SettingsManager port

    QList <ServerConnection *> m_clients;           //!< live connections
    QList <NetworkClientModel *> m_knownClients;    //!< known clients, exposed to QML

    QNetworkAccessManager *m_nam = nullptr;
    QString m_artUrl;                               //!< source URL the cached bytes were loaded from
    QString m_artMime;                              //!< mime type of the cached bytes (ex: "image/png")
    QByteArray m_artBytes;                          //!< raw image bytes for the current track (may be empty)

    bool isRunning() const { return m_serverRunning; }

    /*!
     * \return True if at least one live connection is allowed to receive messages.
     * \note In secure mode, the connections that did not authenticate yet are not counted.
     */
    bool areClientsConnected() const;

    QList <QObject *> getClients() const;

    QString getServerAddress() const { return m_serverAddress; }
    int getServerPort() const { return m_tcpServerPort; }

    ServerConnection *connectionForSocket(QObject *socket) const;

    /*!
     * \return True if this connection is allowed to receive messages.
     * \note In secure mode, only authenticated connections are.
     */
    bool isAddressable(const ServerConnection *conn) const;

    /*!
     * \brief Stamp the known clients behind these connections as last seen now, and persist them.
     */
    void updateLastSeen(const QList<ServerConnection *> &connections);
    NetworkClientModel *knownClientForToken(const QString &token) const;

    void addKnownClient(NetworkClientModel *client);
    void saveClients();

    /*!
     * \brief Close the live connections whose token is no longer known, or has been disabled.
     * \note In secure mode, the clients that never proved the password are closed too.
     */
    void enforceRevocations();

    /*!
     * \brief Sync a known client's runtime connected flag with the live connections.
     * \param token Token of the known client to update.
     */
    void updateKnownClientState(const QString &token);

    void handleClientHello(ServerConnection *conn, const QString &cData);
    void processClientMessage(ServerConnection *conn, const QString &cData);

    /*!
     * \brief Send a message to every addressable live connection.
     */
    void broadcast(const QByteArray &msg);

    /*!
     * \brief Send the whole desktop state to a connection that just authenticated.
     */
    void sendFullStateTo(ServerConnection *conn);

    QByteArray volumeStateMessage() const;
    QByteArray mediaStateMessage() const;
    QByteArray mediaMetadataMessage() const;
    QByteArray mediaArtMessage() const;             //!< empty payload when no art (clears the client thumbnail)
    QByteArray claudeStateMessage() const;          //!< empty when the ClaudeMonitor is compiled out
    QByteArray typingStateMessage() const;

    /*!
     * \brief (Re)load the artwork bytes if the URL changed, then broadcast them.
     * \param url A data:, http(s)://, file:// URL, or a bare local file path.
     */
    void refreshArt(const QString &url);

    /*!
     * \brief Downscale and re-encode the artwork (JPEG, or PNG if transparent), then broadcast it.
     * \note Only decodable images are kept: the URL comes from any local media player,
     *       and must not be usable to push arbitrary local files to the network clients.
     */
    void setArt(const QByteArray &bytes);

signals:
    void serverEvent();
    void connectionEvent();
    void clientsChanged();

private slots:
    void newClientConnection();
    void closeClientConnection();

    void readClientData();

    void loadClients();

    void onServerChanged();
    void onKnownClientChanged();

    void sendVolumeState();
    void sendMediaState();
    void sendMediaMetadata();
    void sendClaudeState();
    void sendTypingState();
    void updateInputMonitor();      //!< open the input devices only when enabled and someone is connected

public:
    explicit NetworkServer(QObject *parent = nullptr);

    Q_INVOKABLE void startServer();
    Q_INVOKABLE void stopServer();

    Q_INVOKABLE void forgetClient(QObject *client);
};

/* ************************************************************************** */
#endif // NETWORK_SERVER_H
