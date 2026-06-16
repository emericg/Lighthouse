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
    // build the known-clients list from SettingsManager
    loadClients();

    // start listening to connections
    startServer();

    // forward desktop volume/mute changes to the connected clients
    // (routed through LocalControls so we never reference the platform Volume backend)
    LocalControls *ctrls = LocalControls::getInstance();
    connect(ctrls, &LocalControls::volumeChanged, this, &NetworkServer::sendVolumeState);
    connect(ctrls, &LocalControls::muteChanged, this, &NetworkServer::sendVolumeState);

    SettingsManager *sm = SettingsManager::getInstance();
    // react to the server enabled being toggled
    connect(sm, &SettingsManager::netctrlChanged, this, &NetworkServer::onServerChanged);
    // react to the secure mode being toggled
    connect(sm, &SettingsManager::netctrlSecureChanged, this, &NetworkServer::onSecureModeChanged);
    // refresh the known-clients list if the settings are reset/reloaded externally
    connect(sm, &SettingsManager::netctrlClientsChanged, this, &NetworkServer::loadClients);
}

/* ************************************************************************** */

QList<QObject *> NetworkServer::getClients() const
{
    QList<QObject *> out;
    out.reserve(m_knownClients.size());
    for (NetworkClientModel *m : m_knownClients)
    {
        out.push_back(m);
    }
    return out;
}

NetworkClientModel *NetworkServer::knownClientForToken(const QString &token) const
{
    if (token.isEmpty()) return nullptr;

    for (NetworkClientModel *m : m_knownClients)
    {
        if (m->getToken() == token) return m;
    }
    return nullptr;
}

void NetworkServer::loadClients()
{
    qDeleteAll(m_knownClients);
    m_knownClients.clear();

    SettingsManager *sm = SettingsManager::getInstance();

    const QList<NetworkClientSettings> &list = sm->getNetCtrlClients();
    for (const NetworkClientSettings &s : list)
    {
        // connected/secure are runtime flags, false until a live connection authenticates
        NetworkClientModel *m = new NetworkClientModel(s, this);
        connect(m, &NetworkClientModel::enabledChanged, this, &NetworkServer::onKnownClientChanged);
        connect(m, &NetworkClientModel::nameChanged, this, &NetworkServer::onKnownClientChanged);
        m_knownClients.push_back(m);
    }

    Q_EMIT clientsChanged();
}

void NetworkServer::persistClients()
{
    SettingsManager *sm = SettingsManager::getInstance();

    QList<NetworkClientSettings> &list = sm->getNetCtrlClients();
    list.clear();

    // write the known-clients list back to SettingsManager and save
    for (const NetworkClientModel *m : std::as_const(m_knownClients))
    {
        list.push_back(m->toSettings());
    }

    sm->saveNetCtrlClients();
}

void NetworkServer::enforceRevocations()
{
    // A disabled (or forgotten) client must not stay connected, regardless of the server's secure mode
    const QList<Client *> snapshot = m_clients;
    for (Client *c : snapshot)
    {
        if (c->m_token.isEmpty()) continue;

        NetworkClientModel *m = knownClientForToken(c->m_token);
        if (!m || !m->isEnabled())
        {
            qDebug() << "NetworkServer::enforceRevocations() revoking" << c->m_peer;
            c->write(QStringLiteral("auth:denied"));
            if (c->m_connection) c->m_connection->close();
            // the disconnected signal -> closeClientConnection() removes & deletes it
        }
    }
}

void NetworkServer::onKnownClientChanged()
{
    // a known client's enabled/name was edited (from QML) : persist + enforce
    persistClients();

    // if a client was just disabled, drop its live connection
    enforceRevocations();

    Q_EMIT clientsChanged();
}

void NetworkServer::onSecureModeChanged()
{
    // server secure mode toggled (from QML) : refresh clients + enforce
    enforceRevocations();
}

void NetworkServer::onServerChanged()
{
    SettingsManager *sm = SettingsManager::getInstance();
    bool server = sm->getNetCtrl();

    // server enabled or disabled (from QML)
    if (server) startServer();
    else stopServer();
}

void NetworkServer::forgetClient(QObject *client)
{
    // forget a known client and drop its connection

    NetworkClientModel *known = qobject_cast<NetworkClientModel *>(client);
    if (!known) return;

    const QString token = known->getToken();

    // drop any live connection using this token
    const QList<Client *> snapshot = m_clients;
    for (Client *c : snapshot)
    {
        if (!token.isEmpty() && c->m_token == token)
        {
            c->write(QStringLiteral("auth:denied"));
            if (c->m_connection) c->m_connection->close();
        }
    }

    // delete client
    m_knownClients.removeOne(known);
    known->deleteLater();

    persistClients();
    Q_EMIT clientsChanged();
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
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkServer::newClientConnection);
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

        qDeleteAll(m_clients);
        m_clients.clear();

        // no live connections left: clear the runtime "connected"/"secure" flags
        for (NetworkClientModel *m : std::as_const(m_knownClients))
        {
            m->setConnected(false);
            m->setSecure(false);
        }

        m_serverRunning = false;
        Q_EMIT serverEvent();
        Q_EMIT connectionEvent();
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

    Client *client = new Client(conn);
    m_clients.push_back(client);

    //qDebug() << "NetworkServer::newClientConnection(ACCEPTED)";
    connect(conn, &QIODevice::readyRead, this, &NetworkServer::readClientData);
    connect(conn, &QAbstractSocket::disconnected, this, &NetworkServer::closeClientConnection);

    Q_EMIT connectionEvent();

    client->write(QStringLiteral("WELCOME CLIENT"));
}

Client *NetworkServer::clientForSocket(QObject *socket) const
{
    for (Client *c : m_clients)
    {
        if (c->m_connection == socket) return c;
    }
    return nullptr;
}

void NetworkServer::closeClientConnection()
{
    Client *client = clientForSocket(sender());
    if (!client) return;

    qDebug() << "NetworkServer::closeClientConnection()" << client->m_peer;

    const QString token = client->m_token;

    m_clients.removeOne(client);
    delete client;

    // update the known client's "connected" flag (unless another live connection still uses the same token)
    if (!token.isEmpty())
    {
        bool stillConnected = false;
        bool stillSecure = false;

        for (const Client *c : std::as_const(m_clients))
        {
            if (c->m_token == token)
            {
                stillConnected = true;
                if (c->m_secure) stillSecure = true;
            }
        }

        NetworkClientModel *m = knownClientForToken(token);
        if (m)
        {
            m->setConnected(stillConnected);
            m->setSecure(stillSecure);
        }
    }

    Q_EMIT connectionEvent();
    Q_EMIT clientsChanged();
}

void NetworkServer::readClientData()
{
    Client *client = clientForSocket(sender());
    if (!client) return;

    const bool secure = SettingsManager::getInstance()->getNetCtrlSecure();

    // Drain every complete message currently buffered:
    // QAbstractSocket emits readyRead once even when several messages arrived at once,
    // so we must drain every complete message here, or risk processing message with some latency...
    while (true)
    {
        QString cData;
        client->m_dataStream.startTransaction();
        client->m_dataStream >> cData;
        if (!client->m_dataStream.commitTransaction()) break;

        if (cData.startsWith("hello:"))
        {
            handleClientHello(client, cData);
        }
        else if (!secure || client->m_authenticated)
        {
            // In non-secure mode commands are accepted even from a client that never introduced itself
            processClientMessage(client, cData);
        }
        else
        {
            // Secure mode, not yet authenticated, ignore until "hello:"
        }
    }
}

void NetworkServer::handleClientHello(Client *client, const QString &cData)
{
    // register/authenticate on the hello handshake

    // 'hello:<name>:<token>:<password>'
    // name and token never contain ':'; the password (last field) may, so keep the rest

    const QStringList p = cData.mid(6).split(':');
    const QString name = p.value(0);
    const QString token = p.value(1);
    const QString password = (p.size() > 2) ? p.mid(2).join(':') : QString();

    SettingsManager *sm = SettingsManager::getInstance();
    const bool secure = sm->getNetCtrlSecure();

    // Returning client: a token we already know about
    NetworkClientModel *known = knownClientForToken(token);
    if (known)
    {
        if (!known->isEnabled())
        {
            // token exists but was revoked (disabled): refused regardless of secure mode
            qDebug() << "NetworkServer::handleClientHello() denied revoked token from" << client->m_peer;
            client->write(QStringLiteral("auth:denied"));
            if (client->m_connection) client->m_connection->close();
            return;
        }

        client->m_authenticated = true;
        client->m_secure = secure;
        client->m_name = name;
        client->m_token = token;

        if (!name.isEmpty()) known->setName(name);
        known->setLastSeen(QDateTime::currentDateTime());
        known->setConnected(true);
        known->setSecure(secure);
        persistClients();

        client->write(QStringLiteral("auth:ok"));
        sendVolumeStateTo(client);
        Q_EMIT clientsChanged();
        return;
    }

    // New client (no token, or an unknown one): enroll it
    // The shared password is only required when the server runs in secure mode
    if (secure && password != sm->getNetCtrlPassword())
    {
        qDebug() << "NetworkServer::handleClientHello() enrollment refused (password) for" << client->m_peer;
        client->write(QStringLiteral("auth:denied"));
        if (client->m_connection) client->m_connection->close();
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    NetworkClientModel *m = new NetworkClientModel(this);
    m->setName(name.isEmpty() ? QStringLiteral("client") : name);
    m->setToken(QUuid::createUuid().toString(QUuid::Id128));
    m->setEnabled(true);
    m->setFirstSeen(now);
    m->setLastSeen(now);
    m->setSecure(secure);
    m->setConnected(true);
    connect(m, &NetworkClientModel::enabledChanged, this, &NetworkServer::onKnownClientChanged);
    connect(m, &NetworkClientModel::nameChanged, this, &NetworkServer::onKnownClientChanged);
    m_knownClients.push_back(m);

    client->m_authenticated = true;
    client->m_secure = secure;
    client->m_name = name;
    client->m_token = m->getToken();
    persistClients();

    qDebug() << "NetworkServer::handleClientHello() enrolled new client" << m->getName();

    client->write(QStringLiteral("auth:ok:") + m->getToken());
    sendVolumeStateTo(client);
    Q_EMIT clientsChanged();
}

void NetworkServer::processClientMessage(Client *client, const QString &cData)
{
    Q_UNUSED(client)

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
        //LocalControls *ctrls = LocalControls::getInstance();
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

void NetworkServer::sendVolumeStateTo(Client *client)
{
    if (!client) return;

    LocalControls *ctrls = LocalControls::getInstance();
    const float level = ctrls->getVolumeLevel();
    const int pct = (level < 0.f) ? 0 : qRound(level * 100.f);
    const int muted = ctrls->isMuted() ? 1 : 0;

    client->write(QStringLiteral("volume:state:%1;%2").arg(pct).arg(muted));
}

void NetworkServer::sendVolumeState()
{
    const bool secure = SettingsManager::getInstance()->getNetCtrlSecure();

    for (Client *c : std::as_const(m_clients))
    {
        // in secure mode only authenticated clients are addressed
        if (!secure || c->m_authenticated) sendVolumeStateTo(c);
    }
}

/* ************************************************************************** */
