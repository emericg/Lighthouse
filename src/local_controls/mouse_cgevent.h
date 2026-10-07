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

#ifdef ENABLE_MOUSE_CGEVENT
#ifndef MOUSE_CGEVENT_H
#define MOUSE_CGEVENT_H
/* ************************************************************************** */

#include "mouse.h"

#include <QObject>
#include <QElapsedTimer>

#include <CoreGraphics/CoreGraphics.h>

/*!
 * Virtual mouse (macOS Quartz events version)
 *
 * macOS cannot inject relative motion, so the pointer position is tracked here
 * and every motion is posted as an absolute position, clamped to the displays.
 * Dragging and double clicks are synthesized too, as the system only infers
 * them from real hardware.
 *
 * Posting events requires the "Accessibility" permission (Privacy & Security
 * settings), which macOS asks for the first time the mouse is used.
 * Without it, events are silently dropped by the system.
 */
class Mouse_cgevent: public Mouse
{
    Q_OBJECT

    CGEventSourceRef m_source = nullptr;
    bool m_permissionRequested = false;

    CGPoint m_pos = {0, 0};             //!< last position we posted
    QElapsedTimer m_posAge;             //!< since that last post

    bool m_buttons[3] = {};             //!< left, right, middle

    // click counting, for double/triple clicks
    int m_clickButton = -1;
    int m_clickCount = 0;
    CGPoint m_clickPos = {0, 0};
    QElapsedTimer m_clickAge;

    //! Where to start a relative motion from
    CGPoint currentPosition();

    //! Bring a point back onto the closest display
    static CGPoint clampToDisplays(CGPoint target, CGPoint from);

    void postMove(CGPoint pos, int dx, int dy);
    void postButton(int button, bool pressed);
    void setButtons(int btn_left, int btn_right, int btn_middle);

public:
    Mouse_cgevent(QObject *parent = nullptr);
    virtual ~Mouse_cgevent();

    virtual void setup();
    virtual void action(int action_code);

    virtual void action_abs(int x, int y,
                            int btn_left, int btn_right, int btn_middle);
    virtual void action_rel(int dx, int dy,
                            int btn_left, int btn_right, int btn_middle);

    /*!
     * \param code: 0 = left, 1 = right, 2 = middle.
     */
    virtual void button(int code, bool pressed);

    virtual void move_abs(int x, int y);    //!< global display coordinates, in points
    virtual void move_rel(int dx, int dy);
    virtual void scroll(int dx, int dy);    //!< in wheel notches, positive is up / right
};

/* ************************************************************************** */
#endif // MOUSE_CGEVENT_H
#endif // ENABLE_MOUSE_CGEVENT
