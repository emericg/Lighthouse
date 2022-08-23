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

#include "mpris_dbus.h"
#include "local_actions.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#endif

/* ************************************************************************** */

static const QString mprisObjectPath = QStringLiteral("/org/mpris/MediaPlayer2");
static const QString mprisInterface_root = QStringLiteral("org.mpris.MediaPlayer2");
static const QString mprisInterface_player = QStringLiteral("org.mpris.MediaPlayer2.Player");
static const QString mprisInterface_playlists = QStringLiteral("org.mpris.MediaPlayer2.Playlists");
static const QString mprisInterface_trackList = QStringLiteral("org.mpris.MediaPlayer2.TrackList");
static const QString mprisInterface_properties = QStringLiteral("org.freedesktop.DBus.Properties");

/* ************************************************************************** */

MprisController *MprisController::instance = nullptr;

MprisController *MprisController::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MprisController();
    }

    return instance;
}

MprisController::MprisController()
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        qWarning() << "DBus is not connected?";
    }

    select_player();
}

MprisController::~MprisController()
{
    //
}

/* ************************************************************************** */

bool MprisController::select_player()
{
    QString player_was = m_player_selected;

    m_player_registered.clear();
    m_player_selected.clear();

    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    if (reply.isValid())
    {
        const QStringList values = reply.value();
        for (const QString &name : values)
        {
            if (name.contains("mpris"))
            {
                //qDebug() << "MPRIS player detected:" << name;

                m_player_registered.push_back(name);

                if (m_player_selected.isEmpty())
                {
                    QDBusInterface remoteApp(name, mprisObjectPath, mprisInterface_root);
                    m_playerName = remoteApp.property("Identity").toString();

                    QDBusInterface remoteAppPlayer(name, mprisObjectPath, mprisInterface_player);

                    m_canControl = remoteAppPlayer.property("CanControl").toBool();
                    m_canPlayPause = remoteAppPlayer.property("CanPlayPause").toBool();
                    m_canSeek = remoteAppPlayer.property("CanSeek").toBool();
                    m_canGoPrevious = remoteAppPlayer.property("CanGoPrevious").toBool();
                    m_canGoNext = remoteAppPlayer.property("CanGoNext").toBool();

                    m_playbackStatus = remoteAppPlayer.property("PlaybackStatus").toString();
                    Q_EMIT statusUpdated();

                    if (m_playbackStatus == "Playing")
                    {
                        //if (m_player_selected != name) Q_EMIT playerUpdated();

                        m_player_selected = name;
                        //qDebug() << "MPRIS player SELECTED:" << m_playerName << m_player_selected
                        //         << " (" << remoteAppPlayer.property("PlaybackStatus") << ")";
                    }
                    if (m_playbackStatus == "Playing" || m_playbackStatus == "Paused")
                    {
                        getMetadata();

                        m_volume = remoteAppPlayer.property("Volume").toReal();
                        //Q_EMIT volumeUpdated();

                        m_position = remoteAppPlayer.property("Position").toInt() / 60;
                        //Q_EMIT positionUpdated();

                        //qDebug() << "pos: " << m_position << " / " << m_metaDuration;
                    }
                }
            }
        }

        if (m_player_selected.isEmpty() && !m_player_registered.isEmpty())
        {
            // if we have player(s) but none is currently playing, then use the first one we have
            m_player_selected = m_player_registered.first();
        }

        if (m_player_selected != player_was)
        {
            Q_EMIT playerUpdated();
        }
    }
    else
    {
        qWarning() << "Error:" << reply.error().message();
    }

    return !m_player_selected.isEmpty();
}

/* ************************************************************************** */

void MprisController::setPosition(int64_t pos)
{
    //
}

void MprisController::setVolume(float vol)
{
    //
}

/* ************************************************************************** */

void MprisController::action(unsigned action_code)
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (action_code == LocalActions::ACTION_MPRIS_playpause)
    {
        remoteApp.call("PlayPause");
    }
    else if (action_code == LocalActions::ACTION_MPRIS_stop)
    {
        remoteApp.call("Stop");
    }
    else if (action_code == LocalActions::ACTION_MPRIS_prev)
    {
        remoteApp.call("Previous");
    }
    else if (action_code == LocalActions::ACTION_MPRIS_next)
    {
        remoteApp.call("Next");
    }
}

/* ************************************************************************** */

void MprisController::media_playpause()
{
    select_player();

    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);
    remoteApp.call("PlayPause");
}
void MprisController::media_stop()
{
    select_player();

    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);
    remoteApp.call("Stop");
}
void MprisController::media_prev()
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);
    remoteApp.call("Previous");
}
void MprisController::media_next()
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);
    remoteApp.call("Next");
}

/* ************************************************************************** */

void MprisController::getMetadata()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    QDBusMessage request = QDBusMessage::createMethodCall(m_player_selected,
                                                          mprisObjectPath,
                                                          mprisInterface_properties,
                                                          QString("Get"));

    QVariantList args = QVariantList() << QString("org.mpris.MediaPlayer2.Player")
                                       << QString("Metadata");
    request.setArguments(args);

    QDBusMessage response = bus.call(request, QDBus::BlockWithGui);
    if ((response.type() != QDBusMessage::ReplyMessage) || (response.arguments().isEmpty()))
    {
        qDebug() << "Error message" << response.errorMessage();

        m_metaTitle.clear();
        m_metaArtist.clear();
        m_metaAlbum.clear();
        m_metaThumbnail.clear();
        m_metaPosition = 0;
        m_metaDuration = 0;
    }
    else
    {
        QVariantHash map = qdbus_cast<QVariantHash>(response.arguments()
                                                    .first()
                                                    .value<QDBusVariant>()
                                                    .variant()
                                                    .value<QDBusArgument>());

        m_metaTitle = map.value(QString("xesam:title"), QString("unknown")).toString();
        m_metaArtist =  map.value(QString("xesam:artist"), QString("unknown")).toString();
        m_metaAlbum = map.value(QString("xesam:album"), QString("unknown")).toString();
        m_metaThumbnail = map.value(QString("mpris:artUrl"), QString("")).toString();

        if (m_player_selected == "org.mpris.MediaPlayer2.vlc")
        {
            m_metaDuration =  map.value(QString("vlc:time"), 0).toInt();
        }
        else
        {
            m_metaDuration = map.value(QString("mpris:length"), 0).toInt() / (100000000);
        }
    }

    Q_EMIT metadataUpdated();
}

/* ************************************************************************** */
