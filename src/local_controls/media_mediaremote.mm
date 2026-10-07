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

#if defined(ENABLE_MEDIA_MEDIAREMOTE)

#include "media_mediaremote.h"
#include "local_actions.h"

#import <AppKit/AppKit.h>
#include <dlfcn.h>

#include <QPointer>
#include <QTimer>
#include <QDebug>

/* ************************************************************************** */

// MediaRemote private API, loaded at runtime

typedef void (*MRRegisterForNowPlayingNotificationsFn)(dispatch_queue_t queue);
typedef void (*MRUnregisterForNowPlayingNotificationsFn)(void);
typedef void (*MRGetNowPlayingInfoFn)(dispatch_queue_t queue, void (^handler)(CFDictionaryRef info));
typedef void (*MRGetNowPlayingApplicationIsPlayingFn)(dispatch_queue_t queue, void (^handler)(Boolean playing));
typedef void (*MRGetNowPlayingApplicationPIDFn)(dispatch_queue_t queue, void (^handler)(int pid));
typedef Boolean (*MRSendCommandFn)(int command, CFDictionaryRef options);
typedef void (*MRSetElapsedTimeFn)(double elapsed);

static MRRegisterForNowPlayingNotificationsFn MRRegisterForNowPlayingNotifications = nullptr;
static MRUnregisterForNowPlayingNotificationsFn MRUnregisterForNowPlayingNotifications = nullptr;
static MRGetNowPlayingInfoFn MRGetNowPlayingInfo = nullptr;
static MRGetNowPlayingApplicationIsPlayingFn MRGetNowPlayingApplicationIsPlaying = nullptr;
static MRGetNowPlayingApplicationPIDFn MRGetNowPlayingApplicationPID = nullptr;
static MRSendCommandFn MRSendCommand = nullptr;
static MRSetElapsedTimeFn MRSetElapsedTime = nullptr;

// MRMediaRemoteCommand
enum
{
    kMRPlay = 0,
    kMRPause = 1,
    kMRTogglePlayPause = 2,
    kMRStop = 3,
    kMRNextTrack = 4,
    kMRPreviousTrack = 5,
};

static NSString *const kNowPlayingInfoDidChange = @"kMRMediaRemoteNowPlayingInfoDidChangeNotification";
static NSString *const kNowPlayingApplicationDidChange = @"kMRMediaRemoteNowPlayingApplicationDidChangeNotification";
static NSString *const kNowPlayingIsPlayingDidChange = @"kMRMediaRemoteNowPlayingApplicationIsPlayingDidChangeNotification";

static NSString *const kInfoTitle = @"kMRMediaRemoteNowPlayingInfoTitle";
static NSString *const kInfoArtist = @"kMRMediaRemoteNowPlayingInfoArtist";
static NSString *const kInfoAlbum = @"kMRMediaRemoteNowPlayingInfoAlbum";
static NSString *const kInfoDuration = @"kMRMediaRemoteNowPlayingInfoDuration";
static NSString *const kInfoElapsedTime = @"kMRMediaRemoteNowPlayingInfoElapsedTime";
static NSString *const kInfoTimestamp = @"kMRMediaRemoteNowPlayingInfoTimestamp";
static NSString *const kInfoPlaybackRate = @"kMRMediaRemoteNowPlayingInfoPlaybackRate";
static NSString *const kInfoArtworkData = @"kMRMediaRemoteNowPlayingInfoArtworkData";
static NSString *const kInfoArtworkMIMEType = @"kMRMediaRemoteNowPlayingInfoArtworkMIMEType";
static NSString *const kInfoArtworkIdentifier = @"kMRMediaRemoteNowPlayingInfoArtworkIdentifier";
static NSString *const kInfoContentItemIdentifier = @"kMRMediaRemoteNowPlayingInfoContentItemIdentifier";

static QString toQString(id value)
{
    return [value isKindOfClass:[NSString class]] ? QString::fromNSString(value) : QString();
}

static double toDouble(id value, double fallback = 0.0)
{
    return [value isKindOfClass:[NSNumber class]] ? [value doubleValue] : fallback;
}

/* ************************************************************************** */

Media_mediaremote *Media_mediaremote::instance = nullptr;

Media_mediaremote *Media_mediaremote::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Media_mediaremote();
    }

    return instance;
}

Media_mediaremote::Media_mediaremote()
{
    // extrapolate the position while playing, at the same pace as the MPRIS polling
    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(1000);
    connect(m_positionTimer, &QTimer::timeout, this, &Media_mediaremote::updatePosition);

    load();
    refreshApplication();
}

Media_mediaremote::~Media_mediaremote()
{
    if (m_observers)
    {
        NSMutableArray *observers = static_cast<NSMutableArray *>(m_observers);
        for (id observer in observers)
        {
            [[NSNotificationCenter defaultCenter] removeObserver:observer];
        }
        [observers release];
        m_observers = nullptr;
    }

    if (m_loaded) MRUnregisterForNowPlayingNotifications();
}

/* ************************************************************************** */

void Media_mediaremote::load()
{
    if ([[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){15, 4, 0}])
    {
        qWarning() << "Media_mediaremote: MediaRemote is restricted to Apple processes since macOS 15.4";
        return;
    }

    void *handle = dlopen("/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote", RTLD_NOW);
    if (!handle)
    {
        qWarning() << "Media_mediaremote: unable to load MediaRemote" << dlerror();
        return;
    }

    MRRegisterForNowPlayingNotifications = (MRRegisterForNowPlayingNotificationsFn)dlsym(handle, "MRMediaRemoteRegisterForNowPlayingNotifications");
    MRUnregisterForNowPlayingNotifications = (MRUnregisterForNowPlayingNotificationsFn)dlsym(handle, "MRMediaRemoteUnregisterForNowPlayingNotifications");
    MRGetNowPlayingInfo = (MRGetNowPlayingInfoFn)dlsym(handle, "MRMediaRemoteGetNowPlayingInfo");
    MRGetNowPlayingApplicationIsPlaying = (MRGetNowPlayingApplicationIsPlayingFn)dlsym(handle, "MRMediaRemoteGetNowPlayingApplicationIsPlaying");
    MRGetNowPlayingApplicationPID = (MRGetNowPlayingApplicationPIDFn)dlsym(handle, "MRMediaRemoteGetNowPlayingApplicationPID");
    MRSendCommand = (MRSendCommandFn)dlsym(handle, "MRMediaRemoteSendCommand");
    MRSetElapsedTime = (MRSetElapsedTimeFn)dlsym(handle, "MRMediaRemoteSetElapsedTime");

    if (!MRRegisterForNowPlayingNotifications || !MRUnregisterForNowPlayingNotifications ||
        !MRGetNowPlayingInfo || !MRGetNowPlayingApplicationIsPlaying ||
        !MRGetNowPlayingApplicationPID || !MRSendCommand || !MRSetElapsedTime)
    {
        qWarning() << "Media_mediaremote: MediaRemote is missing some functions";
        return;
    }

    m_loaded = true;

    // callbacks and notifications are delivered on the main queue, which Qt's Cocoa event loop runs
    MRRegisterForNowPlayingNotifications(dispatch_get_main_queue());

    QPointer<Media_mediaremote> self(this);
    NSMutableArray *observers = [[NSMutableArray alloc] init];
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];

    [observers addObject:[center addObserverForName:kNowPlayingApplicationDidChange object:nil queue:mainQueue
                                         usingBlock:^(NSNotification *) { if (self) self->refreshApplication(); }]];
    [observers addObject:[center addObserverForName:kNowPlayingIsPlayingDidChange object:nil queue:mainQueue
                                         usingBlock:^(NSNotification *) { if (self) self->refreshPlaying(); }]];
    [observers addObject:[center addObserverForName:kNowPlayingInfoDidChange object:nil queue:mainQueue
                                         usingBlock:^(NSNotification *) { if (self) self->refreshInfo(); }]];

    m_observers = observers;
}

/* ************************************************************************** */

void Media_mediaremote::refreshApplication()
{
    if (!m_loaded) return;

    QPointer<Media_mediaremote> self(this);
    MRGetNowPlayingApplicationPID(dispatch_get_main_queue(), ^(int pid) {
        if (!self) return;

        if (pid != m_pid)
        {
            m_pid = pid;

            m_playerName.clear();
            if (pid > 0)
            {
                // web players report the browser itself
                NSRunningApplication *app = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
                if (app.localizedName) m_playerName = QString::fromNSString(app.localizedName);
                else if (app.bundleIdentifier) m_playerName = QString::fromNSString(app.bundleIdentifier);
            }

            // MediaRemote has no capability flags, every command is sent and the player may ignore it
            m_canControl = m_canPlayPause = m_canGoPrevious = m_canGoNext = (pid > 0);

            Q_EMIT playerUpdated();
        }

        refreshPlaying();
        refreshInfo();
    });
}

void Media_mediaremote::refreshPlaying()
{
    if (!m_loaded) return;

    QPointer<Media_mediaremote> self(this);
    MRGetNowPlayingApplicationIsPlaying(dispatch_get_main_queue(), ^(Boolean playing) {
        if (self) setPlaying(playing);
    });
}

void Media_mediaremote::refreshInfo()
{
    if (!m_loaded) return;

    QPointer<Media_mediaremote> self(this);
    MRGetNowPlayingInfo(dispatch_get_main_queue(), ^(CFDictionaryRef info) {
        if (self) applyInfo(info);
    });
}

/* ************************************************************************** */

void Media_mediaremote::applyInfo(const void *info_ptr)
{
    NSDictionary *info = (__bridge NSDictionary *)info_ptr;

    const QString title = toQString(info[kInfoTitle]);
    const QString artist = toQString(info[kInfoArtist]);
    const QString album = toQString(info[kInfoAlbum]);
    const qint64 duration = qint64(toDouble(info[kInfoDuration]) * 1000000.0);

    // the artwork usually comes with a single update, the next ones (ex: playback starting) omit it
    // even though the track still has one. Some players (Firefox) also change the content item
    // identifier with every update, so the title and artist decide too.
    QString trackId = toQString(info[kInfoContentItemIdentifier]);
    if (trackId.isEmpty() && !title.isEmpty()) trackId = title + QChar(0x1F) + artist + QChar(0x1F) + album;
    const bool sameTrack = (!trackId.isEmpty() && trackId == m_trackId) ||
                           (!title.isEmpty() && title == m_metaTitle && artist == m_metaArtist);
    m_trackId = trackId;

    QString artworkId = toQString(info[kInfoArtworkIdentifier]);
    if (artworkId.isEmpty()) artworkId = trackId;

    NSData *artwork = info[kInfoArtworkData];
    if (![artwork isKindOfClass:[NSData class]] || [artwork length] == 0) artwork = nil;

    QString thumbnail = m_metaThumbnail;
    if (!artwork)
    {
        // keep the previous artwork while the track does not change
        if (!sameTrack)
        {
            thumbnail.clear();
            m_artworkId.clear();
        }
    }
    else if (artworkId != m_artworkId || m_metaThumbnail.isEmpty())
    {
        QString mime = toQString(info[kInfoArtworkMIMEType]);
        if (mime.isEmpty()) mime = QStringLiteral("image/jpeg");

        const QByteArray bytes = QByteArray::fromRawNSData(artwork);
        thumbnail = QStringLiteral("data:") + mime + QStringLiteral(";base64,") + QString::fromLatin1(bytes.toBase64());
        m_artworkId = artworkId;
    }

    if (title != m_metaTitle || artist != m_metaArtist || album != m_metaAlbum ||
        duration != m_metaDuration || thumbnail != m_metaThumbnail)
    {
        m_metaTitle = title;
        m_metaArtist = artist;
        m_metaAlbum = album;
        m_metaDuration = duration;
        m_metaThumbnail = thumbnail;
        m_canSeek = (duration > 0);

        Q_EMIT metadataUpdated();
    }

    // position
    if (info[kInfoElapsedTime])
    {
        m_elapsed_s = toDouble(info[kInfoElapsedTime]);

        NSDate *timestamp = info[kInfoTimestamp];
        m_elapsedTimestamp = [timestamp isKindOfClass:[NSDate class]] ? QDateTime::fromNSDate(timestamp)
                                                                     : QDateTime::currentDateTimeUtc();
    }
    else
    {
        m_elapsed_s = -1.0;
    }

    const double rate = toDouble(info[kInfoPlaybackRate], -1.0);
    if (!qFuzzyCompare(rate + 2.0, m_rate + 2.0))
    {
        m_rate = rate;
        Q_EMIT rateUpdated();
    }

    updatePosition();
}

void Media_mediaremote::setPlaying(bool playing)
{
    m_playing = playing;

    const QString status = (m_pid <= 0) ? QString() : (playing ? QStringLiteral("Playing") : QStringLiteral("Paused"));
    if (status != m_playbackStatus)
    {
        m_playbackStatus = status;
        Q_EMIT statusUpdated();
    }

    if (playing) m_positionTimer->start();
    else m_positionTimer->stop();

    updatePosition();
}

void Media_mediaremote::updatePosition()
{
    qint64 pos = -1;

    if (m_pid > 0 && m_elapsed_s >= 0.0)
    {
        double elapsed = m_elapsed_s;

        // the reported elapsed time is a snapshot, it keeps running while playing
        if (m_playing && m_rate > 0.0 && m_elapsedTimestamp.isValid())
        {
            elapsed += m_elapsedTimestamp.msecsTo(QDateTime::currentDateTimeUtc()) / 1000.0 * m_rate;
        }

        pos = qint64(elapsed * 1000000.0);
        if (m_metaDuration > 0) pos = qBound<qint64>(0, pos, m_metaDuration);
    }

    if (pos != m_position_us)
    {
        m_position_us = pos;
        Q_EMIT positionUpdated();
    }
}

/* ************************************************************************** */

bool Media_mediaremote::select_player()
{
    // macOS picks the now playing application itself, we can only refresh our view of it
    refreshApplication();
    return isAvailable();
}

bool Media_mediaremote::sendCommand(int command)
{
    if (!m_loaded) return false;
    return MRSendCommand(command, nullptr);
}

void Media_mediaremote::action(unsigned action_code)
{
    if (action_code == LocalActions::ACTION_MEDIA_playpause) media_playpause();
    else if (action_code == LocalActions::ACTION_MEDIA_stop) media_stop();
    else if (action_code == LocalActions::ACTION_MEDIA_next) media_next();
    else if (action_code == LocalActions::ACTION_MEDIA_prev) media_prev();
}

void Media_mediaremote::media_playpause()
{
    sendCommand(kMRTogglePlayPause);
}

void Media_mediaremote::media_stop()
{
    sendCommand(kMRStop);
}

void Media_mediaremote::media_next()
{
    sendCommand(kMRNextTrack);
}

void Media_mediaremote::media_prev()
{
    sendCommand(kMRPreviousTrack);
}

void Media_mediaremote::media_seek(qint64 offset_us)
{
    if (!m_loaded || m_position_us < 0) return;

    updatePosition();

    qint64 target = m_position_us + offset_us;
    if (m_metaDuration > 0) target = qBound<qint64>(0, target, m_metaDuration);
    else target = qMax<qint64>(0, target);

    MRSetElapsedTime(target / 1000000.0);
}

/* ************************************************************************** */
#endif // defined(ENABLE_MEDIA_MEDIAREMOTE)
