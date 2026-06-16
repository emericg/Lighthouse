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

#ifndef NETWORK_ART_PROVIDER_H
#define NETWORK_ART_PROVIDER_H
/* ************************************************************************** */

#include "network_client.h"

#include <QQuickImageProvider>
#include <QImage>

/* ************************************************************************** */

/*!
 * \brief Serves the network client's current media artwork to QML.
 *
 * The artwork arrives as raw image bytes over the network and is decoded into a
 * QImage by the NetworkClient. Going through an image provider (rather than a
 * base64 "data:" URL) avoids QUrl mangling the base64 payload, and keeps the
 * decode off the QML string path. The engine takes ownership once added.
 */
class NetworkArtProvider : public QQuickImageProvider
{
    NetworkClient *m_client = nullptr;

public:
    explicit NetworkArtProvider(NetworkClient *client) : QQuickImageProvider(QQuickImageProvider::Image), m_client(client) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        Q_UNUSED(id) // the id only varies to defeat the QML cache, the image is always "current"

        QImage img = m_client ? m_client->currentArt() : QImage();
        if (size) *size = img.size();

        if (!img.isNull() && requestedSize.width() > 0 && requestedSize.height() > 0)
        {
            return img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        return img;
    }
};

/* ************************************************************************** */
#endif // NETWORK_ART_PROVIDER_H
