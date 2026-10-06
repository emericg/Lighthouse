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
#include "network_protocol.h"
#include "SettingsManager.h"
#include "local_controls/local_controls.h"
#include "local_controls/local_actions.h"
#include "local_monitors/ClaudeMonitor.h"
#include "local_monitors/InputMonitor.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include <QBuffer>
#include <QFile>
#include <QUrl>
#include <QUuid>

#include <algorithm>

static constexpr qint64 kMaxInboundBytes = 64 * 1024;      //!< client messages are tiny, more is abuse
static constexpr qint64 kMaxArtBytes = 8 * 1024 * 1024;    //!< album art is never that big
static constexpr int kArtMaxSize = 768;                     //!< clients only show a thumbnail

/*!
 * \return True if the image has at least one pixel that is not fully opaque.
 */
static bool hasTransparency(const QImage &img)
{
    if (!img.hasAlphaChannel()) return false;

    const QImage argb = img.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < argb.height(); y++)
    {
        const QRgb *line = reinterpret_cast<const QRgb *>(argb.constScanLine(y));
        for (int x = 0; x < argb.width(); x++)
        {
            if (qAlpha(line[x]) < 255) return true;
        }
    }

    return false;
}

/* ************************************************************************** */

/*!
 * \brief A single remote connection handled by the NetworkServer.
 *
 * Owns its socket and the QDataStream used to (de)serialize the message frames.
 * The token is filled in once the client authenticates,
 * in non-secure mode the connection is accepted without authentication.
 */
class ServerConnection
{
public:
    QTcpSocket *m_socket = nullptr;
    QDataStream m_dataStream;
    bool m_authenticated = false;
    QString m_token;
    QString m_peer;

    explicit ServerConnection(QTcpSocket *socket) : m_socket(socket)
    {
        m_dataStream.setDevice(m_socket);
        m_dataStream.setVersion(QDataStream::Qt_6_0);
        m_peer = m_socket->peerAddress().toString();
    }

    /*!
     * \note The socket signals are disconnected first,
     *       or closing it would re-enter NetworkServer::closeClientConnection().
     */
    ~ServerConnection()
    {
        m_socket->disconnect();
        m_socket->close();
        m_socket->deleteLater();
    }

    /*!
     * \brief Serialize and queue a message frame, empty messages are ignored.
     */
    void write(const QByteArray &msg)
    {
        if (msg.isEmpty() || !m_socket->isOpen()) return;

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << msg;
        m_socket->write(block);
    }

    /*!
     * \brief Tell the client it is denied, and close the connection.
     * \note The reply is still pending when this returns, so the socket is only closed
     *       (and this object deleted) later, from the event loop.
     */
    void deny()
    {
        write(QByteArrayLiteral("auth:denied"));
        m_socket->disconnectFromHost();
    }
};

/* ************************************************************************** */
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

    // forward desktop media playback/metadata changes to the connected clients
    connect(ctrls, &LocalControls::mediaChanged, this, &NetworkServer::sendMediaState);
    connect(ctrls, &LocalControls::mediaMetadataChanged, this, &NetworkServer::sendMediaMetadata);

    // forward the Claude Code plan limits; only limitsChanged is relayed, the per second
    // countdown is interpolated client side rather than pushed over the network
    connect(ClaudeMonitor::getInstance(), &ClaudeMonitor::limitsChanged,
            this, &NetworkServer::sendClaudeState);
    connect(ClaudeMonitor::getInstance(), &ClaudeMonitor::probeChanged,
            this, &NetworkServer::sendClaudeState);

    // forward the typing activity, with the input devices only opened while
    // the feature is turned on and a client is connected
    InputMonitor *im = InputMonitor::getInstance();
    connect(im, &InputMonitor::typingChanged, this, &NetworkServer::sendTypingState);
    connect(im, &InputMonitor::availableChanged, this, &NetworkServer::sendTypingState);
    connect(this, &NetworkServer::connectionEvent, this, &NetworkServer::updateInputMonitor);
    connect(SettingsManager::getInstance(), &SettingsManager::monitorInputChanged,
            this, &NetworkServer::updateInputMonitor);

    // the media backend's initial metadata fires before we connect above, so prime the art
    // cache here for whatever is already playing (no clients yet: this just fills the cache)
    refreshArt(ctrls->getMediaArtUrl());

    SettingsManager *sm = SettingsManager::getInstance();
    // react to the server enabled being toggled
    connect(sm, &SettingsManager::netctrlChanged, this, &NetworkServer::onServerChanged);
    // react to the secure mode being toggled
    connect(sm, &SettingsManager::netctrlSecureChanged, this, &NetworkServer::enforceRevocations);
    // refresh the known-clients list if the settings are reset/reloaded externally
    connect(sm, &SettingsManager::netctrlClientsChanged, this, &NetworkServer::loadClients);
}

/* ************************************************************************** */
/* ************************************************************************** */

QList<QObject *> NetworkServer::getClients() const
{
    return QList<QObject *>(m_knownClients.cbegin(), m_knownClients.cend());
}

/* ************************************************************************** */

void NetworkServer::loadClients()
{
    // QML may still hold the previous models, so they are not deleted right away
    for (NetworkClientModel *m : std::as_const(m_knownClients))
    {
        m->deleteLater();
    }
    m_knownClients.clear();

    const SettingsManager *sm = SettingsManager::getInstance();
    for (const NetworkClientSettings &s : sm->getNetCtrlClients())
    {
        addKnownClient(new NetworkClientModel(s, this));
    }

    // connected is a runtime flag, restore it from the live connections
    for (const ServerConnection *c : std::as_const(m_clients))
    {
        updateKnownClientState(c->m_token);
    }

    Q_EMIT clientsChanged();
}

void NetworkServer::addKnownClient(NetworkClientModel *client)
{
    connect(client, &NetworkClientModel::enabledChanged, this, &NetworkServer::onKnownClientChanged);
    connect(client, &NetworkClientModel::nameChanged, this, &NetworkServer::onKnownClientChanged);
    m_knownClients.push_back(client);
}

void NetworkServer::saveClients()
{
    SettingsManager *sm = SettingsManager::getInstance();

    QList<NetworkClientSettings> &list = sm->getNetCtrlClients();
    list.clear();
    for (const NetworkClientModel *m : std::as_const(m_knownClients))
    {
        list.push_back(m->toSettings());
    }

    sm->saveNetCtrlClients();
}

/* ************************************************************************** */

ServerConnection *NetworkServer::connectionForSocket(QObject *socket) const
{
    for (ServerConnection *c : m_clients)
    {
        if (c->m_socket == socket) return c;
    }
    return nullptr;
}

bool NetworkServer::isAddressable(const ServerConnection *conn) const
{
    return conn->m_authenticated || !SettingsManager::getInstance()->getNetCtrlSecure();
}

bool NetworkServer::areClientsConnected() const
{
    return std::any_of(m_clients.cbegin(), m_clients.cend(),
                       [this](const ServerConnection *c) { return isAddressable(c); });
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

void NetworkServer::updateKnownClientState(const QString &token)
{
    NetworkClientModel *m = knownClientForToken(token);
    if (!m) return;

    // several live connections may share the same token
    const bool connected = std::any_of(m_clients.cbegin(), m_clients.cend(),
                                       [&token](const ServerConnection *c) { return c->m_token == token; });
    m->setConnected(connected);
}

void NetworkServer::enforceRevocations()
{
    const bool secure = SettingsManager::getInstance()->getNetCtrlSecure();

    // a disabled (or forgotten) client must not stay connected, regardless of the secure mode,
    // and in secure mode neither can a client that never proved the password
    for (ServerConnection *c : std::as_const(m_clients))
    {
        if (c->m_token.isEmpty()) continue;

        NetworkClientModel *m = knownClientForToken(c->m_token);
        if (!m || !m->isEnabled() || (secure && !m->isVerified()))
        {
            qDebug() << "NetworkServer::enforceRevocations() revoking" << c->m_peer;
            c->deny();
        }
    }

    // toggling the secure mode changes which connections are addressable
    Q_EMIT connectionEvent();
}

void NetworkServer::updateLastSeen(const QList<ServerConnection *> &connections)
{
    const QDateTime now = QDateTime::currentDateTime();
    bool changed = false;

    for (const ServerConnection *c : connections)
    {
        NetworkClientModel *m = knownClientForToken(c->m_token);
        if (!m) continue;

        m->setLastSeen(now);
        changed = true;
    }

    if (changed) saveClients();
}

/* ************************************************************************** */

void NetworkServer::onKnownClientChanged()
{
    // a known client's enabled/name was edited (from QML): persist, and drop it if disabled
    saveClients();
    enforceRevocations();
}

void NetworkServer::onServerChanged()
{
    if (SettingsManager::getInstance()->getNetCtrl()) startServer();
    else stopServer();
}

/* ************************************************************************** */
/* ************************************************************************** */

void NetworkServer::startServer()
{
    stopServer();

    const SettingsManager *sm = SettingsManager::getInstance();
    if (!sm->getNetCtrl()) return;

    const int port = sm->getNetCtrlPort();
    if (port > 0 && port <= 65535) m_tcpServerPort = static_cast<quint16>(port);

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkServer::newClientConnection);

    if (m_tcpServer->listen(QHostAddress::Any, m_tcpServerPort))
    {
        // advertise the first non-loopback IPv4 address, or IPv4 localhost if there is none
        m_serverAddress = QHostAddress(QHostAddress::LocalHost).toString();
        const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
        for (const QHostAddress &a : addresses)
        {
            if (!a.isLoopback() && a.protocol() == QAbstractSocket::IPv4Protocol)
            {
                m_serverAddress = a.toString();
                break;
            }
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
    if (!m_tcpServer) return;

    qDebug() << "NetworkServer::stopServer()";

    // the connections go first, their sockets are children of the QTcpServer
    const QList<ServerConnection *> clients = std::exchange(m_clients, {});
    updateLastSeen(clients);
    qDeleteAll(clients);

    m_tcpServer->close();
    delete m_tcpServer;
    m_tcpServer = nullptr;

    // no live connections left: clear the runtime "connected" flags
    for (NetworkClientModel *m : std::as_const(m_knownClients))
    {
        m->setConnected(false);
    }

    m_serverRunning = false;
    Q_EMIT serverEvent();
    Q_EMIT connectionEvent();
}

/* ************************************************************************** */

void NetworkServer::forgetClient(QObject *client)
{
    NetworkClientModel *known = qobject_cast<NetworkClientModel *>(client);
    if (!known || !m_knownClients.removeOne(known)) return;

    known->deleteLater();
    saveClients();

    // drops any live connection still using the forgotten token
    enforceRevocations();

    Q_EMIT clientsChanged();
}

/* ************************************************************************** */
/* ************************************************************************** */

void NetworkServer::newClientConnection()
{
    while (QTcpSocket *socket = m_tcpServer->nextPendingConnection())
    {
        // local connections are refused, this server is meant for remote devices
        if (socket->peerAddress().isLoopback())
        {
            socket->abort();
            socket->deleteLater();
            continue;
        }

        qDebug() << "NetworkServer::newClientConnection()" << socket->peerAddress() << socket->peerPort();

        ServerConnection *conn = new ServerConnection(socket);
        m_clients.push_back(conn);

        connect(socket, &QIODevice::readyRead, this, &NetworkServer::readClientData);
        connect(socket, &QAbstractSocket::disconnected, this, &NetworkServer::closeClientConnection);

        conn->write("welcome:" + QByteArray::number(kNetworkProtocolVersion));
    }

    Q_EMIT connectionEvent();
}

void NetworkServer::closeClientConnection()
{
    ServerConnection *conn = connectionForSocket(sender());
    if (!conn) return;

    const QString token = conn->m_token;

    m_clients.removeOne(conn);
    updateLastSeen({ conn });
    delete conn;

    updateKnownClientState(token);

    Q_EMIT connectionEvent();
}

/* ************************************************************************** */
/* ************************************************************************** */

void NetworkServer::readClientData()
{
    ServerConnection *conn = connectionForSocket(sender());
    if (!conn) return;

    const bool secure = SettingsManager::getInstance()->getNetCtrlSecure();

    // Drain every complete message currently buffered:
    // QAbstractSocket emits readyRead once even when several messages arrived at once,
    // so we must drain every complete message here, or risk processing message with some latency...
    while (conn->m_socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray frame;
        conn->m_dataStream.startTransaction();
        conn->m_dataStream >> frame;
        if (!conn->m_dataStream.commitTransaction()) break;

        const QString cData = QString::fromUtf8(frame);

        if (cData.startsWith("hello:"))
        {
            handleClientHello(conn, cData);
        }
        else if (!secure || conn->m_authenticated)
        {
            // in non-secure mode commands are accepted even from a client that never introduced itself
            processClientMessage(conn, cData);
        }
    }

    // an incomplete message this large is not coming from a legitimate client
    if (conn->m_socket->bytesAvailable() > kMaxInboundBytes)
    {
        qWarning() << "NetworkServer::readClientData() dropping" << conn->m_peer << "(oversized message)";
        conn->m_socket->abort();
    }
}

/* ************************************************************************** */

void NetworkServer::handleClientHello(ServerConnection *conn, const QString &cData)
{
    // 'hello:<name>:<token>:<password>'
    // name and token never contain ':', the password (last field) may, so keep the rest
    const QStringList p = cData.mid(6).split(':');
    const QString name = p.value(0);
    const QString token = p.value(1);
    const QString password = p.mid(2).join(':');

    SettingsManager *sm = SettingsManager::getInstance();
    const bool secure = sm->getNetCtrlSecure();

    NetworkClientModel *known = knownClientForToken(token);
    const bool enroll = !known;
    const bool passwordOk = (password == sm->getNetCtrlPassword());
    const QString previousToken = conn->m_token;

    if (known && !known->isEnabled())
    {
        // token exists but was revoked (disabled): refused regardless of secure mode
        qDebug() << "NetworkServer::handleClientHello() denied revoked token from" << conn->m_peer;
        conn->deny();
        return;
    }

    // In secure mode the password is required, unless the token has already been verified:
    // a token issued while the server was not secure does not get a free pass
    if (secure && !passwordOk && !(known && known->isVerified()))
    {
        qDebug() << "NetworkServer::handleClientHello() refused (password) for" << conn->m_peer;
        conn->deny();
        return;
    }

    if (enroll)
    {
        // new client (no token, or an unknown one): enroll it
        known = new NetworkClientModel(this);
        known->setName(name.isEmpty() ? QStringLiteral("client") : name);
        known->setToken(QUuid::createUuid().toString(QUuid::Id128));
        known->setFirstSeen(QDateTime::currentDateTime());
        addKnownClient(known);

        qDebug() << "NetworkServer::handleClientHello() enrolled new client" << known->getName();
    }
    else if (!name.isEmpty())
    {
        known->setName(name);
    }

    conn->m_authenticated = true;
    conn->m_token = known->getToken();

    // the password is checked whatever the mode, so a client can be verified before secure mode is on
    if (passwordOk) known->setVerified(true);
    known->setLastSeen(QDateTime::currentDateTime());
    updateKnownClientState(conn->m_token);
    if (previousToken != conn->m_token) updateKnownClientState(previousToken);
    saveClients();

    // a newly enrolled client gets its token, to present on its next connections
    conn->write(enroll ? "auth:ok:" + conn->m_token.toUtf8() : QByteArrayLiteral("auth:ok"));
    sendFullStateTo(conn);

    if (enroll) Q_EMIT clientsChanged();
    Q_EMIT connectionEvent();
}

/* ************************************************************************** */

void NetworkServer::processClientMessage(ServerConnection *conn, const QString &cData)
{
    //qDebug() << "NetworkServer::processClientMessage() >" << cData;

    LocalControls *ctrls = LocalControls::getInstance();

    if (cData.startsWith("press:"))
    {
        const int action = networkPressActionFromName(QStringView(cData).mid(6));
        if (action >= 0) ctrls->action(action);
    }
    else if (cData.startsWith("text:"))
    {
        ctrls->keyboard_text(QStringView(cData).mid(5));
    }
    else if (cData.startsWith("mouse:"))
    {
        const QStringList p = cData.mid(6).split(';');
        const QString sub = p.value(0);

        if (sub == "m" && p.size() >= 3) // relative move
        {
            ctrls->mouse_move(p.at(1).toInt(), p.at(2).toInt());
        }
        else if (sub == "s" && p.size() >= 3) // scroll
        {
            ctrls->mouse_scroll(p.at(1).toInt(), p.at(2).toInt());
        }
        else if (sub == "c" && p.size() >= 2) // click (press + release)
        {
            const int btn = p.at(1).toInt();
            if (btn == 1) ctrls->action(LocalActions::ACTION_MOUSE_click_right);
            else if (btn == 2) ctrls->action(LocalActions::ACTION_MOUSE_click_middle);
            else ctrls->action(LocalActions::ACTION_MOUSE_click_left);
        }
        else if (sub == "b" && p.size() >= 3) // button hold/release (drag)
        {
            ctrls->mouse_button(p.at(1).toInt(), p.at(2).toInt() != 0);
        }
    }
    else if (cData.startsWith("pad:"))
    {
        // pad:<x1>;<y1>;<x2>;<y2>[;<a>;<b>;<x>;<y>], axes normalized to [-1 ; 1]
        const QStringList p = cData.mid(4).split(';');
        if (p.size() < 4) return;

        auto axis = [&p](int i) { return qBound(-1.f, p.at(i).toFloat(), 1.f) * 32767.f; };
        auto button = [&p](int i) { return p.value(i).toInt(); };

        ctrls->gamepad_action(axis(0), axis(1), axis(2), axis(3),
                              button(4), button(5), button(6), button(7));
    }
    else if (cData.startsWith("volume:"))
    {
        if (cData == "volume:up") ctrls->volume_up();
        else if (cData == "volume:down") ctrls->volume_down();
        else if (cData == "volume:mute") ctrls->volume_mute();
        else if (cData == "volume:unmute") ctrls->volume_unmute();
        else if (cData == "volume:toggle") ctrls->volume_toggle_mute();
        else if (cData == "volume:get") conn->write(volumeStateMessage());
        else if (cData.startsWith("volume:set:"))
        {
            const int pct = qBound(0, cData.mid(11).toInt(), 100);
            ctrls->volume_set(pct / 100.f);
            sendVolumeState();
        }
    }
    else if (cData == "claude:probe")
    {
        // A probe consumes plan quota, so it is only honored when there is nothing fresh to show:
        // a client repeating the command cannot burn through the quota
        ClaudeMonitor *cm = ClaudeMonitor::getInstance();
        if (cm->getCaptureState() != ClaudeMonitor::CaptureLive) cm->probe();
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void NetworkServer::broadcast(const QByteArray &msg)
{
    if (msg.isEmpty()) return;

    for (ServerConnection *c : std::as_const(m_clients))
    {
        if (isAddressable(c)) c->write(msg);
    }
}

void NetworkServer::sendFullStateTo(ServerConnection *conn)
{
    conn->write(volumeStateMessage());
    conn->write(mediaStateMessage());
    conn->write(mediaMetadataMessage());
    conn->write(mediaArtMessage());
    conn->write(claudeStateMessage());
    conn->write(typingStateMessage());
}

void NetworkServer::sendVolumeState() { broadcast(volumeStateMessage()); }
void NetworkServer::sendMediaState() { broadcast(mediaStateMessage()); }
void NetworkServer::sendClaudeState() { broadcast(claudeStateMessage()); }
void NetworkServer::sendTypingState() { broadcast(typingStateMessage()); }

void NetworkServer::sendMediaMetadata()
{
    broadcast(mediaMetadataMessage());

    // the artwork is heavy, so it is (re)loaded and broadcast only when the URL actually changes
    refreshArt(LocalControls::getInstance()->getMediaArtUrl());
}

void NetworkServer::updateInputMonitor()
{
    const bool wanted = SettingsManager::getInstance()->getMonitorInput() && areClientsConnected();
    InputMonitor::getInstance()->setActive(wanted);
}

/* ************************************************************************** */

QByteArray NetworkServer::volumeStateMessage() const
{
    LocalControls *ctrls = LocalControls::getInstance();
    const float level = ctrls->getVolumeLevel();

    // volume:state:<percent>;<muted>
    return QStringLiteral("volume:state:%1;%2")
        .arg((level < 0.f) ? 0 : qRound(level * 100.f))
        .arg(ctrls->isMuted() ? 1 : 0).toUtf8();
}

QByteArray NetworkServer::mediaStateMessage() const
{
    LocalControls *ctrls = LocalControls::getInstance();

    // media:state:<playerId>;<status>;<position_us>;<duration_us>
    return QStringLiteral("media:state:%1;%2;%3;%4")
        .arg(ctrls->getMediaPlayerId())
        .arg(ctrls->getMediaStatus())
        .arg(ctrls->getMediaPosition_us())
        .arg(ctrls->getMediaDuration_us()).toUtf8();
}

QByteArray NetworkServer::mediaMetadataMessage() const
{
    LocalControls *ctrls = LocalControls::getInstance();

    // free-text fields can contain any character, so carry them as a compact JSON blob
    QJsonObject o;
    o["playerId"] = ctrls->getMediaPlayerId();
    o["player"] = ctrls->getMediaPlayerName();
    o["title"] = ctrls->getMediaTitle();
    o["artist"] = ctrls->getMediaArtist();
    o["album"] = ctrls->getMediaAlbum();

    return "media:meta:" + QJsonDocument(o).toJson(QJsonDocument::Compact);
}

QByteArray NetworkServer::mediaArtMessage() const
{
    // media:art:<mime>;<raw image bytes>
    if (m_artBytes.isEmpty()) return QByteArrayLiteral("media:art:");

    return "media:art:" + m_artMime.toUtf8() + ';' + m_artBytes;
}

QByteArray NetworkServer::claudeStateMessage() const
{
    // Compiled out: nothing to report, stay off the wire entirely.
    // Turned off in the settings: the cleared state still goes out, so clients hide it.
    if (!ClaudeMonitor::isSupported()) return QByteArray();
    ClaudeMonitor *cm = ClaudeMonitor::getInstance();

    // Countdowns travel as seconds remaining rather than as absolute reset dates:
    // the client ticks them down on its own clock, which cannot drift against ours.
    // A negative percentage already means "this window is unknown", so no separate
    // per window validity flag is carried.
    QJsonObject fiveHour;
    fiveHour["percent"] = cm->getFiveHourPercent();
    fiveHour["remaining"] = cm->getFiveHourRemaining();

    QJsonObject sevenDay;
    sevenDay["percent"] = cm->getSevenDayPercent();
    sevenDay["remaining"] = cm->getSevenDayRemaining();

    // lets a client offer a probe, and follow its progress
    QJsonObject probe;
    probe["available"] = cm->isProbeAvailable();
    probe["state"] = cm->getProbeState();

    QJsonObject o;
    o["state"] = cm->getCaptureState();
    o["fiveHour"] = fiveHour;
    o["sevenDay"] = sevenDay;
    o["probe"] = probe;

    return "claude:state:" + QJsonDocument(o).toJson(QJsonDocument::Compact);
}

QByteArray NetworkServer::typingStateMessage() const
{
    InputMonitor *im = InputMonitor::getInstance();

    // typing:state:<available>;<typing>
    return QStringLiteral("typing:state:%1;%2")
        .arg(im->isAvailable() ? 1 : 0)
        .arg(im->isTyping() ? 1 : 0).toUtf8();
}

/* ************************************************************************** */

void NetworkServer::refreshArt(const QString &url)
{
    // already loaded (or currently loading) this exact URL: nothing to do
    if (url == m_artUrl) return;

    m_artUrl = url;
    m_artMime.clear();
    m_artBytes.clear();

    if (url.startsWith("http://") || url.startsWith("https://"))
    {
        // online remote art: fetch asynchronously, then embed the bytes like any other source
        if (!m_nam) m_nam = new QNetworkAccessManager(this);

        QNetworkReply *reply = m_nam->get(QNetworkRequest(QUrl(url)));
        connect(reply, &QNetworkReply::downloadProgress, reply, [reply](qint64 received, qint64) {
            if (received > kMaxArtBytes) reply->abort();
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
            reply->deleteLater();

            // the track may have moved on while we were fetching: only apply if still current
            if (url != m_artUrl) return;

            setArt((reply->error() == QNetworkReply::NoError) ? reply->read(kMaxArtBytes) : QByteArray());
        });
        return;
    }

    QByteArray bytes;

    if (url.startsWith("data:"))
    {
        // data:[<mime>][;base64],<payload>
        const int comma = url.indexOf(',');
        if (comma > 5)
        {
            const QByteArray payload = url.mid(comma + 1).toLatin1();
            bytes = url.left(comma).contains(";base64") ? QByteArray::fromBase64(payload)
                                                        : QByteArray::fromPercentEncoding(payload);
        }
    }
    else if (!url.isEmpty())
    {
        // otherwise assume a local file (file:// URL or a bare file path)
        const QUrl u(url);
        QFile f(u.isLocalFile() ? u.toLocalFile() : url);
        if (f.open(QIODevice::ReadOnly)) bytes = f.read(kMaxArtBytes);
    }

    setArt(bytes);
}

void NetworkServer::setArt(const QByteArray &bytes)
{
    m_artBytes.clear();
    m_artMime.clear();

    QImage img;
    if (!bytes.isEmpty() && img.loadFromData(bytes))
    {
        if (img.width() > kArtMaxSize || img.height() > kArtMaxSize)
        {
            img = img.scaled(kArtMaxSize, kArtMaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // JPEG unless the image actually uses transparency
        const bool png = hasTransparency(img);
        QBuffer buffer(&m_artBytes);
        buffer.open(QIODevice::WriteOnly);
        if (img.save(&buffer, png ? "PNG" : "JPG", png ? -1 : 85))
        {
            m_artMime = png ? QStringLiteral("image/png") : QStringLiteral("image/jpeg");
        }
        else
        {
            m_artBytes.clear();
        }
    }

    broadcast(mediaArtMessage());
}

/* ************************************************************************** */
