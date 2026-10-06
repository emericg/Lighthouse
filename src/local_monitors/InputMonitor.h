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

#ifndef INPUT_MONITOR_H
#define INPUT_MONITOR_H
/* ************************************************************************** */

#include <QtQml/qqmlregistration.h>

#include <QObject>

class QQmlEngine;
class QJSEngine;
class QTimer;

/* ************************************************************************** */

/*!
 * \brief Platform specific source of input activity, used by InputMonitor.
 *
 * A backend only reports that the user is active, never which key or button.
 * Event based backends emit keyPressed() and let InputMonitor debounce it,
 * state based backends emit activityChanged() and handle their own timeout.
 */
class InputMonitorBackend: public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void keyPressed();                  //!< one or more key presses happened since the last emission
    void activityChanged(bool active);  //!< the user became active, or idle
    void availableChanged();

public:
    InputMonitorBackend(QObject *parent = nullptr) : QObject(parent) { }
    virtual ~InputMonitorBackend() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    //! True while the backend is actually able to report activity
    virtual bool isAvailable() const = 0;
};

/* ************************************************************************** */

/*!
 * \brief The InputMonitor class
 *
 * Turns the raw keyboard activity reported by a platform backend into a
 * debounced "typing" state, suitable to be relayed to remote clients.
 *
 * Only that boolean ever leaves this class: no key codes, and no per key
 * timing either, as inter keystroke delays are enough to infer what is typed.
 *
 * Depending on the backend, "typing" means keyboard activity only (evdev),
 * or any user input, pointer included (ext-idle-notify-v1).
 *
 * The monitor is idle until setActive(true) is called, so that keyboards are
 * only opened while someone is actually there to watch the result.
 */
class InputMonitor: public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Backend backend READ getBackend CONSTANT)
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)
    Q_PROPERTY(bool typing READ isTyping NOTIFY typingChanged)

public:
    enum Backend
    {
        BackendNone = 0,    //!< no backend compiled in for this platform
        BackendEvdev,       //!< keyboard activity only, needs read access to the input devices
        BackendIdleNotify,  //!< any input activity, as reported by the Wayland compositor
    };
    Q_ENUM(Backend)

private:
    InputMonitorBackend *m_backend = nullptr;   //!< nullptr when no backend is compiled in

    QTimer *m_idleTimer = nullptr;

    bool m_active = false;
    bool m_typing = false;

    //! Typing stops being reported after this long without a key press
    static constexpr int s_idleTimeoutMs = 600;

    // Singleton
    explicit InputMonitor(QObject *parent = nullptr);

    void setTyping(bool typing);

private slots:
    void onKeyPressed();

Q_SIGNALS:
    void availableChanged();
    void typingChanged();

public:
    ~InputMonitor() override = default;

    static InputMonitor *getInstance();
    static InputMonitor *create(QQmlEngine *, QJSEngine *);

    static Backend getBackend();
    bool isAvailable() const;
    bool isTyping() const { return m_typing; }

    /*!
     * \brief Start or stop listening to the keyboards.
     * \param active: true to open the input devices, false to release them all.
     */
    void setActive(bool active);
};

/* ************************************************************************** */
#endif // INPUT_MONITOR_H
