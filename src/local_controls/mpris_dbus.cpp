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
static const QString freedesktopInterface_properties = QStringLiteral("org.freedesktop.DBus.Properties");

/* ************************************************************************** */

Media_MPRIS *Media_MPRIS::instance = nullptr;

Media_MPRIS *Media_MPRIS::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Media_MPRIS();
    }

    return instance;
}

Media_MPRIS::Media_MPRIS()
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        qWarning() << "DBus is not connected?";
    }

    select_player();
}

Media_MPRIS::~Media_MPRIS()
{
    //
}

/* ************************************************************************** */

bool Media_MPRIS::select_player()
{
    QString player_was = m_player_selected;
    QString player_selected;
    QStringList player_registered;

    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    if (reply.isValid())
    {
        const QStringList values = reply.value();
        for (const QString &player_name : values)
        {
            if (player_name.contains("mpris"))
            {
                //qDebug() << "> MPRIS player detected:" << player_name;
                player_registered.push_back(player_name);

                if (player_selected.isEmpty())
                {
                    QDBusInterface remoteAppPlayer(player_name, mprisObjectPath, mprisInterface_player);
                    QString playbackStatus = remoteAppPlayer.property("PlaybackStatus").toString();
                    if (playbackStatus == "Playing")
                    {
                        //qDebug() << "> MPRIS player SELECTED:" << player_name << "is playing";
                        player_selected = player_name;
                    }
                }
            }
        }

        // if we have player(s) but none is currently playing
        if (player_selected.isEmpty() && !player_registered.isEmpty())
        {
            if (player_registered.contains(player_was))
            {
                // if the current one is still in the list, keep it
                player_selected = player_was;
            }
            else
            {
                // otherwise, use the first one we have in the list
                player_selected = player_registered.first();
            }
        }
    }
    else
    {
        qWarning() << "Error:" << reply.error().message();
    }

    if (m_player_selected != player_selected)
    {
        m_player_selected = player_selected;
        Q_EMIT playerUpdated();
    }

    if (m_player_selected != player_was)
    {
        QDBusInterface remoteApp(m_player_selected, mprisObjectPath, mprisInterface_root);
        //bool CanQuit = remoteApp.property("CanQuit").toBool();
        //bool Fullscreen = remoteApp.property("Fullscreen").toBool(); // R/W
        //bool CanSetFullscreen = remoteApp.property("CanSetFullscreen").toBool();
        //bool CanRaise = remoteApp.property("CanRaise").toBool();
        //bool HasTrackList = remoteApp.property("HasTrackList").toBool();
        m_playerName = remoteApp.property("Identity").toString();
        //QString DesktopEntry = remoteApp.property("DesktopEntry").toString();

        QDBusInterface remoteAppPlayer(m_player_selected, mprisObjectPath, mprisInterface_player);
        m_canControl = remoteAppPlayer.property("CanControl").toBool();
        m_canPlayPause = remoteAppPlayer.property("CanPlayPause").toBool();
        m_canGoPrevious = remoteAppPlayer.property("CanGoPrevious").toBool();
        m_canGoNext = remoteAppPlayer.property("CanGoNext").toBool();
        m_canSeek = remoteAppPlayer.property("CanSeek").toBool();

        double volume = remoteAppPlayer.property("Volume").toDouble(); // R/W
        QString loopStatus = remoteAppPlayer.property("Loop_Status").toString(); // R/W
        double rate = remoteAppPlayer.property("Rate").toDouble(); // R/W
        bool shuffle = remoteAppPlayer.property("Shuffle").toBool(); // R/W
        int64_t position = remoteAppPlayer.property("Position").toInt();
        QString playbackStatus = remoteAppPlayer.property("PlaybackStatus").toString();

        getMetadata();

        setVolume(volume);
        setRate(rate);
        setPosition_us(position);
        setPlaybackStatus(playbackStatus);

        //qDebug() << "status: " << playbackStatus;
        //qDebug() << "volume: " << volume;
        //qDebug() << "loop  : " << loopStatus;
        //qDebug() << "shuffl: " << shuffle;
        //qDebug() << "rate  : " << rate;
        //qDebug() << "pos µs: " << position << " / " << m_metaDuration;

        // We don't use that (yet)
        //QDBusInterface remoteAppPlaylists(name, mprisObjectPath, mprisInterface_playlists);
        //QDBusInterface remoteAppTracklists(name, mprisObjectPath, mprisInterface_trackList);

        // Subscribe to notifications
        QDBusConnection::sessionBus().connect(
            m_player_selected,
            mprisObjectPath,
            freedesktopInterface_properties,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
        );
    }

    return !m_player_selected.isEmpty();
}

/* ************************************************************************** */

void Media_MPRIS::onPropertiesChanged(const QString &interfaceName,
                                      const QVariantMap &changedProps,
                                      const QStringList &invalidatedProps)
{
    //qDebug() << "Media_MPRIS::onPropertiesChanged()" << changedProps;
    Q_UNUSED(invalidatedProps);

    if (interfaceName != "org.mpris.MediaPlayer2.Player")
    {
        qWarning() << "Wrong MPRIS interface ?" << interfaceName;
        return;
    }

    if (changedProps.contains("PlaybackStatus"))
    {
        setPlaybackStatus(changedProps.value("PlaybackStatus").toString());
    }

    if (changedProps.contains("Metadata"))
    {
        getMetadata();
    }

    if (changedProps.contains("Volume"))
    {
        setVolume(changedProps.value("Volume").toFloat());
    }

    if (changedProps.contains("Rate"))
    {
        setRate(changedProps.value("Rate").toFloat());
    }

    if (changedProps.contains("Position"))
    {
        setPosition(changedProps.value("Position").toLongLong() / 60);
    }
}

/* ************************************************************************** */

void Media_MPRIS::setPlaybackStatus(const QString &status)
{
    if (m_playbackStatus != status)
    {
        m_playbackStatus = status;
        Q_EMIT statusUpdated();
    }
}

void Media_MPRIS::setPosition_us(int64_t pos)
{
    if (m_position_us != pos)
    {
        m_position_us = pos;
        Q_EMIT positionUpdated();
    }
}

void Media_MPRIS::setPosition(float pos)
{
    if (pos >= 0.f && pos <= 100.f)
    {
        if (m_position != pos)
        {
            m_position = pos;
            Q_EMIT positionUpdated();
        }
    }
    else
    {
        qWarning() << "Media_MPRIS::setPosition(" << pos << ") ERROR";
    }
}

void Media_MPRIS::setVolume(float vol)
{
    if (vol >= 0.f && vol <= 100.f)
    {
        if (m_volume != vol)
        {
            m_volume = vol;
            Q_EMIT volumeUpdated();
        }
    }
    else
    {
        qWarning() << "Media_MPRIS::setVolume(" << vol << ") ERROR";
    }
}

void Media_MPRIS::setRate(float rate)
{
    if (rate >= 0.f && rate <= 100.f)
    {
        if (m_rate != rate)
        {
            m_rate = rate;
            Q_EMIT rateUpdated();
        }
    }
    else
    {
        qWarning() << "Media_MPRIS::setRate(" << rate << ") ERROR";
    }
}

/* ************************************************************************** */

void Media_MPRIS::action(unsigned action_code)
{
    select_player();

    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        if (action_code == LocalActions::ACTION_MEDIA_playpause)
        {
            remoteApp.call("PlayPause");
        }
        else if (action_code == LocalActions::ACTION_MEDIA_stop)
        {
            remoteApp.call("Stop");
        }
        else if (action_code == LocalActions::ACTION_MEDIA_prev)
        {
            remoteApp.call("Previous");
        }
        else if (action_code == LocalActions::ACTION_MEDIA_next)
        {
            remoteApp.call("Next");
        }
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}

/* ************************************************************************** */

void Media_MPRIS::media_playpause()
{
    select_player();

    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        remoteApp.call("PlayPause");
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}
void Media_MPRIS::media_stop()
{
    select_player();

    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        remoteApp.call("Stop");
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}
void Media_MPRIS::media_prev()
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        remoteApp.call("Previous");
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}
void Media_MPRIS::media_next()
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        remoteApp.call("Next");
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}
void Media_MPRIS::media_seek(qint64 offset_us)
{
    QDBusInterface remoteApp(m_player_selected,
                             mprisObjectPath,
                             mprisInterface_player);

    if (remoteApp.isValid())
    {
        remoteApp.call("Seek", offset_us);
    }
    else
    {
        qCritical() << "Cannot connect to" << m_player_selected << "MPRIS D-Bus interface!";
    }
}

/* ************************************************************************** */

void Media_MPRIS::getMetadata()
{
    if (m_player_selected.isEmpty())
    {
        qDebug() << "getMetadata() no player selected";
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();

    QDBusMessage request = QDBusMessage::createMethodCall(m_player_selected,
                                                          mprisObjectPath,
                                                          freedesktopInterface_properties,
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
                                                    .constFirst()
                                                    .value<QDBusVariant>()
                                                    .variant()
                                                    .value<QDBusArgument>());

        m_metaTitle = map.value(QString("xesam:title"), QString("unknown")).toString();
        m_metaArtist =  map.value(QString("xesam:artist"), QString("unknown")).toString();
        m_metaAlbum = map.value(QString("xesam:album"), QString("unknown")).toString();
        m_metaThumbnail = map.value(QString("mpris:artUrl"), QString("")).toString();

        if (map.contains("mpris:length")) m_metaDuration = map.value(QString("mpris:length"), 0).toInt();
        else if (map.contains("vlc:time")) m_metaDuration = map.value(QString("vlc:time"), 0).toInt();
        else m_metaDuration = 0;
    }

    Q_EMIT metadataUpdated();
}

/* ************************************************************************** */
