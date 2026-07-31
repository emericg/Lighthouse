/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 2026 Emeric Grange - All Rights Reserved
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

#ifndef PGP_KEYSTORE_H
#define PGP_KEYSTORE_H
/* ************************************************************************** */

#include <QByteArray>
#include <QString>

/* ************************************************************************** */

/*!
 * Per device secrets for a "Pokemon GO Plus".
 *
 * Every PGP has a device key burned into its OTP memory, without which the certification handshake cannot be answered.
 * There is no way to derive it, it has to be extracted from the device itself (see docs/porygon.md),
 * so we load it from a key dump the user provides.
 *
 * Dump your using the SUOTA Go+ tool (https://github.com/Jesus805/Suota-Go-Plus)
 * and write the resulting '.json' file at the designated 'AppDataLocation'.
 *
 * On Linux that's : "~/.local/share/Lighthouse/Lighthouse/pgp/"
 */
struct PgpKeys
{
    QByteArray deviceKey;   //!< 16 bytes
    QByteArray blob;        //!< 256 bytes
    QString address;        //!< the device this belongs to

    bool isValid() const { return (deviceKey.size() == 16 && blob.size() == 256); }
};

/* ************************************************************************** */

/*!
 * Loads the keys for a given device address.
 *
 * Key dumps are JSON files placed in <AppDataLocation>/pgp/, in the format produced by the usual firmware patching tools:
 * {
 *   "bluetooth": "AA:BB:CC:DD:EE:FF",
 *   "device": "<32 hex chars>",
 *   "blob": "<512 hex chars>"
 * }
 *
 * Every *.json in that directory is checked, the file name does not matter, only the "bluetooth" field.
 * Returns an invalid PgpKeys if no dump matches.
 */
PgpKeys pgp_load_keys(const QString &deviceAddress);

/*!
 * Directory the key dumps are read from, so we can tell the user where to put them.
 */
QString pgp_keys_directory();

/* ************************************************************************** */
#endif // PGP_KEYSTORE_H
