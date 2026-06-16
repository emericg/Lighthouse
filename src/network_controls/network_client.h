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

/* ************************************************************************** */

class NetworkClient: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool wifi READ isWifiConnected NOTIFY wifiEvent)
    Q_PROPERTY(bool connected READ isClientConnected NOTIFY connectionEvent)

    Q_PROPERTY(float volume READ getVolume NOTIFY volumeStateChanged)
    Q_PROPERTY(bool volumeMuted READ isVolumeMuted NOTIFY volumeStateChanged)

    QTcpSocket *m_tcpSocket = nullptr;
    QDataStream m_dataInput;

    QString m_ssid;
    QString m_host;
    unsigned m_port = 5555;

    bool m_wifi = false;
    bool m_connected = false;

    float m_volume = -1.f;   //!< desktop volume, normalized [0.0 ; 1.0], -1.0 if unknown
    bool m_volumeMuted = false;

    void connected();
    void disconnected();

    void sendCommand(const QString &cmd);

signals:
    void wifiEvent();
    void connectionEvent();
    void volumeStateChanged();

public:
    explicit NetworkClient(QObject *parent = nullptr);

    bool isClientConnected() const { return m_connected; }
    bool isWifiConnected() const { return m_wifi; }

    float getVolume() const { return m_volume; }
    bool isVolumeMuted() const { return m_volumeMuted; }

public slots:
    void connectToServer();
    void disconnectFromServer();

    void displayError(QAbstractSocket::SocketError socketError);

    void readMetadata();

    void sendAction(int action);
    void sendKey(QChar key);
    void sendGamepad(float x1, float y1, float x2, float y2,
                     int a, int b, int x, int y);

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
