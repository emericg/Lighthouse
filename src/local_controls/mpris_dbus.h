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

#ifdef ENABLE_MEDIA_MPRIS
#ifndef MPRIS_DBUS_H
#define MPRIS_DBUS_H
/* ************************************************************************** */

#include "media.h"

#include <QObject>
#include <QString>

class QTimer;
class QDBusMessage;
class QDBusServiceWatcher;

/* ************************************************************************** */

/*!
 * Media controller (Linux / MPRIS, over D-Bus)
 */
class Media_MPRIS: public Media
{
    Q_OBJECT

    Q_PROPERTY(float position READ getPosition WRITE setPosition NOTIFY positionUpdated)
    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeUpdated)
    Q_PROPERTY(qint64 metaPosition READ getMetaPosition NOTIFY metadataUpdated)

    //QStringList m_player_registered;
    QString m_player_selected;  //!< well-known bus name of the selected player
    QString m_player_owner;     //!< unique bus name currently owning m_player_selected

    bool m_canControlRate = false;
    bool m_canControlVolume = false;

    double m_position = -1.f; // in %
    double m_volume = -1.f; // in %

    QString m_metadata; // raw

    int64_t m_metaPosition = 0;

    void getMetadata();

    // MPRIS players don't push Position updates, so we poll it while playing
    QTimer *m_positionTimer = nullptr;

    // notifies us when MPRIS players appear, disappear, or change owner
    QDBusServiceWatcher *m_serviceWatcher = nullptr;

    /*!
     * \brief Select the player to follow, and refresh our state if it changed.
     * \param preferredOwner: unique bus name of a player to select in priority (if registered).
     * \return true if a player is selected.
     *
     * Without a preferred owner, the first playing player is selected,
     * otherwise the current one is kept, otherwise the first registered one is used.
     */
    bool selectPlayer(const QString &preferredOwner);

    // Singleton
    static Media_MPRIS *instance;
    Media_MPRIS();
    ~Media_MPRIS();

signals:
    void volumeUpdated();

private slots:
    /*!
     * \brief Handle PropertiesChanged signals coming from any MPRIS player.
     * \param msg: the D-Bus signal message, used to identify the emitting player.
     *
     * Changes from the selected player update our state.
     * Another player starting playback, or the selected one stopping, triggers a new selection.
     */
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProps,
                             const QStringList &invalidatedProps,
                             const QDBusMessage &msg);

    void refreshPosition();     //!< poll the player's Position over D-Bus (it is never pushed)

public:
    static Media_MPRIS *getInstance();

    bool isAvailable() const override { return !m_player_selected.isEmpty(); }

    void setPlaybackStatus(const QString &status);

    void setPosition_us(int64_t pos);

    float getPosition() const { return m_position; }
    void setPosition(float pos);

    float getVolume() const { return m_volume; }
    void setVolume(float vol);

    void setRate(float vol);

    qint64 getMetaPosition() const { return m_metaPosition; }

    void action(unsigned action_code) override;

    bool select_player() override;

    // shortcut
    void media_playpause() override;
    void media_stop() override;
    void media_next() override;
    void media_prev() override;
    void media_seek(qint64 offset_us) override;
};

/* ************************************************************************** */
#endif // MPRIS_DBUS_H
#endif // ENABLE_MEDIA_MPRIS
