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
 * \date      2020
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef DEVICE_POKEMONGOPLUS_H
#define DEVICE_POKEMONGOPLUS_H
/* ************************************************************************** */

#include "../device_beacon.h"
#include "../crypto/pgp/pgp_keystore.h"

#include <QObject>
#include <QList>
#include <QUrl>

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QBluetoothLocalDevice>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Nintendo "Pokemon Go Plus" (PGP)
 * Advertising name is 'Pokemon GO Plus'
 *
 * Protocol infos:
 * - docs/porygon.md (reconstructed from https://github.com/fbrzlarosa/porygon)
 * - https://tinyhack.com/2018/11/21/reverse-engineering-pokemon-go-plus/
 * - https://www.reddit.com/r/pokemongodev/comments/7cba9t/controlling_the_pokemon_go_plus_led_and_button/
 */
class DevicePokemonGoPlus: public DeviceBeacon
{
    Q_OBJECT

    Q_PROPERTY(bool autoConnect READ getAutoConnect WRITE setAutoConnect NOTIFY autoconnectChanged)
    Q_PROPERTY(bool hasDeviceKey READ hasDeviceKey NOTIFY deviceKeyChanged)

    bool m_autoConnect = true;
    bool m_hasDeviceKey = false;

    ////////

    static inline const QBluetoothUuid s_srvBattery {QStringLiteral("0000180f-0000-1000-8000-00805f9b34fb")};
    static inline const QBluetoothUuid s_chrBatteryLevel {QStringLiteral("00002a19-0000-1000-8000-00805f9b34fb")};

    static inline const QBluetoothUuid s_srvCertificate {QStringLiteral("bbe87709-5b89-4433-ab7f-8b8eef0d8e37")};
    static inline const QBluetoothUuid s_chrSfidaCommands {QStringLiteral("bbe87709-5b89-4433-ab7f-8b8eef0d8e39")};
    static inline const QBluetoothUuid s_chrCentralToSfida {QStringLiteral("bbe87709-5b89-4433-ab7f-8b8eef0d8e38")};
    static inline const QBluetoothUuid s_chrSfidaToCentral {QStringLiteral("bbe87709-5b89-4433-ab7f-8b8eef0d8e3a")};

    static inline const QBluetoothUuid s_srvControl {QStringLiteral("21c50462-67cb-63a3-5c4c-82b5b9939aeb")};
    static inline const QBluetoothUuid s_chrLedVibrate {QStringLiteral("21c50462-67cb-63a3-5c4c-82b5b9939aec")};
    static inline const QBluetoothUuid s_chrButtonNotif {QStringLiteral("21c50462-67cb-63a3-5c4c-82b5b9939aed")};

    ////////

    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_certificate(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_control(QLowEnergyService::ServiceState newState);
    void discoverControlService();

    QLowEnergyService *m_serviceBattery = nullptr;
    QLowEnergyService *m_serviceCertificate = nullptr;
    QLowEnergyDescriptor m_sifdaNotifDesc;
    QLowEnergyService *m_serviceControl = nullptr;
    QLowEnergyDescriptor m_buttonNotifDesc;

    void bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void bleServiceErrored(QLowEnergyService::ServiceError error);

    ////////

    /*!
     * Certification handshake.
     *
     * The device hangs up on connections that do not certify, so this runs first and
     * everything else waits for it. The steps are driven by SFIDA_COMMANDS notifications
     * carrying a 4 bytes state, with the payload of each step read from SFIDA_TO_CENTRAL
     * and our answers written to CENTRAL_TO_SFIDA.
     *
     * States, from the device's point of view:
     * - 0: device offers the 378 bytes challenge, we answer with the decrypted challenge
     * - 1: device sends a 52 bytes challenge, we verify it and send our own
     * - 2: device acknowledges, we send the final challenge
     * - 3: device sends 4 00 23 00, certified
     */
    enum CertState {
        CERT_IDLE = 0,          //!< nothing started yet
        CERT_CHALLENGE_SENT,    //!< we answered the initial challenge
        CERT_EXCHANGE,          //!< mutual verification
        CERT_FINAL,             //!< final challenge sent
        CERT_DONE,              //!< certified, the link is ready
        CERT_FAILED,            //!< failed...
    };

    CertState m_certState = CERT_IDLE;
    PgpKeys m_keys;
    QByteArray m_sessionKey;    //!< 16 bytes, valid once the initial challenge is decrypted
    QByteArray m_appChallenge;  //!< 16 bytes we sent at step 2, the device echoes them back decrypted

    void certificationStart();
    void certificationNotify(const QByteArray &state);
    void certificationData(const QByteArray &data);
    void certificationFailed(const QString &why);
    void certificationDone();

    void writeCentralToSfida(const QByteArray &payload);
    void readSfidaToCentral();
    void subscribeToSfidaCommands();

    // The certificate service needs an encrypted link, and Qt's BLE controller never initiates bonding on its own
    QBluetoothLocalDevice *m_localDevice = nullptr;
    bool ensurePaired();
    void pairingDone(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

Q_SIGNALS:
    void autoconnectChanged();
    void btnChanged();
    void deviceKeyChanged();

protected:
    int getButtonCount() { return 1; }
    bool hasPress() { return true; }
    bool hasDoublePress() { return false; }
    bool hasLongPress() { return false; }

public:
    DevicePokemonGoPlus(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DevicePokemonGoPlus(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DevicePokemonGoPlus();

    bool getAutoConnect() { return m_autoConnect; }
    void setAutoConnect(const bool value);

    //! Whether a key dump for this device has already been installed
    bool hasDeviceKey() const { return m_hasDeviceKey; }

    //! Installs the key dump the user picked, returns false if it isn't usable for this device
    Q_INVOKABLE bool setDeviceKeyFile(const QUrl &fileUrl);

    //! Where the key dumps live, so the UI can point the user at it
    Q_INVOKABLE QString getDeviceKeyDirectory() const { return pgp_keys_directory(); }

    void setNotification(const QByteArray &value);
};

/* ************************************************************************** */
#endif // DEVICE_POKEMONGOPLUS_H
