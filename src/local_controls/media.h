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
 * \date      2025
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef MEDIA_H
#define MEDIA_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

/* ************************************************************************** */

/*!
 * Minimal API to create media controllers.
 *
 * A media controller follows one media player at a time (the one playing, if any),
 * exposes its state and metadata, and forwards playback commands to it.
 *
 * The playback status uses the MPRIS vocabulary: "Playing", "Paused", "Stopped",
 * or empty when no player is followed.
 * Times are expressed in microseconds.
 */
class Media: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ isAvailable NOTIFY playerUpdated)

    Q_PROPERTY(bool canControl READ canControl NOTIFY playerUpdated)
    Q_PROPERTY(bool canPlayPause READ canPlayPause NOTIFY playerUpdated)
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY playerUpdated)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY playerUpdated)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY playerUpdated)

    Q_PROPERTY(QString playerName READ getPlayerName NOTIFY playerUpdated)
    Q_PROPERTY(QString playbackStatus READ getPlaybackStatus NOTIFY statusUpdated)
    Q_PROPERTY(qint64 position_us READ getPosition_us NOTIFY positionUpdated)
    Q_PROPERTY(float rate READ getRate NOTIFY rateUpdated)

    Q_PROPERTY(QString metaTitle READ getTitle NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaArtist READ getArtist NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaAlbum READ getAlbum NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaThumbnail READ getThumbnail NOTIFY metadataUpdated)
    Q_PROPERTY(qint64 metaDuration READ getMetaDuration NOTIFY metadataUpdated)

protected:
    bool m_canControl = false;
    bool m_canPlayPause = false;
    bool m_canGoPrevious = false;
    bool m_canGoNext = false;
    bool m_canSeek = false;

    QString m_playerName;
    QString m_playbackStatus;       //!< "Playing", "Paused", "Stopped", or empty
    qint64 m_position_us = -1;      //!< -1 if unknown
    double m_rate = -1.0;           //!< 1.0 is normal speed, -1.0 if unknown

    QString m_metaTitle;
    QString m_metaArtist;
    QString m_metaAlbum;
    QString m_metaThumbnail;        //!< may be a file://, http(s):// or data: URL
    qint64 m_metaDuration = 0;      //!< 0 if unknown

public:
    Media(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~Media() = default;

    //! True while a media player is being followed
    virtual bool isAvailable() const = 0;

    bool canControl() const { return m_canControl; }
    bool canPlayPause() const { return m_canPlayPause; }
    bool canSeek() const { return m_canSeek; }
    bool canGoPrevious() const { return m_canGoPrevious; }
    bool canGoNext() const { return m_canGoNext; }

    QString getPlayerName() const { return m_playerName; }
    QString getPlaybackStatus() const { return m_playbackStatus; }
    qint64 getPosition_us() const { return m_position_us; }
    float getRate() const { return m_rate; }

    QString getTitle() const { return m_metaTitle; }
    QString getArtist() const { return m_metaArtist; }
    QString getAlbum() const { return m_metaAlbum; }
    QString getThumbnail() const { return m_metaThumbnail; }
    qint64 getMetaDuration() const { return m_metaDuration; }

    /*!
     * \brief Look for the media player to follow (the one playing, if any).
     * \return True if a player is followed.
     */
    Q_INVOKABLE virtual bool select_player() = 0;

    //! Execute a LocalActions::ACTION_MEDIA_* action
    Q_INVOKABLE virtual void action(unsigned action_code) = 0;

    Q_INVOKABLE virtual void media_playpause() = 0;
    Q_INVOKABLE virtual void media_stop() = 0;
    Q_INVOKABLE virtual void media_next() = 0;
    Q_INVOKABLE virtual void media_prev() = 0;
    Q_INVOKABLE virtual void media_seek(qint64 offset_us) = 0; //!< relative to the current position

signals:
    void playerUpdated();       //!< followed player, its name or capabilities changed
    void statusUpdated();
    void positionUpdated();
    void rateUpdated();
    void metadataUpdated();
};

/* ************************************************************************** */
#endif // MEDIA_H
