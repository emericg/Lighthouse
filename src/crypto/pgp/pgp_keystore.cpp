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

#include "pgp_keystore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <QDebug>

/* ************************************************************************** */

QString pgp_keys_directory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pgp";
}

/*!
 * File name a dump gets when we install it: the device address, minus the characters
 * that are not legal in a file name on every platform we build for ("::" on Windows,
 * the braces around the UUID we use as an address on macOS and iOS).
 */
static QString pgp_keys_filename(const QString &deviceAddress)
{
    QString name = deviceAddress.trimmed().toUpper();

    name.replace(':', '-');
    name.remove('{');
    name.remove('}');

    return name + ".json";
}

PgpKeys pgp_load_keys(const QString &deviceAddress)
{
    PgpKeys keys;

    QDir dir(pgp_keys_directory());
    if (!dir.exists())
    {
        qWarning() << "PGP: no key directory, expected key dumps in" << dir.absolutePath();
        return keys;
    }

    const QString wanted = deviceAddress.trimmed().toUpper();

    const QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    for (const QString &name: files)
    {
        QFile f(dir.filePath(name));
        if (!f.open(QIODevice::ReadOnly)) continue;

        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        f.close();

        if (err.error != QJsonParseError::NoError || !doc.isObject())
        {
            qWarning() << "PGP: cannot parse key dump" << name << ":" << err.errorString();
            continue;
        }

        const QJsonObject obj = doc.object();
        if (obj.value("bluetooth").toString().trimmed().toUpper() != wanted) continue;

        // We never log the values themselves, only whether they are the right shape
        const QByteArray deviceKey = QByteArray::fromHex(obj.value("device").toString().toLatin1());
        const QByteArray blob = QByteArray::fromHex(obj.value("blob").toString().toLatin1());

        if (deviceKey.size() != 16 || blob.size() != 256)
        {
            qWarning() << "PGP: key dump" << name << "matches" << wanted
                       << "but is malformed (device key" << deviceKey.size()
                       << "bytes, blob" << blob.size() << "bytes)";
            continue;
        }

        keys.deviceKey = deviceKey;
        keys.blob = blob;
        keys.address = wanted;

        qDebug() << "PGP: loaded keys for" << wanted << "from" << name;
        return keys;
    }

    qWarning() << "PGP: no key dump for" << wanted << "in" << dir.absolutePath();
    return keys;
}

bool pgp_has_keys(const QString &deviceAddress)
{
    return pgp_load_keys(deviceAddress).isValid();
}

bool pgp_import_keys(const QString &deviceAddress, const QString &filePath)
{
    const QString wanted = deviceAddress.trimmed().toUpper();

    QFile src(filePath);
    if (!src.open(QIODevice::ReadOnly))
    {
        qWarning() << "PGP: cannot read key dump" << filePath << ":" << src.errorString();
        return false;
    }
    const QByteArray content = src.readAll();
    src.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(content, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "PGP: cannot parse key dump" << filePath << ":" << err.errorString();
        return false;
    }

    // We never log the values themselves, only whether they are the right shape
    const QJsonObject obj = doc.object();
    const QString address = obj.value("bluetooth").toString().trimmed().toUpper();
    const QByteArray deviceKey = QByteArray::fromHex(obj.value("device").toString().toLatin1());
    const QByteArray blob = QByteArray::fromHex(obj.value("blob").toString().toLatin1());

    if (deviceKey.size() != 16 || blob.size() != 256)
    {
        qWarning() << "PGP: key dump" << filePath << "is malformed (device key" << deviceKey.size()
                   << "bytes, blob" << blob.size() << "bytes)";
        return false;
    }
    if (address != wanted)
    {
        // The dump carries the address it was taken from, and that is what we match on when loading
        qWarning() << "PGP: key dump" << filePath << "belongs to" << address << "not to" << wanted;
        return false;
    }

    QDir dir(pgp_keys_directory());
    if (!dir.mkpath("."))
    {
        qWarning() << "PGP: cannot create key directory" << dir.absolutePath();
        return false;
    }

    // Note: the source file may already be the destination, so the content has to be read before this
    const QString destination = dir.filePath(pgp_keys_filename(wanted));
    QFile dst(destination);
    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "PGP: cannot write" << destination << ":" << dst.errorString();
        return false;
    }
    const bool written = (dst.write(content) == content.size());
    dst.close();

    if (!written)
    {
        qWarning() << "PGP: cannot write" << destination;
        return false;
    }

    qDebug() << "PGP: installed keys for" << wanted << "at" << destination;
    return true;
}

/* ************************************************************************** */
