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

#include "device_ylkg07yl.h"

#if !defined(Q_OS_ANDROID)
#include "mbedtls/aes.h"
#include "mbedtls/ccm.h"
#endif

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QByteArray>
#include <QDebug>

#include <cstdint>
#include <cmath>

/* ************************************************************************** */

DeviceYLKG07YL::DeviceYLKG07YL(QString &deviceAddr, QString &deviceName, QObject *parent):
    DeviceBeacon(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;

    m_deviceModel = "YLKG07YL";
    m_deviceModelID = "YLKG07YL";

    m_beaconkey = getSetting("beaconkey").toString();
}

DeviceYLKG07YL::DeviceYLKG07YL(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceBeacon(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_REMOTE;
    m_deviceCapabilities += DeviceUtils::DEVICE_BUTTONS;

    m_deviceModel = "YLKG07YL";
    m_deviceModelID = "YLKG07YL";

    m_beaconkey = getSetting("beaconkey").toString();
}

DeviceYLKG07YL::~DeviceYLKG07YL()
{
    //
}

/* ************************************************************************** */

void DeviceYLKG07YL::setBeaconKey(const QString &key)
{
    //qDebug() << "DeviceYLKG07YL::setBeaconKey(" << key << ")";

    if (m_beaconkey != key)
    {
        m_beaconkey = key;
        Q_EMIT beaconkeyChanged();

        setSetting("beaconkey", key);
    }
}

/* ************************************************************************** */

void DeviceYLKG07YL::parseAdvertisementData(const uint16_t type,
                                            const uint16_t identifier,
                                            const QByteArray &ba)
{
/*
    qDebug() << "DeviceYLKG07YL::parseAdvertisementData(" << m_deviceAddress
             << " - " << type << " - 0x" << identifier << ")";
    qDebug() << "DATA (" << ba.size() << "bytes)   >  0x" << ba.toHex();
*/

/*
    ServiceID     [2Byte] 95FE
    FrameControl  [2B]    5830          <-- encyrpted, contains Obj, version 3, no auth
    Product ID    [2B]    B603
    Sequence      [1B]    8B
    MAC           [6B]    112233445566
    OBJ Encyrpted [6B]    E0B0EA70E356  <-- payload
    Ext seq       [3B]    080000
*/

    if (type == DeviceUtils::BLE_ADV_SERVICEDATA && identifier == 0xFE95 && ba.size() == 21)
    {
        if (prev_data_dimmer.isEmpty()) { prev_data_dimmer = ba; return; } // init
        if (prev_data_dimmer.compare(ba) == 0) return; // duplicate

        const quint8 *data = reinterpret_cast<const quint8 *>(ba.constData());

        int FrameControl = data[0] + (data[1] << 8);
        int ProductID = data[2] + (data[3] << 8);
        int FrameSequence = data[4];

        // Save mac address (for macOS and iOS)
        if (!hasAddressMAC())
        {
            QString mac;

            mac += ba.mid(10,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(9,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(8,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(7,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(6,1).toHex().toUpper();
            mac += ':';
            mac += ba.mid(5,1).toHex().toUpper();

            setAddressMAC(mac);
        }

        // for instance, this is how the key works:
        uint8_t beaconkey[12] = {0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2};
        uint8_t beaconkey_padding[4] = {0x8D, 0x3D, 0x3C, 0x97};
        uint8_t key[16] = {0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1, 0x8D, 0x3D, 0x3C, 0x97, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2};
        uint8_t add[1] = {0x11};

        if (m_beaconkey.size() == 24)
        {
            const QByteArray keychars = QByteArray::fromHex(m_beaconkey.toLatin1());
            // fill the actual key
            key[0] = keychars.at(0);
            key[1] = keychars.at(1);
            key[2] = keychars.at(2);
            key[3] = keychars.at(3);
            key[4] = keychars.at(4);
            key[5] = keychars.at(5);
            // padding
            key[10] = keychars.at(6);
            key[11] = keychars.at(7);
            key[12] = keychars.at(8);
            key[13] = keychars.at(9);
            key[14] = keychars.at(10);
            key[15] = keychars.at(11);
        }
        else
        {
            return;
        }

        uint8_t *data_uint8 = new uint8_t [ba.size()];
        memcpy(data_uint8, data, ba.size());

        uint8_t *mac_reversed;
        uint8_t *frame_ctl_data;
        uint8_t *device_type;
        uint8_t *encrypted_payload;
        uint8_t *encrypted_payload_counter;
        uint8_t *packet_id;

        uint8_t nonce[13];
        uint8_t decrypted_payload[6];

        frame_ctl_data = data_uint8;
        device_type = data_uint8 + 2;
        packet_id = data_uint8 + 4;
        mac_reversed = data_uint8 + 5;
        encrypted_payload = data_uint8 + 11;
        encrypted_payload_counter = encrypted_payload + 6;

        uint8_t offset = 0;
        memcpy(nonce + offset, frame_ctl_data, 2);
        offset += 2;
        memcpy(nonce + offset, device_type, 2);
        offset += 2;
        memcpy(nonce + offset, packet_id, 1);
        offset += 1;
        memcpy(nonce + offset, encrypted_payload_counter, 3);
        offset += 3;
        memcpy(nonce + offset, mac_reversed, 5);
        offset += 5;

#if !defined(Q_OS_ANDROID)
        // USING MBED
        mbedtls_ccm_context mtctx;
        mbedtls_ccm_init(&mtctx);
        if (mbedtls_ccm_setkey(&mtctx, MBEDTLS_CIPHER_ID_AES, key, 128) != 0)
        {
            qWarning() << "CCM setup failed";
            return;
        }

        int res = mbedtls_ccm_star_auth_decrypt(&mtctx, 1,
                             nonce, 13,
                             NULL, 0,
                             add, add,
                             NULL, 0);
        if (res != 0)
            return;

        res = mbedtls_ccm_star_auth_decrypt(&mtctx, 6,
                             nonce, 13,
                             NULL, 0,
                             encrypted_payload, decrypted_payload,
                             NULL, 0);
        if (res != 0)
            return;

        mbedtls_ccm_free(&mtctx);
#endif

        int8_t value1 = decrypted_payload[3];
        int8_t value2 = decrypted_payload[4];
        int8_t state = decrypted_payload[5];

        int btn = 0;
        int mode = 0;

        if (state == 3) // clicked
        {
            btn = 1;
            mode = 0;
        }
        else
        {
            if (value1 != 0) { // press + rotate
                mode = 2;
                if (value1 < 0) btn = 2;
                if (value1 > 0) btn = 3;
            }
            if (value2 != 0) { // rotate
                mode = 0;
                if (value2 < 0) btn = 2;
                if (value2 > 0) btn = 3;
            }
        }

        triggerAction(btn, mode);
        triggerEvent(btn, mode);
/*
        qDebug() << "- ----------------";
        qDebug() << "DATA (" << ba.size() << "bytes)   >  0x" << ba.toHex();

        qDebug() << "- FrameControl:" << FrameControl;
        qDebug() << "- ProductID:" << ProductID;
        qDebug() << "- mac:" << m_deviceAddress;
        qDebug() << "- FrameSequence:" << FrameSequence;

        qDebug() << "- btn:" << btn;
        qDebug() << "- mode:" << mode;
*/
        prev_data_dimmer = ba;
    }
}

/* ************************************************************************** */
