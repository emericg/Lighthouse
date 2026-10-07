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
 * \date      2026
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifdef ENABLE_MEDIA_MEDIAREMOTE
#ifndef MEDIA_MEDIAREMOTE_H
#define MEDIA_MEDIAREMOTE_H
/* ************************************************************************** */

#include "media.h"

#include <QObject>
#include <QString>
#include <QDateTime>

class QTimer;

/* ************************************************************************** */

/*!
 * Media controller (macOS / MediaRemote)
 *
 * MediaRemote is the private framework behind the "Now Playing" menu bar widget.
 * It follows whatever application macOS considers to be playing (Music, Spotify,
 * a browser tab...), and changes are pushed through notifications.
 *
 * \note Starting with macOS 15.4, only Apple signed processes can read the now
 *       playing information, so this backend reports itself unavailable there.
 */
class Media_mediaremote: public Media
{
    Q_OBJECT

    bool m_loaded = false;          //!< MediaRemote is loaded and usable on this macOS version
    void *m_observers = nullptr;    //!< NSMutableArray of notification observers

    int m_pid = 0;                  //!< now playing application, 0 if none
    bool m_playing = false;

    // position is reported as "elapsed time at a timestamp", playing at a given rate
    double m_elapsed_s = 0.0;
    QDateTime m_elapsedTimestamp;

    QString m_trackId;              //!< identity of the current track, to tell track changes apart
    QString m_artworkId;            //!< to avoid re-encoding the same artwork

    // MediaRemote does not push position updates, so we extrapolate it while playing
    QTimer *m_positionTimer = nullptr;

    void load();

    void refreshApplication();      //!< now playing application (and everything else)
    void refreshPlaying();
    void refreshInfo();

    void applyInfo(const void *info);   //!< NSDictionary of kMRMediaRemoteNowPlayingInfo* keys
    void setPlaying(bool playing);
    void updatePosition();

    bool sendCommand(int command);

    // Singleton
    static Media_mediaremote *instance;
    Media_mediaremote();
    ~Media_mediaremote();

public:
    static Media_mediaremote *getInstance();

    bool isAvailable() const override { return m_pid > 0; }

    bool select_player() override;
    void action(unsigned action_code) override;

    void media_playpause() override;
    void media_stop() override;
    void media_next() override;
    void media_prev() override;
    void media_seek(qint64 offset_us) override;
};

/* ************************************************************************** */
#endif // MEDIA_MEDIAREMOTE_H
#endif // ENABLE_MEDIA_MEDIAREMOTE
