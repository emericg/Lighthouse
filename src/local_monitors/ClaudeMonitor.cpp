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

#include "ClaudeMonitor.h"
#include "SettingsManager.h"

#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlEngine>
#include <QJSEngine>
#include <QDebug>

/* ************************************************************************** */

ClaudeMonitor *ClaudeMonitor::getInstance()
{
    static ClaudeMonitor *instance = new ClaudeMonitor(QCoreApplication::instance());
    return instance;
}

ClaudeMonitor *ClaudeMonitor::create(QQmlEngine *, QJSEngine *)
{
    ClaudeMonitor *instance = getInstance();
    QJSEngine::setObjectOwnership(instance, QJSEngine::CppOwnership);
    return instance;
}

bool ClaudeMonitor::isSupported()
{
#if defined(ENABLE_CLAUDE_MONITORING)
    return true;
#else
    return false;
#endif
}

bool ClaudeMonitor::isEnabled() const
{
    return isSupported() && SettingsManager::getInstance()->getMonitorClaude();
}

ClaudeMonitor::ClaudeMonitor(QObject *parent) : QObject(parent)
{
    m_fiveHour.durationSecs = s_fiveHourSecs;
    m_sevenDay.durationSecs = s_sevenDaySecs;

    // The object always exists, so QML always resolves; but with the feature
    // compiled out nothing is ever wired up, so no timer fires, no file is
    // watched and no disk access happens
    if (!isSupported()) return;

    // Capture files we know about, most trusted first. The Lighthouse hook is
    // the one we document; the claude-monitor tool writes the second one, and
    // reading it too means an existing claude-monitor setup just works.
    m_candidates << getDefaultCapturePath() << QDir::homePath() + "/.claude-monitor/statusline/latest.json";

    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &ClaudeMonitor::reload);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &ClaudeMonitor::reload);

    // The captures are written through an atomic rename, which drops the inode
    // the watcher is holding; poll as a safety net so we cannot get stuck
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(10 * 1000);
    connect(m_pollTimer, &QTimer::timeout, this, &ClaudeMonitor::reload);

    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);
    connect(m_countdownTimer, &QTimer::timeout, this, &ClaudeMonitor::countdownChanged);

#if QT_CONFIG(process)
    m_probeAvailable = !claudeExecutable().isEmpty();
#endif

    connect(SettingsManager::getInstance(), &SettingsManager::monitorClaudeChanged,
            this, &ClaudeMonitor::applyEnabled);
    applyEnabled();
}

ClaudeMonitor::~ClaudeMonitor()
{
#if QT_CONFIG(process)
    // ~QProcess waits for the process, and could deliver finished() to a half destroyed monitor
    if (m_probeProcess)
    {
        m_probeProcess->disconnect(this);
        m_probeProcess->kill();
        m_probeProcess->waitForFinished(1000);
    }
#endif
}

/* ************************************************************************** */
/* ************************************************************************** */

QString ClaudeMonitor::getDefaultCapturePath() const
{
    // Same directory QSettings writes to, so the capture sits next to the settings
    QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (dir.isEmpty()) dir = QDir::homePath() + "/.config";
    dir += "/" + QCoreApplication::organizationName();

    return dir + "/claude_statusline.json";
}

QString ClaudeMonitor::getSettingsPath() const
{
    return QDir::homePath() + "/.claude/settings.json";
}

QString ClaudeMonitor::statuslineCommand() const
{
    const QString path = getDefaultCapturePath();
    const QString dir = QFileInfo(path).absolutePath();

    // Consume the session JSON from stdin and publish it atomically, so that a
    // reader never sees a half written file. The trailing echo is what Claude Code
    // renders in its status bar: it doubles as a confirmation that the hook runs.
    // Users who already have a statusline can chain theirs after the mv instead.
    return QStringLiteral(
               "sh -c 'mkdir -p \"%1\"; t=\"%2.$$.tmp\"; cat > \"$t\"; mv -f \"$t\" \"%2\"; echo Lighthouse'"
               ).arg(dir, path);
}

/* ************************************************************************** */
/* ************************************************************************** */

QString ClaudeMonitor::captureFingerprint() const
{
    // Match on the file name alone. A hook may spell the path out in full, or
    // write it as "~/...", "$HOME/...", or assemble it from shell variables, and
    // then no full path substring exists to look for. The name is ours, so any
    // command mentioning it is writing our capture.
    return QFileInfo(getDefaultCapturePath()).fileName();
}

void ClaudeMonitor::reloadHookState()
{
    const bool wasInstalled = m_hookInstalled;
    const bool wasForeign = m_hookForeign;

    m_hookInstalled = false;
    m_hookForeign = false;

    QFile file(getSettingsPath());
    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        const QByteArray blob = file.readAll();
        file.close();

        const QJsonDocument doc = QJsonDocument::fromJson(blob);
        if (doc.isObject())
        {
            const QJsonValue statusLine = doc.object().value("statusLine");
            if (statusLine.isObject())
            {
                // Any statusline writing to our capture file is ours, however it
                // was written: the user may well have hand rolled a richer one
                const QString command = statusLine.toObject().value("command").toString();
                if (command.contains(captureFingerprint())) m_hookInstalled = true;
                else m_hookForeign = true;
            }
        }
    }

    if (m_hookInstalled != wasInstalled || m_hookForeign != wasForeign)
    {
        Q_EMIT hookChanged();
    }
}

bool ClaudeMonitor::installStatuslineHook(bool overwrite)
{
    if (!isEnabled()) return false;

    const QString path = getSettingsPath();

    QJsonObject settings;
    QFile file(path);
    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly)) return false;
        const QByteArray blob = file.readAll();
        file.close();

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(blob, &error);

        // Never overwrite a settings file we failed to understand !!!
        if (error.error != QJsonParseError::NoError || !doc.isObject()) return false;

        settings = doc.object();

        const QJsonValue statusLine = settings.value("statusLine");
        if (statusLine.isObject() &&
            !statusLine.toObject().value("command").toString().contains(captureFingerprint()) &&
            !overwrite)
        {
            return false;
        }
    }

    QJsonObject hook;
    hook.insert("type", "command");
    hook.insert("command", statuslineCommand());
    hook.insert("refreshInterval", 60);
    settings.insert("statusLine", hook);

    QDir().mkpath(QFileInfo(path).absolutePath());

    // Write through a rename, so a crash or a concurrent read never sees a truncated settings file
    const QString tmp = path + "." + QString::number(QCoreApplication::applicationPid()) + ".tmp";
    QFile out(tmp);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    if (out.write(QJsonDocument(settings).toJson(QJsonDocument::Indented)) < 0)
    {
        out.close();
        QFile::remove(tmp);
        return false;
    }
    out.close();

    QFile::remove(path);
    if (!QFile::rename(tmp, path))
    {
        QFile::remove(tmp);
        return false;
    }

    reloadHookState();
    rearmWatcher();

    return true;
}

/* ************************************************************************** */
/* ************************************************************************** */

double ClaudeMonitor::cleanPercentage(const QJsonValue &value)
{
    if (!value.isDouble()) return -1.0;

    const double pct = value.toDouble();
    if (!std::isfinite(pct) || pct < 0.0) return -1.0;

    // Claude Code bug #52326: used_percentage sometimes carries the resets_at
    // epoch instead of a percentage. Anything past a rounding artifact is junk,
    // and must be reported as unknown rather than rendered as a full bar.
    if (pct > 101.0) return -1.0;
    if (pct > 100.0) return 100.0;

    return pct;
}

qint64 ClaudeMonitor::cleanEpoch(const QJsonValue &value)
{
    if (value.isDouble())
    {
        const double epoch = value.toDouble();
        if (!std::isfinite(epoch) || epoch <= 0.0) return -1;

        return static_cast<qint64>(epoch);
    }

    if (value.isString())
    {
        const QDateTime dt = QDateTime::fromString(value.toString(), Qt::ISODate);
        if (dt.isValid()) return dt.toSecsSinceEpoch();
    }

    return -1;
}

bool ClaudeMonitor::parseWindow(const QJsonValue &value, qint64 nowEpoch,
                                qint64 durationSecs, Window &window)
{
    window.durationSecs = durationSecs;
    window.reset();

    if (!value.isObject()) return false;

    const QJsonObject obj = value.toObject();
    window.resetEpoch = cleanEpoch(obj.value("resets_at"));
    window.percent = cleanPercentage(obj.value("used_percentage"));

    // Past the reset time the window has rolled over: the captured percentage
    // describes an expired window and no longer says anything about the quota
    if (window.resetEpoch > 0 && nowEpoch >= window.resetEpoch)
    {
        window.percent = -1.0;
    }

    return window.valid();
}

void ClaudeMonitor::parsePromptCache(const QJsonValue &value, PromptCache &cache)
{
    cache.reset();

    if (!value.isObject()) return;

    const QJsonObject obj = value.toObject();
    cache.observed = obj.value("caching_observed").toBool();
    if (!cache.observed) return;

    // "5m" or "1h" as of today, any "<number><s|m|h>" is accepted
    const QString ttl = obj.value("ttl").toString().trimmed();
    if (ttl.size() >= 2)
    {
        bool ok = false;
        const qint64 count = ttl.first(ttl.size() - 1).toLongLong(&ok);
        const QChar unit = ttl.back();

        if (ok && count > 0)
        {
            if (unit == 's') cache.ttlSecs = count;
            else if (unit == 'm') cache.ttlSecs = count * 60;
            else if (unit == 'h') cache.ttlSecs = count * 60 * 60;
        }
    }

    // "expires_at" is null when the last response reported no cache tokens, "warm" is false then too
    if (obj.value("warm").toBool()) cache.expiresEpoch = cleanEpoch(obj.value("expires_at"));
}

bool ClaudeMonitor::loadCapture(const QString &path, qint64 nowEpoch, qint64 &bestCapturedAt)
{
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return false;

    const QByteArray blob = file.readAll();
    file.close();

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(blob, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return false;

    const QJsonObject payload = doc.object();

    // claude-monitor stamps its own captures; a raw statusline dump does not, so fall back to the file date
    qint64 capturedAt = cleanEpoch(payload.value("captured_at_epoch"));
    if (capturedAt < 0) capturedAt = QFileInfo(path).lastModified().toSecsSinceEpoch();

    if (!applyCapture(payload, capturedAt, SourceStatusline, nowEpoch, bestCapturedAt)) return false;

    m_capturePath = path;
    return true;
}

bool ClaudeMonitor::applyCapture(const QJsonObject &payload, qint64 capturedAt, DataSource source,
                                 qint64 nowEpoch, qint64 &bestCapturedAt)
{
    // Several captures may coexist (ours, claude-monitor's, a probe); the freshest wins
    if (capturedAt <= bestCapturedAt) return false;

    // A null rate_limits block is a tombstone:
    // claude-monitor writes it when the payload carries no limits (free tier, older Claude Code),
    // precisely so thatstale official data is not served forever. Honor it.
    const QJsonValue limits = payload.value("rate_limits");
    if (!limits.isObject()) return false;

    Window fiveHour, sevenDay;
    const bool gotFiveHour = parseWindow(limits.toObject().value("five_hour"), nowEpoch, s_fiveHourSecs, fiveHour);
    const bool gotSevenDay = parseWindow(limits.toObject().value("seven_day"), nowEpoch, s_sevenDaySecs, sevenDay);
    if (!gotFiveHour && !gotSevenDay) return false;

    bestCapturedAt = capturedAt;

    m_fiveHour = fiveHour;
    m_sevenDay = sevenDay;
    m_capturedAt = capturedAt;
    m_capturePath.clear();
    m_source = source;
    m_stale = ((nowEpoch - capturedAt) > s_captureTTL);

    parsePromptCache(payload.value("prompt_cache"), m_cache);

    const QJsonValue model = payload.value("model");
    m_modelName = model.isObject() ? model.toObject().value("display_name").toString() : QString();

    return true;
}

/* ************************************************************************** */

QString ClaudeMonitor::claudeExecutable()
{
    // A desktop session PATH often misses the per user install locations
    QString exe = QStandardPaths::findExecutable("claude");
    if (exe.isEmpty())
    {
        exe = QStandardPaths::findExecutable("claude", { QDir::homePath() + "/.local/bin",
                                                         QDir::homePath() + "/.claude/local" });
    }

    return exe;
}

QJsonObject ClaudeMonitor::probeToCapture(const QJsonObject &rateLimitInfo, qint64 capturedAt)
{
    // "utilization" is a 0-1 fraction, where the statusline carries a 0-100 "used_percentage"
    const auto toWindow = [](const QJsonObject &window) -> QJsonObject {
        const QJsonValue utilization = window.value("utilization");
        if (!utilization.isDouble()) return QJsonObject();

        QJsonObject out;
        out.insert("used_percentage", utilization.toDouble() * 100.0);
        out.insert("resets_at", window.value("resetsAt"));
        return out;
    };

    QJsonObject limits;
    const QJsonObject windows = rateLimitInfo.value("unifiedWindows").toObject();
    for (const QString &key: { QStringLiteral("five_hour"), QStringLiteral("seven_day") })
    {
        const QJsonObject window = toWindow(windows.value(key).toObject());
        if (!window.isEmpty()) limits.insert(key, window);
    }

    // Without "unifiedWindows", fall back to the single window the event is about
    if (limits.isEmpty())
    {
        const QString type = rateLimitInfo.value("rateLimitType").toString();
        const QJsonObject window = toWindow(rateLimitInfo);
        if ((type == "five_hour" || type == "seven_day") && !window.isEmpty()) limits.insert(type, window);
    }

    if (limits.isEmpty()) return QJsonObject();

    QJsonObject capture;
    capture.insert("captured_at_epoch", capturedAt);
    capture.insert("rate_limits", limits);
    return capture;
}

void ClaudeMonitor::setProbeState(ProbeState state)
{
    if (m_probeState == state) return;

    m_probeState = state;
    Q_EMIT probeChanged();
}

void ClaudeMonitor::probe()
{
#if QT_CONFIG(process)
    if (!isEnabled() || m_probeProcess) return;

    const QString exe = claudeExecutable();
    if (m_probeAvailable != !exe.isEmpty())
    {
        m_probeAvailable = !exe.isEmpty();
        Q_EMIT probeChanged();
    }
    if (exe.isEmpty())
    {
        setProbeState(ProbeFailed);
        return;
    }

    m_probeOutput.clear();
    m_probeProcess = new QProcess(this);
    m_probeProcess->setProgram(exe);
    m_probeProcess->setArguments({ "-p", "Reply with OK.",
                                   "--model", "haiku",
                                   "--tools", "",
                                   "--no-session-persistence",
                                   "--max-turns", "1",
                                   "--output-format", "stream-json", "--verbose" });

    // A neutral directory, so that no project CLAUDE.md gets loaded into the request
    m_probeProcess->setWorkingDirectory(QDir::tempPath());
    m_probeProcess->setStandardInputFile(QProcess::nullDevice());
    m_probeProcess->setStandardErrorFile(QProcess::nullDevice());

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("MAX_THINKING_TOKENS", "0");
    m_probeProcess->setProcessEnvironment(env);

    connect(m_probeProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        m_probeOutput += m_probeProcess->readAllStandardOutput();
    });
    connect(m_probeProcess, &QProcess::finished, this, &ClaudeMonitor::probeFinished);
    connect(m_probeProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) probeFinished(); // finished() never comes then
    });

    // Killing makes finished() fire, which reports the failure
    QProcess *process = m_probeProcess;
    QTimer::singleShot(s_probeTimeoutMs, process, [process]() { process->kill(); });

    setProbeState(ProbeRunning);
    m_probeProcess->start();
#endif
}

void ClaudeMonitor::probeFinished()
{
#if QT_CONFIG(process)
    if (!m_probeProcess) return;

    m_probeOutput += m_probeProcess->readAllStandardOutput();
    m_probeProcess->deleteLater();
    m_probeProcess = nullptr;

    QJsonObject rateLimitInfo;
    bool authFailure = false;

    const QList<QByteArray> lines = m_probeOutput.split('\n');
    for (const QByteArray &line: lines)
    {
        const QJsonObject message = QJsonDocument::fromJson(line).object();
        const QString type = message.value("type").toString();

        if (type == "rate_limit_event")
        {
            rateLimitInfo = message.value("rate_limit_info").toObject();
        }
        else if (type == "result" && message.value("is_error").toBool())
        {
            // There is no error code to go by, only the message
            authFailure = message.value("result").toString().contains("authenticate", Qt::CaseInsensitive);
        }
    }
    m_probeOutput.clear();

    // Limits that came through are worth keeping, even when the request itself then failed
    const QJsonObject capture = probeToCapture(rateLimitInfo, QDateTime::currentSecsSinceEpoch());
    if (capture.isEmpty())
    {
        setProbeState(authFailure ? ProbeAuthExpired : ProbeFailed);
        return;
    }

    m_probeCapture = capture;
    setProbeState(ProbeDone);
    reload();
#endif
}

/* ************************************************************************** */

void ClaudeMonitor::reload()
{
    if (!isEnabled()) return; // refresh() is reachable from QML whatever the build

    const qint64 nowEpoch = QDateTime::currentSecsSinceEpoch();

    const DataSource wasSource = m_source;
    const bool wasStale = m_stale;
    const qint64 wasCapturedAt = m_capturedAt;
    const double wasFiveHour = m_fiveHour.percent;
    const double wasSevenDay = m_sevenDay.percent;
    const qint64 wasFiveHourReset = m_fiveHour.resetEpoch;
    const qint64 wasSevenDayReset = m_sevenDay.resetEpoch;
    const PromptCache wasCache = m_cache;

    resetCapture();

    qint64 bestCapturedAt = -1;
    for (const QString &candidate: std::as_const(m_candidates))
    {
        loadCapture(candidate, nowEpoch, bestCapturedAt);
    }
    if (!m_probeCapture.isEmpty())
    {
        applyCapture(m_probeCapture, cleanEpoch(m_probeCapture.value("captured_at_epoch")),
                     SourceProbe, nowEpoch, bestCapturedAt);
    }

    reloadHookState();
    rearmWatcher();
    rearmCountdown();

    if (m_source != wasSource ||
        m_stale != wasStale ||
        m_capturedAt != wasCapturedAt ||
        !qFuzzyCompare(m_fiveHour.percent, wasFiveHour) ||
        !qFuzzyCompare(m_sevenDay.percent, wasSevenDay) ||
        m_fiveHour.resetEpoch != wasFiveHourReset ||
        m_sevenDay.resetEpoch != wasSevenDayReset ||
        m_cache.observed != wasCache.observed ||
        m_cache.ttlSecs != wasCache.ttlSecs ||
        m_cache.expiresEpoch != wasCache.expiresEpoch)
    {
        Q_EMIT limitsChanged();
        Q_EMIT countdownChanged();
    }
}

void ClaudeMonitor::resetCapture()
{
    m_source = SourceNone;
    m_stale = false;
    m_capturePath.clear();
    m_capturedAt = -1;
    m_modelName.clear();
    m_fiveHour.reset();
    m_sevenDay.reset();
    m_cache.reset();
}

void ClaudeMonitor::applyEnabled()
{
    if (isEnabled())
    {
        m_pollTimer->start();
        reload();
    }
    else
    {
        m_pollTimer->stop();
        m_countdownTimer->stop();

        const QStringList watched = m_watcher->files() + m_watcher->directories();
        if (!watched.isEmpty()) m_watcher->removePaths(watched);

        // a probe in flight is left to finish, reload() ignores it while disabled
        resetCapture();
        m_probeCapture = QJsonObject();

        Q_EMIT limitsChanged();
        Q_EMIT countdownChanged();
    }

    Q_EMIT probeChanged();
    Q_EMIT enabledChanged();
}

void ClaudeMonitor::rearmWatcher()
{
    // Watch both the capture files and their directories:
    // an atomic rename replaces the file, and the watcher would silently stop following it
    QStringList paths;
    QStringList targets = m_candidates;
    targets << getSettingsPath(); // so hookInstalled follows edits made outside the app

    for (const QString &target: std::as_const(targets))
    {
        const QFileInfo info(target);
        if (info.exists()) paths << info.absoluteFilePath();
        if (info.dir().exists()) paths << info.absolutePath();
    }
    paths.removeDuplicates();
    paths.sort();

    QStringList watched = m_watcher->files() + m_watcher->directories();
    watched.sort();
    if (watched == paths) return;

    if (!watched.isEmpty()) m_watcher->removePaths(watched);
    if (!paths.isEmpty()) m_watcher->addPaths(paths);
}

void ClaudeMonitor::rearmCountdown()
{
    // The countdowns only tick while there is a window or a warm cache to count down to
    const bool needed = (m_fiveHour.resetEpoch > 0 || m_sevenDay.resetEpoch > 0 || m_cache.expiresEpoch > 0);

    if (needed && !m_countdownTimer->isActive()) m_countdownTimer->start();
    else if (!needed && m_countdownTimer->isActive()) m_countdownTimer->stop();
}

/* ************************************************************************** */
/* ************************************************************************** */

double ClaudeMonitor::elapsedPercent(const Window &window, qint64 nowEpoch)
{
    if (window.resetEpoch <= 0 || window.durationSecs <= 0) return -1.0;

    const qint64 start = window.resetEpoch - window.durationSecs;
    const double ratio = static_cast<double>(nowEpoch - start) / static_cast<double>(window.durationSecs);

    return qBound(0.0, ratio, 1.0) * 100.0;
}

ClaudeMonitor::Pace ClaudeMonitor::pace(const Window &window, qint64 nowEpoch)
{
    if (!window.valid()) return PaceUnknown;

    const double elapsed = elapsedPercent(window, nowEpoch);
    if (elapsed < 0.0) return PaceUnknown;

    const double delta = window.percent - elapsed;
    if (delta > s_paceTolerance) return PaceSlowDown;
    if (delta < -s_paceTolerance) return PaceSpeedUp;

    return PaceOnTrack;
}

int ClaudeMonitor::remaining(const Window &window, qint64 nowEpoch)
{
    if (window.resetEpoch <= 0) return -1;

    return static_cast<int>(qMax(qint64(0), window.resetEpoch - nowEpoch));
}

/* ************************************************************************** */

int ClaudeMonitor::getStatus() const
{
    // Both windows gate usage, so the worst of the two is the one that matters
    const double worst = qMax(m_fiveHour.percent, m_sevenDay.percent);

    if (worst < 0.0) return StatusUnknown;
    if (worst >= 100.0) return StatusLimitHit;
    if (worst >= s_nearLimitThreshold) return StatusNearLimit;

    return StatusOk;
}

QDateTime ClaudeMonitor::getCapturedAt() const
{
    if (m_capturedAt < 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_capturedAt);
}

/* ************************************************************************** */

QDateTime ClaudeMonitor::getFiveHourStart() const
{
    if (m_fiveHour.resetEpoch <= 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_fiveHour.resetEpoch - m_fiveHour.durationSecs);
}

QDateTime ClaudeMonitor::getFiveHourReset() const
{
    if (m_fiveHour.resetEpoch <= 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_fiveHour.resetEpoch);
}

int ClaudeMonitor::getFiveHourRemaining() const
{
    return remaining(m_fiveHour, QDateTime::currentSecsSinceEpoch());
}

double ClaudeMonitor::getFiveHourElapsedPercent() const
{
    return elapsedPercent(m_fiveHour, QDateTime::currentSecsSinceEpoch());
}

int ClaudeMonitor::getFiveHourPace() const
{
    return pace(m_fiveHour, QDateTime::currentSecsSinceEpoch());
}

/* ************************************************************************** */

QDateTime ClaudeMonitor::getSevenDayStart() const
{
    if (m_sevenDay.resetEpoch <= 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_sevenDay.resetEpoch - m_sevenDay.durationSecs);
}

QDateTime ClaudeMonitor::getSevenDayReset() const
{
    if (m_sevenDay.resetEpoch <= 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_sevenDay.resetEpoch);
}

int ClaudeMonitor::getSevenDayRemaining() const
{
    return remaining(m_sevenDay, QDateTime::currentSecsSinceEpoch());
}

double ClaudeMonitor::getSevenDayElapsedPercent() const
{
    return elapsedPercent(m_sevenDay, QDateTime::currentSecsSinceEpoch());
}

int ClaudeMonitor::getSevenDayPace() const
{
    return pace(m_sevenDay, QDateTime::currentSecsSinceEpoch());
}

/* ************************************************************************** */

QDateTime ClaudeMonitor::getCacheExpiry() const
{
    if (m_cache.expiresEpoch <= 0) return QDateTime();

    return QDateTime::fromSecsSinceEpoch(m_cache.expiresEpoch);
}

int ClaudeMonitor::getCacheRemaining() const
{
    if (!m_cache.valid()) return -1;
    if (m_cache.expiresEpoch <= 0) return 0;

    return static_cast<int>(qMax(qint64(0), m_cache.expiresEpoch - QDateTime::currentSecsSinceEpoch()));
}

/* ************************************************************************** */
