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

#ifndef DEVICE_LOCAL_CONTROLS_H
#define DEVICE_LOCAL_CONTROLS_H
/* ************************************************************************** */

#include "mouse.h"
#include "keyboard.h"
#include "gamepad.h"
#include "os_commander.h"

#include "media.h"
#include "volume.h"

#include <QObject>
#include <QString>

class QMediaPlayer;
class QAudioOutput;

/* ************************************************************************** */

/*!
 * LocalControls is the action dispatcher.
 * It holds virtual mouse, keyboard and gamepad to perform actions.
 * It also follows the media player currently in use, through the platform media backend.
 */
class LocalControls: public QObject
{
    Q_OBJECT

    Mouse *mouse = nullptr;
    Keyboard *keyboard = nullptr;
    Gamepad *gamepad = nullptr;
    OsCommander *oscommander = nullptr;

    Media *media = nullptr;         //!< media player backend: state, metadata and playback commands
    Volume *volume = nullptr;       //!< audio-server backend: absolute level + state + cap
    Volume *volume_keys = nullptr;  //!< media-key backend: relative up/down/mute with desktop OSD

    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;

    // Singleton
    static LocalControls *instance;
    LocalControls();
    ~LocalControls();

public:
    static LocalControls *getInstance();

    Media *getMediaController() const { return media; }
    Volume *getVolumeController() const { return volume; }

    Q_INVOKABLE void action(int action_code, const QString &action_params = QString());

    // volume (routed to the active Volume backend, when available)
    Q_INVOKABLE void volume_set(float level);
    Q_INVOKABLE void volume_up();
    Q_INVOKABLE void volume_down();
    Q_INVOKABLE void volume_mute();
    Q_INVOKABLE void volume_unmute();
    Q_INVOKABLE void volume_toggle_mute();
    Q_INVOKABLE float getVolumeLevel() const;
    Q_INVOKABLE bool isMuted() const;

    // media (routed from the active media backend, when available)
    int getMediaPlayerId() const { return 0; }  //!< reserved for future multi-player support
    QString getMediaPlayerName() const;
    QString getMediaStatus() const;             //!< "Playing", "Paused", "Stopped", or empty
    QString getMediaTitle() const;
    QString getMediaArtist() const;
    QString getMediaAlbum() const;
    QString getMediaArtUrl() const;             //!< may be a file://, http(s):// or data: URL
    qint64 getMediaPosition_us() const;
    qint64 getMediaDuration_us() const;
    float getMediaRate() const;

    /*!
     * \brief Type text through the virtual keyboard, one Unicode code point at a time.
     */
    void keyboard_text(QStringView text);

    Q_INVOKABLE void mouse_action(int dx, int dy, int btn_left, int btn_right, int btn_middle);
    void mouse_move(int dx, int dy);
    void mouse_button(int code, bool pressed);
    void mouse_scroll(int dx, int dy);

    Q_INVOKABLE void gamepad_action(float x1, float y1, float x2, float y2,
                                    int a, int b, int x, int y);

    // shortcuts
    Q_INVOKABLE void keyboard_computer_lock();
    Q_INVOKABLE void keyboard_computer_sleep();
    Q_INVOKABLE void keyboard_computer_poweroff();
    Q_INVOKABLE void keyboard_media_playpause();
    Q_INVOKABLE void keyboard_media_stop();
    Q_INVOKABLE void keyboard_media_next();
    Q_INVOKABLE void keyboard_media_prev();
    Q_INVOKABLE void keyboard_volume_up();
    Q_INVOKABLE void keyboard_volume_down();
    Q_INVOKABLE void keyboard_volume_mute();

signals:
    // re-emitted from the active Volume backend, so consumers (ex: NetworkServer)
    // never need to reference the platform-specific Volume backend directly
    void volumeChanged();
    void muteChanged();

    // re-emitted from the active media backend, same rationale as the volume signals
    void mediaChanged();            //!< playback status / position / rate changed (cheap, frequent)
    void mediaMetadataChanged();    //!< title / artist / album / art URL changed
};

/* ************************************************************************** */
#endif // DEVICE_LOCAL_CONTROLS_H
