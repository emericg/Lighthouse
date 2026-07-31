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

#include "device_pokemongoplus.h"
#include "../crypto/pgp/pgp_crypto.h"

#include <cstdint>
#include <cstring>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>
#include <QRandomGenerator>

#include <QDebug>

/* ************************************************************************** */

DevicePokemonGoPlus::DevicePokemonGoPlus(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PGP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }

    m_hasDeviceKey = pgp_has_keys(m_deviceAddress);
}

DevicePokemonGoPlus::DevicePokemonGoPlus(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_PGP;
    m_deviceCapabilities += DeviceUtils::DEVICE_BATTERY;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;
    m_deviceCapabilities += DeviceUtils::DEVICE_LED_RGB;

    if (hasSetting("autoConnect"))
    {
        m_autoConnect = (getSetting("autoConnect").toString() == "true");
    }

    m_hasDeviceKey = pgp_has_keys(m_deviceAddress);
}

DevicePokemonGoPlus::~DevicePokemonGoPlus()
{
    delete m_serviceBattery;
    delete m_serviceCertificate;
    delete m_serviceControl;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokemonGoPlus::setAutoConnect(const bool value)
{
    if (m_autoConnect != value)
    {
        m_autoConnect = value;
        Q_EMIT autoconnectChanged();

        setSetting("autoConnect", m_autoConnect);
    }
}

bool DevicePokemonGoPlus::setDeviceKeyFile(const QUrl &fileUrl)
{
    const QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();

    if (!pgp_import_keys(m_deviceAddress, path)) return false;

    m_hasDeviceKey = true;
    Q_EMIT deviceKeyChanged();

    return true;
}

void DevicePokemonGoPlus::setNotification(const QByteArray &value)
{
    if (m_serviceControl)
    {
        // Set LED color and/or vibration
        //QLowEnergyCharacteristic ll = m_serviceControl->characteristic(s_chrLedVibrate);
        //m_serviceControl->writeCharacteristic(ll, QByteArray::fromHex("06000012"));
    }
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokemonGoPlus::serviceScanDone()
{
    qDebug() << "DevicePokemonGoPlus::serviceScanDone(" << m_deviceAddress << ")";

    if (m_serviceCertificate)
    {
        connect(m_serviceCertificate, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_certificate);
        connect(m_serviceCertificate, &QLowEnergyService::characteristicWritten, this, &DevicePokemonGoPlus::bleWriteDone);
        connect(m_serviceCertificate, &QLowEnergyService::characteristicRead, this, &DevicePokemonGoPlus::bleReadDone);
        connect(m_serviceCertificate, &QLowEnergyService::characteristicChanged, this, &DevicePokemonGoPlus::bleReadNotify);
        connect(m_serviceCertificate, &QLowEnergyService::errorOccurred, this, &DevicePokemonGoPlus::bleServiceErrored);

        // Windows hack, see: QTBUG-80770 and QTBUG-78488
        QTimer::singleShot(0, this, [this] () { m_serviceCertificate->discoverDetails(QLowEnergyService::SkipValueDiscovery); });
    }

    if (m_serviceBattery)
    {
        connect(m_serviceBattery, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_battery);
        connect(m_serviceBattery, &QLowEnergyService::errorOccurred, this, &DevicePokemonGoPlus::bleServiceErrored);

        // Windows hack, see: QTBUG-80770 and QTBUG-78488
        //QTimer::singleShot(0, this, [this] () { m_serviceBattery->discoverDetails(); });
    }

    if (m_serviceControl)
    {
        connect(m_serviceControl, &QLowEnergyService::stateChanged, this, &DevicePokemonGoPlus::serviceDetailsDiscovered_control);
        connect(m_serviceControl, &QLowEnergyService::characteristicWritten, this, &DevicePokemonGoPlus::bleWriteDone);
        connect(m_serviceControl, &QLowEnergyService::characteristicRead, this, &DevicePokemonGoPlus::bleReadDone);
        connect(m_serviceControl, &QLowEnergyService::characteristicChanged, this, &DevicePokemonGoPlus::bleReadNotify);
        connect(m_serviceControl, &QLowEnergyService::errorOccurred, this, &DevicePokemonGoPlus::bleServiceErrored);

        // Windows hack, see: QTBUG-80770 and QTBUG-78488
        //QTimer::singleShot(0, this, [this] () { m_serviceControl->discoverDetails(); });
    }
}

void DevicePokemonGoPlus::addLowEnergyService(const QBluetoothUuid &uuid)
{
    qDebug() << "DevicePokemonGoPlus::addLowEnergyService(" << uuid.toString() << ")";

    if (uuid == s_srvBattery)
    {
        delete m_serviceBattery;
        m_serviceBattery = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            m_serviceBattery = m_bleController->createServiceObject(uuid);
            if (!m_serviceBattery)
                qWarning() << "Cannot create service (battery) for uuid:" << uuid.toString();
        }
    }

    if (uuid == s_srvCertificate)
    {
        delete m_serviceCertificate;
        m_serviceCertificate = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            m_serviceCertificate = m_bleController->createServiceObject(uuid);
            if (!m_serviceCertificate)
                qWarning() << "Cannot create service (certificate) for uuid:" << uuid.toString();
        }
    }

    if (uuid == s_srvControl)
    {
        delete m_serviceControl;
        m_serviceControl = nullptr;

        //if (m_ble_action == ACTION_UPDATE)
        {
            m_serviceControl = m_bleController->createServiceObject(uuid);
            if (!m_serviceControl)
                qWarning() << "Cannot create service (control) for uuid:" << uuid.toString();
        }
    }
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        QLowEnergyCharacteristic cbat = m_serviceBattery->characteristic(s_chrBatteryLevel);
        if (cbat.value().size() > 0)
        {
            int lvl = static_cast<uint8_t>(cbat.value().constData()[0]);
            setBattery(lvl);
        }

        discoverControlService();
    }
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_certificate(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_certificate(" << m_deviceAddress << ") > ServiceDiscovered";

        // The certificate service needs an encrypted link.
        // Without a bond the descriptor write below is refused with "Insufficient Authorization" and the handshake never starts
        if (!ensurePaired())
        {
            qWarning() << "DevicePokemonGoPlus device is not paired!";
            return;
        }

        // Subscribe to SFIDA_COMMANDS, the device drives the handshake through it
        subscribeToSfidaCommands();
    }
}

/*!
 * Subscribing to SFIDA_COMMANDS is what makes the device offer its first challenge.
 */
void DevicePokemonGoPlus::subscribeToSfidaCommands()
{
    if (!m_serviceCertificate) return;

    QLowEnergyCharacteristic csfida = m_serviceCertificate->characteristic(s_chrSfidaCommands);
    m_sifdaNotifDesc = csfida.clientCharacteristicConfiguration();
    m_serviceCertificate->writeDescriptor(m_sifdaNotifDesc, QByteArray::fromHex("0100"));

    certificationStart();
}

/*!
 * Checks the bond, and asks for one if it is missing.
 *
 * Qt's BLE controller never initiates bonding by itself, so without this the device is
 * simply unusable until the user pairs it by hand with an external tool.
 *
 * \return true if we are already paired and the caller can carry on, false if pairing
 * had to be requested, in which case we resume from pairingDone().
 */
bool DevicePokemonGoPlus::ensurePaired()
{
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    // Devices are identified by UUID rather than address, and pairing is handled by the OS
    return true;
#else
    if (!m_localDevice)
    {
        m_localDevice = new QBluetoothLocalDevice(this);
        connect(m_localDevice, &QBluetoothLocalDevice::pairingFinished,
                this, &DevicePokemonGoPlus::pairingDone);
    }

    if (!m_localDevice->isValid()) return true; // no adapter to ask, just try anyway

    const QBluetoothAddress addr(m_deviceAddress);
    if (m_localDevice->pairingStatus(addr) != QBluetoothLocalDevice::Unpaired) return true;

    qWarning() << "PGP >>> device is not paired, requesting pairing";
    m_localDevice->requestPairing(addr, QBluetoothLocalDevice::Paired);

    return false;
#endif
}

void DevicePokemonGoPlus::pairingDone(const QBluetoothAddress &address,
                                      QBluetoothLocalDevice::Pairing pairing)
{
    if (address != QBluetoothAddress(m_deviceAddress)) return;

    if (pairing == QBluetoothLocalDevice::Unpaired)
    {
        certificationFailed("pairing was refused, the device cannot be used unpaired");
        return;
    }

    qWarning() << "PGP >>> paired, resuming";

    // The link stays up through pairing, so we can pick up where we left off
    subscribeToSfidaCommands();
}

void DevicePokemonGoPlus::serviceDetailsDiscovered_control(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        qDebug() << "DevicePokemonGoPlus::serviceDetailsDiscovered_control(" << m_deviceAddress << ") > ServiceDiscovered";

        // Subscribe to button
        QLowEnergyCharacteristic bb = m_serviceControl->characteristic(s_chrButtonNotif);
        m_buttonNotifDesc = bb.clientCharacteristicConfiguration();
        m_serviceControl->writeDescriptor(m_buttonNotifDesc, QByteArray::fromHex("0100"));
    }
}

/* ************************************************************************** */

void DevicePokemonGoPlus::bleWriteDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DevicePokemonGoPlus::bleWriteDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
}

void DevicePokemonGoPlus::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DevicePokemonGoPlus::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid() == s_chrSfidaToCentral)
    {
        certificationData(value);
    }
}

void DevicePokemonGoPlus::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DevicePokemonGoPlus::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA (" << value.size() << "bytes)   >  0x" << value.toHex();

    if (c.uuid() == s_chrSfidaCommands)
    {
        certificationNotify(value);
    }
    else if (c.uuid() == s_chrButtonNotif)
    {
        qDebug() << "BUTTON DATA ??? (" << value.size() << "bytes)   >  0x" << value.toHex();
        Q_EMIT btnChanged();
    }
}

void DevicePokemonGoPlus::bleServiceErrored(QLowEnergyService::ServiceError error)
{
    if (error == QLowEnergyService::NoError) return;
    qWarning() << "DevicePokemonGoPlus::bleServiceErrored(" << m_deviceAddress << ") error:" << error;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DevicePokemonGoPlus::readSfidaToCentral()
{
    if (!m_serviceCertificate) return;

    QLowEnergyCharacteristic c = m_serviceCertificate->characteristic(s_chrSfidaToCentral);
    if (!c.isValid()) { certificationFailed("SFIDA_TO_CENTRAL not found"); return; }

    m_serviceCertificate->readCharacteristic(c);
}

void DevicePokemonGoPlus::writeCentralToSfida(const QByteArray &payload)
{
    if (!m_serviceCertificate) return;

    QLowEnergyCharacteristic c = m_serviceCertificate->characteristic(s_chrCentralToSfida);
    if (!c.isValid()) { certificationFailed("CENTRAL_TO_SFIDA not found"); return; }

    qDebug() << "crypto >>> writing" << payload.size() << "bytes to CENTRAL_TO_SFIDA";

    m_serviceCertificate->writeCharacteristic(c, payload, QLowEnergyService::WriteWithResponse);

    // Steps 0 to 2 are driven by SFIDA_COMMANDS notifications, so we let those arrive on
    // their own. Nothing is announced after the final challenge though, so go and look at
    // SFIDA_TO_CENTRAL ourselves rather than waiting for a notification that may not come.

    if (m_certState == CERT_FINAL)
    {
        QTimer::singleShot(300, this, [this] () {
            if (m_certState != CERT_FINAL) return;
            qDebug() << "crypto >>> final challenge sent, checking SFIDA_TO_CENTRAL";
            readSfidaToCentral();
        });
    }
}

void DevicePokemonGoPlus::certificationFailed(const QString &why)
{
    qWarning() << "PGP crypto >>> certification FAILED at state" << m_certState << ":" << why;
    m_certState = CERT_FAILED;
}

void DevicePokemonGoPlus::certificationDone()
{
    qDebug() << "PGP crypto >>> certificationDone()";
    m_certState = CERT_DONE;

    // Certification is what authorizes GATT: before this the device answers reads and
    // writes with "Insufficient Authorization", after it everything opens up. So this is
    // where the other two services get discovered, not at connection time.

    qDebug() << "PGP >>> battery service" << m_serviceBattery
             << (m_serviceBattery ? m_serviceBattery->state() : QLowEnergyService::InvalidService)
             << "/ control service" << m_serviceControl
             << (m_serviceControl ? m_serviceControl->state() : QLowEnergyService::InvalidService);

    //
    discoverControlService();
}

/*!
 * Only valid once certified, the device refuses everything before that.
 * One at a time: two discoveries queued together on the same controller and only the first one ran.
 */
void DevicePokemonGoPlus::discoverControlService()
{
    if (m_serviceBattery && m_serviceBattery->state() == QLowEnergyService::RemoteService)
    {
        if (m_serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            // Full discovery, we read the battery level from the cached value
            QTimer::singleShot(0, this, [this] () { m_serviceBattery->discoverDetails(); });
        }
        else
        {
            qWarning() << "PGP >>> battery service is not discoverable, state" << m_serviceBattery->state();
        }
    }
    else if (m_serviceControl && m_serviceControl->state() == QLowEnergyService::RemoteService)
    {
        if (m_serviceControl->state() == QLowEnergyService::RemoteService)
        {
            // We only write the LED and subscribe to the button, no values to read
            QTimer::singleShot(0, this, [this] () { m_serviceControl->discoverDetails(QLowEnergyService::SkipValueDiscovery); });
        }
        else
        {
            qWarning() << "PGP >>> control service is not discoverable, state" << m_serviceControl->state();
        }
    }
}

/*!
 * Kicks off the handshake by reading the challenge the device is already offering.
 */
void DevicePokemonGoPlus::certificationStart()
{
    qDebug() << "PGP crypto >>> certificationStart()";

    m_keys = pgp_load_keys(m_deviceAddress);
    if (!m_keys.isValid())
    {
        certificationFailed("no device key, put a key dump in " + pgp_keys_directory());
        return;
    }

    // Every connection gets a fresh session key from the device, so nothing survives a reconnection.
    // This runs on each connection, the certificate service being discovered is what starts the handshake.

    m_certState = CERT_IDLE;
    m_sessionKey.clear();

    // We do not read anything here. The device announces each step on SFIDA_COMMANDS and we fetch
    // the payload in response to that, so the exchange stays exactly one read and one write per step.
    // Reading unprompted puts an extra ATT transaction on the wire that the device does not expect.
}

/*!
 * SFIDA_COMMANDS notification, 4 bytes of state. Every step the device takes is
 * announced here, and the payload that goes with it is read from SFIDA_TO_CENTRAL.
 */
void DevicePokemonGoPlus::certificationNotify(const QByteArray &value)
{
    if (value.size() < 4) return;
    if (m_certState == CERT_FAILED) return;

    const quint8 state = static_cast<quint8>(value.at(0));
    qDebug() << "PGP crypto >>> notify state" << state << "raw" << value.left(4).toHex();

    // 04 00 23 00 is the device telling us certification is complete
    if (state == 0x04)
    {
        if (m_certState == CERT_DONE) return;
        certificationDone();
        return;
    }

    // Every other state has a payload waiting for us
    readSfidaToCentral();
}

/*!
 * Payload read back from SFIDA_TO_CENTRAL, meaning depends on where we are.
 */
void DevicePokemonGoPlus::certificationData(const QByteArray &value)
{
    if (m_certState == CERT_FAILED || m_certState == CERT_DONE) return;
    if (!m_keys.isValid()) return;

    // We ask for the challenge both when the certificate service is discovered and when
    // the device announces state 0, so the initial 378 bytes can come back twice.
    // Once we have answered it, a repeat is a duplicate and not the next step.

    if (m_certState != CERT_IDLE && value.size() == static_cast<int>(sizeof(struct challenge_data)))
    {
        qDebug() << "PGP crypto >>> ignoring duplicate initial challenge";
        return;
    }

    switch (m_certState)
    {
    case CERT_IDLE:
    {
        // The initial 378 bytes challenge, encrypted with this device's key
        if (value.size() < static_cast<int>(sizeof(struct challenge_data)))
        {
            certificationFailed(QString("expected %1 bytes challenge, got %2")
                                .arg(sizeof(struct challenge_data)).arg(value.size()));
            return;
        }

        struct main_challenge_data main;
        const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

        if (!app_decrypt_chal_0(data, reinterpret_cast<const quint8 *>(m_keys.deviceKey.constData()), &main))
        {
            certificationFailed("outer hash mismatch, the device key is wrong for this device");
            return;
        }

        quint8 challenge[16];
        if (!app_decrypt_challenge(&main, challenge))
        {
            certificationFailed("inner hash mismatch");
            return;
        }

        // Keep the session key, everything after this point is keyed with it
        m_sessionKey = QByteArray(reinterpret_cast<const char *>(main.key), 16);

        // Sanity check the decrypted structure against something we already know:
        // the device puts its own address in there, byte reversed. If that matches,
        // the decryption is right in substance and not merely self consistent.

        qDebug() << "PGP crypto >>> decrypted bt_addr"
                 << QByteArray(reinterpret_cast<const char *>(main.bt_addr), 6).toHex(':')
                 << "(expecting" << m_deviceAddress.toLower() << "reversed)";
        qDebug() << "PGP crypto >>> flash_data"
                 << QByteArray(reinterpret_cast<const char *>(main.flash_data), 10).toHex();
        qDebug() << "PGP crypto >>> challenge"
                 << QByteArray(reinterpret_cast<const char *>(challenge), 16).toHex();

        // Echo the challenge back, stamped with the state we are driving the device to.

        struct challenge_response resp;
        memset(&resp, 0, sizeof(resp));
        resp.state[0] = 0x01;
        memcpy(resp.response, challenge, 16);

        QByteArray answer(reinterpret_cast<const char *>(&resp), sizeof(resp));

        m_certState = CERT_CHALLENGE_SENT;
        writeCentralToSfida(answer);
        break;
    }

    case CERT_CHALLENGE_SENT:
    {
        // A 52 bytes next_challenge from the device, which we verify against the
        // session key, then answer with one of our own built over random data
        if (value.size() < static_cast<int>(sizeof(struct next_challenge)))
        {
            certificationFailed(QString("expected %1 bytes next challenge, got %2")
                                .arg(sizeof(struct next_challenge)).arg(value.size()));
            return;
        }

        quint8 plain[16];
        const quint8 *key = reinterpret_cast<const quint8 *>(m_sessionKey.constData());

        if (!decrypt_next(reinterpret_cast<const quint8 *>(value.constData()), key, plain))
        {
            certificationFailed("next challenge hash mismatch");
            return;
        }

        // Our own challenge to the device: it has to send these 16 bytes back decrypted,
        // which is how it proves it holds the session key
        quint8 ours[16];
        for (int i = 0; i < 16; i++) ours[i] = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
        m_appChallenge = QByteArray(reinterpret_cast<const char *>(ours), 16);

        quint8 nonce[16];
        generate_nonce(nonce);

        struct next_challenge out;
        memset(&out, 0, sizeof(out));
        generate_next_chal(ours, key, nonce, &out);
        out.state[0] = 0x02;    // drive the device from state 1 to state 2

        m_certState = CERT_EXCHANGE;
        writeCentralToSfida(QByteArray(reinterpret_cast<const char *>(&out), sizeof(out)));
        break;
    }

    case CERT_EXCHANGE:
    {
        // The device answers our step 2 challenge with a 4 bytes state followed by the
        // 16 bytes we sent, decrypted. Checking them is what authenticates the device to
        // us, the earlier steps only authenticated us to it.
        if (value.size() >= 20 && m_appChallenge.size() == 16)
        {
            if (value.mid(4, 16) != m_appChallenge)
            {
                certificationFailed("the device did not return our challenge, it does not hold the session key");
                return;
            }
            qDebug() << "PGP crypto >>> device proved it holds the session key";
        }
        else
        {
            qWarning() << "PGP crypto >>> unexpected step 2 answer," << value.size() << "bytes:" << value.toHex();
        }

        // The last message wraps a fixed string.
        // We send "PokemonGoooooooo" encrypted with the session key, and the state field goesback to 0 rather than continuing to count up.
        static const char marker[16] = {'P','o','k','e','m','o','n','G','o','o','o','o','o','o','o','o'};

        quint8 nonce[16];
        generate_nonce(nonce);

        struct next_challenge out;
        memset(&out, 0, sizeof(out));
        generate_next_chal(reinterpret_cast<const quint8 *>(marker),
                           reinterpret_cast<const quint8 *>(m_sessionKey.constData()),
                           nonce, &out);

        out.state[0] = 0x00;

        m_certState = CERT_FINAL;
        writeCentralToSfida(QByteArray(reinterpret_cast<const char *>(&out), sizeof(out)));
        break;
    }

    case CERT_FINAL:
        // Waiting for the '04 00 23 00' notification, nothing to do with this payload
        qDebug() << "PGP crypto >>> post-final payload," << value.size() << "bytes:" << value.toHex();
        break;

    default:
        break;
    }
}

/* ************************************************************************** */
