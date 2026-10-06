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

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H
/* ************************************************************************** */

#include <QDataStream>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QImage>

class QTimer;

/* ************************************************************************** */

class NetworkClient: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool wifi READ isWifiConnected NOTIFY wifiEvent)
    Q_PROPERTY(bool connected READ isClientConnected NOTIFY connectionEvent)
    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY connectionEvent)

    Q_PROPERTY(float volume READ getVolume NOTIFY volumeStateChanged)
    Q_PROPERTY(bool volumeMuted READ isVolumeMuted NOTIFY volumeStateChanged)

    Q_PROPERTY(QString playerName READ getPlayerName NOTIFY mediaMetadataChanged)
    Q_PROPERTY(QString playbackStatus READ getPlaybackStatus NOTIFY mediaStateChanged)
    Q_PROPERTY(QString metaTitle READ getMetaTitle NOTIFY mediaMetadataChanged)
    Q_PROPERTY(QString metaArtist READ getMetaArtist NOTIFY mediaMetadataChanged)
    Q_PROPERTY(QString metaAlbum READ getMetaAlbum NOTIFY mediaMetadataChanged)
    Q_PROPERTY(QString metaThumbnail READ getMetaThumbnail NOTIFY mediaArtChanged)
    Q_PROPERTY(qint64 position_us READ getPosition_us NOTIFY mediaStateChanged)
    Q_PROPERTY(qint64 metaDuration READ getMetaDuration NOTIFY mediaStateChanged)
    Q_PROPERTY(float position READ getPosition NOTIFY mediaStateChanged)

    QTcpSocket *m_tcpSocket = nullptr;
    QDataStream m_dataInput;

    QString m_ssid;
    QString m_host;
    unsigned m_port = 5555;

    bool m_wifi = false;
    bool m_connected = false;
    bool m_authenticated = false;   //!< secure handshake completed (always true in non-secure mode)

    float m_volume = -1.f;          //!< desktop volume, normalized [0.0 ; 1.0], -1.0 if unknown
    bool m_volumeMuted = false;

    QString m_playerName;
    QString m_playbackStatus;
    QString m_metaTitle;
    QString m_metaArtist;
    QString m_metaAlbum;
    QString m_metaThumbnail;
    QImage m_artImage;
    int m_artSeq = 0;               //!< bumped on each new artwork so QML doesn't serve a cached image

    qint64 m_position_us = -1;
    qint64 m_duration_us = 0;
    float m_rate = 1.f;

    // most MPRIS players never push Position updates, so we advance it locally while playing
    QTimer *m_positionTimer = nullptr;
    QElapsedTimer m_positionClock;  //!< wall-clock elapsed since the last server position sync

    void connected();
    void disconnected();

    void sendCommand(const QString &cmd);

    void parseMediaState(const QString &payload);
    void parseMediaMetadata(const QString &payload);
    void parseMediaArt(const QString &payload);

private slots:
    void tickPosition();            //!< local interpolation of the playback position while playing
    void tickClaudeCountdown();     //!< local countdown of the Claude reset windows

signals:
    void authError();
    void wifiEvent();
    void connectionEvent();

    void volumeStateChanged();
    void mediaStateChanged();
    void mediaMetadataChanged();
    void mediaArtChanged();

public:
    explicit NetworkClient(QObject *parent = nullptr);

    bool isClientConnected() const { return m_connected; }
    bool isWifiConnected() const { return m_wifi; }
    bool isAuthenticated() const { return m_authenticated; }

    float getVolume() const { return m_volume; }
    bool isVolumeMuted() const { return m_volumeMuted; }

    QString getPlayerName() const { return m_playerName; }
    QString getPlaybackStatus() const { return m_playbackStatus; }
    QString getMetaTitle() const { return m_metaTitle; }
    QString getMetaArtist() const { return m_metaArtist; }
    QString getMetaAlbum() const { return m_metaAlbum; }
    QString getMetaThumbnail() const { return m_metaThumbnail; }
    /*!
     * \brief Current media artwork, as served by NetworkArtProvider.
     * \warning GUI thread only: m_artImage is not guarded.
     *          Images using the "image://networkArt/" source must not set `asynchronous: true`,
     *          or the provider would be called from QML's image-loading thread.
     */
    QImage currentArt() const { return m_artImage; }
    qint64 getPosition_us() const { return m_position_us; }
    qint64 getMetaDuration() const { return m_duration_us; }

    float getPosition() const {
        // playback progress in percent [0 ; 100], -1 if unknown
        if (m_duration_us <= 0 || m_position_us < 0) return -1.f;
        return qBound(0.f, float(double(m_position_us) / double(m_duration_us) * 100.0), 100.f);
    }

public slots:
    void connectToServer();
    void disconnectFromServer();

    void displayError(QAbstractSocket::SocketError socketError);

    void readMetadata();

    void sendAction(int action);
    void sendKey(QChar key);
    void sendGamepad(float x1, float y1, float x2, float y2,
                     int a, int b, int x, int y);

    void sendMouseMove(int dx, int dy);
    void sendMouseScroll(int dx, int dy);
    void sendMouseClick(int btn);
    void sendMouseButton(int btn, bool down);

    void key_up();
    void key_down();
    void key_left();
    void key_right();
    void key_enter();
    void key_escape();

    void media_prev();
    void media_playpause();
    void media_stop();
    void media_next();

    void volume_mute();
    void volume_down();
    void volume_up();
    void volume_set(int pct);   //!< set an absolute level, pct in [0 ; 100]
    void volume_get();          //!< request the current desktop volume state
};

/* ************************************************************************** */
#endif // NETWORK_CLIENT_H
