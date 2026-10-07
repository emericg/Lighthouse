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

#if defined(ENABLE_MOUSE_CGEVENT)

#include "mouse_cgevent.h"
#include "local_actions.h"

#import <AppKit/AppKit.h>

#include <QDebug>

/* ************************************************************************** */

//! Pointer position is re-read from the system after this long without posting,
//! so the real mouse can be used in between
static constexpr int s_positionTimeoutMs = 100;

//! A second click this far from the first one starts a new click sequence
static constexpr double s_clickSlop = 4.0;

static constexpr CGMouseButton s_cgButton[3] = {
    kCGMouseButtonLeft, kCGMouseButtonRight, kCGMouseButtonCenter,
};
static constexpr CGEventType s_downType[3] = {
    kCGEventLeftMouseDown, kCGEventRightMouseDown, kCGEventOtherMouseDown,
};
static constexpr CGEventType s_upType[3] = {
    kCGEventLeftMouseUp, kCGEventRightMouseUp, kCGEventOtherMouseUp,
};
static constexpr CGEventType s_dragType[3] = {
    kCGEventLeftMouseDragged, kCGEventRightMouseDragged, kCGEventOtherMouseDragged,
};

/* ************************************************************************** */

Mouse_cgevent::Mouse_cgevent(QObject *parent) : Mouse(parent)
{
    //
}

Mouse_cgevent::~Mouse_cgevent()
{
    // never leave a button stuck down
    for (int b = 0; b < 3; b++)
    {
        if (m_buttons[b]) postButton(b, false);
    }

    if (m_source) CFRelease(m_source);
}

/* ************************************************************************** */

void Mouse_cgevent::setup()
{
    if (!m_source)
    {
        m_source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
        if (!m_source) qWarning() << "CGEventSourceCreate() error";

        // by default, the real mouse and keyboard are frozen for 250 ms after each posted event
        if (m_source) CGEventSourceSetLocalEventsSuppressionInterval(m_source, 0.0);
    }

    if (!CGPreflightPostEventAccess() && !m_permissionRequested)
    {
        // macOS only shows the prompt once, then the user has to go to the settings
        m_permissionRequested = true;
        qWarning() << "Mouse_cgevent: missing the Accessibility permission"
                   << "(System Settings > Privacy & Security > Accessibility)";
        CGRequestPostEventAccess();
    }
}

/* ************************************************************************** */

CGPoint Mouse_cgevent::currentPosition()
{
    // the system position lags behind events still in flight, so trust our own while moving
    if (m_posAge.isValid() && m_posAge.elapsed() < s_positionTimeoutMs) return m_pos;

    CGEventRef ev = CGEventCreate(nullptr);
    if (ev)
    {
        m_pos = CGEventGetLocation(ev);
        CFRelease(ev);
    }

    return m_pos;
}

CGPoint Mouse_cgevent::clampToDisplays(CGPoint target, CGPoint from)
{
    uint32_t count = 0;
    CGDirectDisplayID display;

    if (CGGetDisplaysWithPoint(target, 1, &display, &count) == kCGErrorSuccess && count > 0) return target;

    // off screen: stay on the display we came from
    if (CGGetDisplaysWithPoint(from, 1, &display, &count) != kCGErrorSuccess || count == 0)
    {
        display = CGMainDisplayID();
    }

    const CGRect b = CGDisplayBounds(display);
    target.x = qBound(CGRectGetMinX(b), target.x, CGRectGetMaxX(b) - 1.0);
    target.y = qBound(CGRectGetMinY(b), target.y, CGRectGetMaxY(b) - 1.0);

    return target;
}

void Mouse_cgevent::postMove(CGPoint pos, int dx, int dy)
{
    if (!m_source) setup();

    // while a button is held, motion must be a drag or applications will not follow it
    CGEventType type = kCGEventMouseMoved;
    CGMouseButton button = kCGMouseButtonLeft;
    for (int b = 0; b < 3; b++)
    {
        if (m_buttons[b]) { type = s_dragType[b]; button = s_cgButton[b]; break; }
    }

    CGEventRef ev = CGEventCreateMouseEvent(m_source, type, pos, button);
    if (!ev) return;

    // raw deltas, for applications that ignore the position (games, 3D views)
    CGEventSetIntegerValueField(ev, kCGMouseEventDeltaX, dx);
    CGEventSetIntegerValueField(ev, kCGMouseEventDeltaY, dy);

    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);

    m_pos = pos;
    m_posAge.start();
}

void Mouse_cgevent::postButton(int button, bool pressed)
{
    if (button < 0 || button > 2) return;
    if (!m_source) setup();

    const CGPoint pos = currentPosition();

    if (pressed)
    {
        // the system does not count clicks for synthetic events, applications rely on the click state
        const bool sameSequence = (button == m_clickButton && m_clickAge.isValid() &&
                                   m_clickAge.elapsed() < [NSEvent doubleClickInterval] * 1000.0 &&
                                   qAbs(pos.x - m_clickPos.x) <= s_clickSlop &&
                                   qAbs(pos.y - m_clickPos.y) <= s_clickSlop);

        m_clickCount = sameSequence ? m_clickCount + 1 : 1;
        m_clickButton = button;
        m_clickPos = pos;
        m_clickAge.start();
    }

    CGEventRef ev = CGEventCreateMouseEvent(m_source, pressed ? s_downType[button] : s_upType[button],
                                            pos, s_cgButton[button]);
    if (!ev) return;

    CGEventSetIntegerValueField(ev, kCGMouseEventClickState, m_clickCount);

    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);

    m_buttons[button] = pressed;
}

void Mouse_cgevent::setButtons(int btn_left, int btn_right, int btn_middle)
{
    const bool wanted[3] = { btn_left != 0, btn_right != 0, btn_middle != 0 };

    for (int b = 0; b < 3; b++)
    {
        if (m_buttons[b] != wanted[b]) postButton(b, wanted[b]);
    }
}

/* ************************************************************************** */

void Mouse_cgevent::action(int action_code)
{
    int button = -1;

    if (action_code == LocalActions::ACTION_MOUSE_click_left) button = 0;
    else if (action_code == LocalActions::ACTION_MOUSE_click_right) button = 1;
    else if (action_code == LocalActions::ACTION_MOUSE_click_middle) button = 2;

    if (button < 0) return;

    postButton(button, true);
    postButton(button, false);
}

void Mouse_cgevent::action_abs(int x, int y,
                               int btn_left, int btn_right, int btn_middle)
{
    move_abs(x, y);
    setButtons(btn_left, btn_right, btn_middle);
}

void Mouse_cgevent::action_rel(int dx, int dy,
                               int btn_left, int btn_right, int btn_middle)
{
    if (dx || dy) move_rel(dx, dy);
    setButtons(btn_left, btn_right, btn_middle);
}

/* ************************************************************************** */

void Mouse_cgevent::button(int code, bool pressed)
{
    // same mapping as the uinput version: unknown codes are the primary button
    if (code < 0 || code > 2) code = 0;

    if (m_buttons[code] == pressed) return;
    postButton(code, pressed);
}

void Mouse_cgevent::move_abs(int x, int y)
{
    const CGPoint from = currentPosition();
    const CGPoint to = clampToDisplays(CGPointMake(x, y), from);

    postMove(to, int(to.x - from.x), int(to.y - from.y));
}

void Mouse_cgevent::move_rel(int dx, int dy)
{
    const CGPoint from = currentPosition();
    const CGPoint to = clampToDisplays(CGPointMake(from.x + dx, from.y + dy), from);

    postMove(to, dx, dy);
}

void Mouse_cgevent::scroll(int dx, int dy)
{
    if (!dx && !dy) return;
    if (!m_source) setup();

    // like evdev REL_WHEEL, positive wheel1 is up, but positive wheel2 is left on macOS
    CGEventRef ev = CGEventCreateScrollWheelEvent(m_source, kCGScrollEventUnitLine, 2, dy, -dx);
    if (!ev) return;

    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);
}

/* ************************************************************************** */
#endif // defined(ENABLE_MOUSE_CGEVENT)
