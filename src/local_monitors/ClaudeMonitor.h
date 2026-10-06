/*!
 * This file is part of Lighthouse.
 * Copyright (c) 2022 Emeric Grange - All Rights Reserved
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

#ifndef CLAUDE_MONITOR_H
#define CLAUDE_MONITOR_H
/* ************************************************************************** */

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>

class QQmlEngine;
class QJSEngine;

class QTimer;
class QFileSystemWatcher;
class QJsonValue;
class QProcess;

/* ************************************************************************** */

/*!
 * \brief The ClaudeMonitor class
 *
 * Monitors the Claude Code plan limits: the rolling 5 hour session window and
 * the 7 day (weekly) window, both expressed as a percentage of the plan quota.
 *
 * Claude Code (>= 2.1.80, on Pro/Max plans) pipes a session JSON to whatever
 * command is registered as "statusLine" in ~/.claude/settings.json. That payload
 * carries the only authoritative rate limit numbers available locally:
 *
 * \code
 * {
 *   "model": { "display_name": "Opus 5", ... },
 *   "rate_limits": {
 *     "five_hour": { "used_percentage": 42.0, "resets_at": 1753980000 },
 *     "seven_day": { "used_percentage": 61.5, "resets_at": 1754300000 }
 *   },
 *   "prompt_cache": { "caching_observed": true, "warm": true, "ttl": "5m", "expires_at": 1753962300 }
 * }
 * \endcode
 *
 * The prompt cache of the captured session is tracked too (Claude Code >= 2.1.251):
 * once it goes cold, the next request re-processes the whole context and eats
 * noticeably more of the plan quota, so its expiry is worth a countdown.
 *
 * A tiny hook command is expected to capture that JSON to a file (see
 * statuslineCommand()), which this class then watches and parses. This class
 * never talks to the network itself, and the weekly window simply does not exist
 * anywhere else: it cannot be recomputed from the local ~/.claude/projects logs.
 *
 * The statusline only runs in the interactive Claude Code CLI. When the capture
 * is stale or missing (desktop app users, CLI not opened lately), probe() can
 * spawn a one-off headless Claude Code request and read the same limits from its
 * "rate_limit_event". That request consumes a little of the plan quota.
 *
 * All percentages are 0-100 doubles, or -1 when unknown. "Unknown" and "0% used"
 * are very different things and must not be conflated by the UI.
 */
class ClaudeMonitor: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    //! Mirrors the ENABLE_CLAUDE_MONITORING build option
    Q_PROPERTY(bool supported READ isSupported CONSTANT)
    //! Supported, and turned on in the settings: false means nothing runs
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)

    Q_PROPERTY(bool available READ isAvailable NOTIFY limitsChanged)
    Q_PROPERTY(bool stale READ isStale NOTIFY limitsChanged)
    Q_PROPERTY(int source READ getSource NOTIFY limitsChanged)
    Q_PROPERTY(int status READ getStatus NOTIFY limitsChanged)
    Q_PROPERTY(QString modelName READ getModelName NOTIFY limitsChanged)
    Q_PROPERTY(QString capturePath READ getCapturePath NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime capturedAt READ getCapturedAt NOTIFY limitsChanged)

    Q_PROPERTY(bool hookInstalled READ isHookInstalled NOTIFY hookChanged)
    Q_PROPERTY(bool hookForeign READ isHookForeign NOTIFY hookChanged)
    Q_PROPERTY(QString settingsPath READ getSettingsPath CONSTANT)

    Q_PROPERTY(bool probeAvailable READ isProbeAvailable NOTIFY probeChanged)
    Q_PROPERTY(int probeState READ getProbeState NOTIFY probeChanged)

    // 5 hour session window
    Q_PROPERTY(bool fiveHourValid READ isFiveHourValid NOTIFY limitsChanged)
    Q_PROPERTY(double fiveHourPercent READ getFiveHourPercent NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime fiveHourStart READ getFiveHourStart NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime fiveHourReset READ getFiveHourReset NOTIFY limitsChanged)
    Q_PROPERTY(int fiveHourRemaining READ getFiveHourRemaining NOTIFY countdownChanged)
    Q_PROPERTY(double fiveHourElapsedPercent READ getFiveHourElapsedPercent NOTIFY countdownChanged)
    Q_PROPERTY(int fiveHourPace READ getFiveHourPace NOTIFY countdownChanged)

    // 7 day window
    Q_PROPERTY(bool sevenDayValid READ isSevenDayValid NOTIFY limitsChanged)
    Q_PROPERTY(double sevenDayPercent READ getSevenDayPercent NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime sevenDayStart READ getSevenDayStart NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime sevenDayReset READ getSevenDayReset NOTIFY limitsChanged)
    Q_PROPERTY(int sevenDayRemaining READ getSevenDayRemaining NOTIFY countdownChanged)
    Q_PROPERTY(double sevenDayElapsedPercent READ getSevenDayElapsedPercent NOTIFY countdownChanged)
    Q_PROPERTY(int sevenDayPace READ getSevenDayPace NOTIFY countdownChanged)

    // prompt cache of the captured session
    Q_PROPERTY(bool cacheValid READ isCacheValid NOTIFY limitsChanged)
    Q_PROPERTY(int cacheTtl READ getCacheTtl NOTIFY limitsChanged)
    Q_PROPERTY(QDateTime cacheExpiry READ getCacheExpiry NOTIFY limitsChanged)
    Q_PROPERTY(int cacheRemaining READ getCacheRemaining NOTIFY countdownChanged)

public:
    enum DataSource
    {
        SourceNone = 0,     //!< no usable capture found
        SourceStatusline,   //!< official rate_limits, captured from the Claude Code statusline
        SourceProbe,        //!< official rate_limits, from a one-off headless Claude Code request
    };
    Q_ENUM(DataSource)

    //! Outcome of the last probe()
    enum ProbeState
    {
        ProbeNone = 0,      //!< never probed
        ProbeRunning,       //!< a probe is in flight
        ProbeDone,          //!< the last probe returned rate limits
        ProbeAuthExpired,   //!< the Claude Code OAuth session expired, a new /login is required
        ProbeFailed,        //!< the last probe failed otherwise (timeout, network, no rate limits)
    };
    Q_ENUM(ProbeState)

    /*!
     * \brief Freshness of the capture as a whole.
     *
     * Collapses "is there anything to show" and "is it still current" into one
     * value. Per window availability is not part of it: a window carries its own
     * percentage, negative when that particular window is unknown.
     */
    enum Capture
    {
        CaptureNone = 0,    //!< no usable capture, nothing to show
        CaptureLive,        //!< captured within the freshness TTL
        CaptureStale,       //!< older than the TTL, may no longer reflect the live limit
    };
    Q_ENUM(Capture)

    //! Overall severity, driven by the worst of the two windows
    enum Status
    {
        StatusUnknown = 0,  //!< no usable data
        StatusOk,           //!< below the near-limit threshold
        StatusNearLimit,    //!< >= 95% used
        StatusLimitHit,     //!< >= 100% used
    };
    Q_ENUM(Status)

    //! Consumption speed compared to how much of the window has elapsed
    enum Pace
    {
        PaceUnknown = 0,
        PaceSlowDown,       //!< burning quota faster than the clock
        PaceOnTrack,        //!< within +/- 10 points of the elapsed ratio
        PaceSpeedUp,        //!< quota to spare at the current rate
    };
    Q_ENUM(Pace)

private:
    //! A single rate limit window (5 hour or 7 day)
    struct Window
    {
        double percent = -1.0;      //!< 0-100, or -1 when unknown
        qint64 resetEpoch = -1;     //!< unix seconds, or -1 when unknown
        qint64 durationSecs = 0;    //!< nominal window length, to derive its start

        bool valid() const { return (percent >= 0.0); }
        void reset() { percent = -1.0; resetEpoch = -1; }
    };

    Window m_fiveHour;
    Window m_sevenDay;

    //! The prompt cache of the captured session
    struct PromptCache
    {
        bool observed = false;      //!< any response of the session reported cache tokens
        qint64 ttlSecs = 0;         //!< lifetime of the cached prefix, or 0 when unknown
        qint64 expiresEpoch = -1;   //!< unix seconds, or -1 when the cache is cold

        bool valid() const { return observed; }
        void reset() { observed = false; ttlSecs = 0; expiresEpoch = -1; }
    };

    PromptCache m_cache;

    DataSource m_source = SourceNone;
    bool m_stale = false;
    QString m_modelName;
    QString m_capturePath;      //!< the capture file actually in use, empty if none
    qint64 m_capturedAt = -1;   //!< unix seconds, or -1 when unknown

    bool m_hookInstalled = false;   //!< ~/.claude/settings.json points its statusline at us
    bool m_hookForeign = false;     //!< a statusline is configured, but not ours

    QStringList m_candidates;   //!< capture files to look for, most trusted first
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_pollTimer = nullptr;      //!< watcher fallback (atomic renames are easy to miss)
    QTimer *m_countdownTimer = nullptr; //!< drives the reset countdowns

    bool m_probeAvailable = false;      //!< the claude executable was found
    ProbeState m_probeState = ProbeNone;
    QProcess *m_probeProcess = nullptr; //!< the probe in flight, if any
    QByteArray m_probeOutput;           //!< stream-json output of the probe in flight
    QJsonObject m_probeCapture;         //!< last probe result, shaped like a statusline capture

    //! How long a capture stays fresh; Claude Code refreshes its statusline often
    static constexpr qint64 s_captureTTL = 600;
    //! Consumption is "on track" within this many points of the elapsed ratio
    static constexpr double s_paceTolerance = 10.0;
    //! Utilization at which we start warning
    static constexpr double s_nearLimitThreshold = 95.0;

    //! A probe is killed past this delay
    static constexpr int s_probeTimeoutMs = 60 * 1000;

    static constexpr qint64 s_fiveHourSecs = 5 * 60 * 60;
    static constexpr qint64 s_sevenDaySecs = 7 * 24 * 60 * 60;

    // Singleton
    explicit ClaudeMonitor(QObject *parent = nullptr);

    //! Sanitize an official "used_percentage", guarding against Claude Code bug #52326
    static double cleanPercentage(const QJsonValue &value);
    //! Coerce a finite unix epoch (number or ISO 8601 string) to seconds, or -1
    static qint64 cleanEpoch(const QJsonValue &value);

    //! Parse one "rate_limits.*" object into a window
    static bool parseWindow(const QJsonValue &value, qint64 nowEpoch, qint64 durationSecs, Window &window);

    //! Parse the "prompt_cache" object
    static void parsePromptCache(const QJsonValue &value, PromptCache &cache);

    //! Read and parse a single capture file, keeping it only if fresher than the current one
    bool loadCapture(const QString &path, qint64 nowEpoch, qint64 &bestCapturedAt);

    //! Apply a statusline shaped capture, only if fresher than the current one
    bool applyCapture(const QJsonObject &payload, qint64 capturedAt, DataSource source,
                      qint64 nowEpoch, qint64 &bestCapturedAt);

    //! Locate the claude executable, empty when it is not installed
    static QString claudeExecutable();

    /*!
     * \brief Convert a probe "rate_limit_info" into a statusline shaped capture.
     * \return The capture, or an empty object when the event carries no usable window.
     */
    static QJsonObject probeToCapture(const QJsonObject &rateLimitInfo, qint64 capturedAt);

    void probeFinished();
    void setProbeState(ProbeState state);

    //! Elapsed ratio of a window, 0-100, or -1 when unknown
    static double elapsedPercent(const Window &window, qint64 nowEpoch);
    //! Compare consumption against the elapsed ratio
    static Pace pace(const Window &window, qint64 nowEpoch);
    //! Seconds until the window resets, or -1 when unknown
    static int remaining(const Window &window, qint64 nowEpoch);

    void rearmWatcher();
    void rearmCountdown();

    //! The part of the capture path a hook command must contain to count as ours
    QString captureFingerprint() const;

    //! Re-read ~/.claude/settings.json to see whether our statusline hook is registered
    void reloadHookState();

    //! Forget the current capture, as if none was ever found
    void resetCapture();

private slots:
    void reload();

    //! Start or stop all monitoring activity, following the settings
    void applyEnabled();

Q_SIGNALS:
    void enabledChanged();
    void limitsChanged();
    void countdownChanged();
    void hookChanged();
    void probeChanged();

public:
    ~ClaudeMonitor() override;

    static ClaudeMonitor *getInstance();
    static ClaudeMonitor *create(QQmlEngine *, QJSEngine *);

    //! Whether the feature was compiled in; when false this object does nothing at all
    static bool isSupported();
    //! Whether the feature is supported and turned on in the settings
    bool isEnabled() const;

    //! Re-read the capture file right now
    Q_INVOKABLE void refresh() { reload(); }

    /*!
     * \brief The shell command to register as "statusLine" in ~/.claude/settings.json.
     *
     * It captures the session JSON piped by Claude Code into getDefaultCapturePath(),
     * atomically (write to a temporary file, then rename), and echoes the model name
     * back so the status bar is not left empty.
     */
    Q_INVOKABLE QString statuslineCommand() const;

    //! Where our own statusline hook is expected to write
    Q_INVOKABLE QString getDefaultCapturePath() const;

    /*!
     * \brief Register our statusline hook in ~/.claude/settings.json.
     *
     * Every other key in the file is preserved, and the write is atomic. Returns
     * false without touching anything when a foreign statusline is already
     * configured (unless \a overwrite is set), when the file exists but does not
     * parse, or when the write fails.
     *
     * Note this only ever pays off in the Claude Code CLI: the desktop app does
     * not run statusline commands, so no capture is produced there.
     */
    Q_INVOKABLE bool installStatuslineHook(bool overwrite = false);

    //! The settings file the hook is registered in
    QString getSettingsPath() const;

    /*!
     * \brief Read the plan limits through a one-off headless Claude Code request.
     *
     * Runs `claude -p` on Haiku with no tools, no thinking and no session persistence,
     * and reads the limits from its "rate_limit_event". The result competes with the
     * statusline capture, the freshest one wins.
     *
     * \warning The request consumes a little of the plan quota,
     *          and starts a new 5 hour window when none is active.
     */
    Q_INVOKABLE void probe();

    bool isProbeAvailable() const { return m_probeAvailable && isEnabled(); }
    int getProbeState() const { return m_probeState; }

    ////

    bool isAvailable() const { return (m_source != SourceNone); }
    bool isStale() const { return m_stale; }

    //! available + stale collapsed into one value, as relayed over the network
    Capture getCaptureState() const {
        if (m_source == SourceNone) return CaptureNone;
        return m_stale ? CaptureStale : CaptureLive;
    }
    bool isHookInstalled() const { return m_hookInstalled; }
    bool isHookForeign() const { return m_hookForeign; }
    int getSource() const { return m_source; }
    int getStatus() const;
    QString getModelName() const { return m_modelName; }
    QString getCapturePath() const { return m_capturePath; }
    QDateTime getCapturedAt() const;

    bool isFiveHourValid() const { return m_fiveHour.valid(); }
    double getFiveHourPercent() const { return m_fiveHour.percent; }
    QDateTime getFiveHourStart() const;
    QDateTime getFiveHourReset() const;
    int getFiveHourRemaining() const;
    double getFiveHourElapsedPercent() const;
    int getFiveHourPace() const;

    bool isSevenDayValid() const { return m_sevenDay.valid(); }
    double getSevenDayPercent() const { return m_sevenDay.percent; }
    QDateTime getSevenDayStart() const;
    QDateTime getSevenDayReset() const;
    int getSevenDayRemaining() const;
    double getSevenDayElapsedPercent() const;
    int getSevenDayPace() const;

    bool isCacheValid() const { return m_cache.valid(); }
    int getCacheTtl() const { return static_cast<int>(m_cache.ttlSecs); }
    QDateTime getCacheExpiry() const;
    //! Seconds until the prompt cache goes cold, 0 once it is cold, or -1 when unknown
    int getCacheRemaining() const;
};

/* ************************************************************************** */
#endif // CLAUDE_MONITOR_H
