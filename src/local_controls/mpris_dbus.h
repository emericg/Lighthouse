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

#ifndef MPRIS_DBUS_H
#define MPRIS_DBUS_H
/* ************************************************************************** */

#include <QObject>
#include <QString>

/* ************************************************************************** */

/*!
 * MPRIS interface
 */
class MprisController: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool available READ isMprisAvailable NOTIFY playerUpdated)

    Q_PROPERTY(bool canControl READ canControl NOTIFY playerUpdated)
    Q_PROPERTY(bool canPlayPause READ canPlayPause NOTIFY playerUpdated)
    Q_PROPERTY(bool canSeek READ canSeek NOTIFY playerUpdated)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY playerUpdated)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY playerUpdated)

    Q_PROPERTY(QString playerName READ getPlayerName NOTIFY playerUpdated)
    Q_PROPERTY(QString playbackStatus READ getPlaybackStatus NOTIFY statusUpdated)
    Q_PROPERTY(qint64 position READ getPosition WRITE setPosition NOTIFY statusUpdated)
    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeUpdated)

    Q_PROPERTY(QString metaTitle READ getTitle NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaArtist READ getArtist NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaAlbum READ getAlbum NOTIFY metadataUpdated)
    Q_PROPERTY(QString metaThumbnail READ getThumbnail NOTIFY metadataUpdated)
    Q_PROPERTY(qint64 metaPosition READ getMetaPosition NOTIFY metadataUpdated)
    Q_PROPERTY(qint64 metaDuration READ getMetaDuration NOTIFY metadataUpdated)

    QStringList m_player_registered;
    QString m_player_selected;

    bool isMprisAvailable() const { return !m_player_selected.isEmpty(); }

    bool m_canControl = false;
    bool m_canPlayPause = false;
    bool m_canSeek = false;
    bool m_canGoPrevious = false;
    bool m_canGoNext = false;

    QString m_playerName;
    QString m_playbackStatus;
    int64_t m_position;
    double m_volume;

    QString m_metadata; // raw

    QString m_metaTitle;
    QString m_metaArtist;
    QString m_metaAlbum;
    QString m_metaThumbnail;
    int64_t m_metaPosition;
    int64_t m_metaDuration;

    void getMetadata();

    // Singleton
    static MprisController *instance;
    MprisController();
    ~MprisController();

signals:
    void playerUpdated();
    void statusUpdated();
    void volumeUpdated();
    void positionUpdated();
    void metadataUpdated();
/*
private slots:
    void playbackStatusChanged();
    void metadataChanged();
    void positionChanged();
    void volumeChanged();
*/
public:
    static MprisController *getInstance();

    bool canControl() const { return m_canControl; }
    bool canPlayPause() const { return m_canPlayPause; }
    bool canSeek() const { return m_canSeek; }
    bool canGoPrevious() const { return m_canGoPrevious; }
    bool canGoNext() const { return m_canGoNext; }

    QString getPlayerName() const { return m_playerName; }
    QString getPlaybackStatus() const  { return m_playbackStatus; }

    qint64 getPosition() const { return m_position; }
    void setPosition(int64_t pos);
    float getVolume() const { return m_volume; }
    void setVolume(float vol);

    QString getTitle() const { return m_metaTitle; }
    QString getArtist() const { return m_metaArtist; }
    QString getAlbum() const { return m_metaAlbum; }
    QString getThumbnail() const { return m_metaThumbnail; }
    qint64 getMetaPosition() const { return m_metaPosition; }
    qint64 getMetaDuration() const { return m_metaDuration; }

    Q_INVOKABLE void action(unsigned action_code);

    Q_INVOKABLE bool select_player();

    // shortcut
    Q_INVOKABLE void media_playpause();
    Q_INVOKABLE void media_stop();
    Q_INVOKABLE void media_next();
    Q_INVOKABLE void media_prev();
};

/* ************************************************************************** */
#endif // MPRIS_DBUS_H
