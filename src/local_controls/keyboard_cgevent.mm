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

#if defined(ENABLE_KEYBOARD_CGEVENT)

#include "keyboard_cgevent.h"
#include "local_actions.h"

#import <AppKit/AppKit.h>
#include <Carbon/Carbon.h> // kVK_* virtual key codes
#include <IOKit/hidsystem/ev_keymap.h> // NX_KEYTYPE_* media keys

#include <QDebug>

/* ************************************************************************** */

Keyboard_cgevent::Keyboard_cgevent(QObject *parent) : Keyboard(parent)
{
    //
}

Keyboard_cgevent::~Keyboard_cgevent()
{
    if (m_source) CFRelease(m_source);
}

/* ************************************************************************** */

void Keyboard_cgevent::setup()
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
        qWarning() << "Keyboard_cgevent: missing the Accessibility permission"
                   << "(System Settings > Privacy & Security > Accessibility)";
        CGRequestPostEventAccess();
    }
}

/* ************************************************************************** */

void Keyboard_cgevent::postKey(CGKeyCode code, CGEventFlags flags, char16_t text)
{
    if (!m_source) setup();

    for (const bool down : {true, false})
    {
        CGEventRef ev = CGEventCreateKeyboardEvent(m_source, code, down);
        if (!ev) return;

        // explicit flags, so held physical modifiers do not leak into the stroke
        CGEventSetFlags(ev, flags);
        if (text)
        {
            const UniChar c = text;
            CGEventKeyboardSetUnicodeString(ev, 1, &c);
        }

        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
    }
}

void Keyboard_cgevent::postMediaKey(int nx_key)
{
    if (!m_source) setup();

    @autoreleasepool
    {
        for (const bool down : {true, false})
        {
            // NX_SUBTYPE_AUX_CONTROL_BUTTONS: key in the high word, key state in the next byte
            const NSInteger data1 = (nx_key << 16) | ((down ? NX_KEYDOWN : NX_KEYUP) << 8);

            NSEvent *ev = [NSEvent otherEventWithType:NSEventTypeSystemDefined
                                             location:NSZeroPoint
                                        modifierFlags:(down ? 0xa00 : 0xb00)
                                            timestamp:0
                                         windowNumber:0
                                              context:nil
                                              subtype:NX_SUBTYPE_AUX_CONTROL_BUTTONS
                                                data1:data1
                                                data2:-1];

            CGEventPost(kCGHIDEventTap, [ev CGEvent]);
        }
    }
}

/* ************************************************************************** */

/*!
 * \brief Open the default application for a URL scheme, or an application by bundle identifier.
 */
static void openApplication(NSString *scheme, NSString *bundleId)
{
    @autoreleasepool
    {
        NSWorkspace *ws = [NSWorkspace sharedWorkspace];
        NSURL *app = nil;

        if (scheme) app = [ws URLForApplicationToOpenURL:[NSURL URLWithString:[scheme stringByAppendingString:@":"]]];
        if (bundleId) app = [ws URLForApplicationWithBundleIdentifier:bundleId];
        if (!app) return;

        [ws openApplicationAtURL:app
                   configuration:[NSWorkspaceOpenConfiguration configuration]
               completionHandler:nil];
    }
}

void Keyboard_cgevent::action(int action_code)
{
    if (action_code == LocalActions::ACTION_KEYBOARD_up) postKey(kVK_UpArrow);
    else if (action_code == LocalActions::ACTION_KEYBOARD_down) postKey(kVK_DownArrow);
    else if (action_code == LocalActions::ACTION_KEYBOARD_left) postKey(kVK_LeftArrow);
    else if (action_code == LocalActions::ACTION_KEYBOARD_right) postKey(kVK_RightArrow);
    else if (action_code == LocalActions::ACTION_KEYBOARD_enter) postKey(kVK_Return);
    else if (action_code == LocalActions::ACTION_KEYBOARD_escape) postKey(kVK_Escape);
    else if (action_code == LocalActions::ACTION_KEYBOARD_backspace) postKey(kVK_Delete);

    // macOS has no dedicated keys for these, so use the system shortcuts
    // (shortcuts carry their character, so they work with any keyboard layout)
    else if (action_code == LocalActions::ACTION_KEYBOARD_computer_lock) postKey(kVK_ANSI_Q, kCGEventFlagMaskControl | kCGEventFlagMaskCommand, u'q');

    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_up) postMediaKey(NX_KEYTYPE_BRIGHTNESS_UP);
    else if (action_code == LocalActions::ACTION_KEYBOARD_monitor_brightness_down) postMediaKey(NX_KEYTYPE_BRIGHTNESS_DOWN);
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_onoff) postMediaKey(NX_KEYTYPE_ILLUMINATION_TOGGLE);
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_up) postMediaKey(NX_KEYTYPE_ILLUMINATION_UP);
    else if (action_code == LocalActions::ACTION_KEYBOARD_keyboard_brightness_down) postMediaKey(NX_KEYTYPE_ILLUMINATION_DOWN);

    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_backward) postKey(kVK_ANSI_LeftBracket, kCGEventFlagMaskCommand, u'[');
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_forward) postKey(kVK_ANSI_RightBracket, kCGEventFlagMaskCommand, u']');
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_refresh) postKey(kVK_ANSI_R, kCGEventFlagMaskCommand, u'r');
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_stop) postKey(kVK_ANSI_Period, kCGEventFlagMaskCommand, u'.');
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_fullscreen) postKey(kVK_ANSI_F, kCGEventFlagMaskControl | kCGEventFlagMaskCommand, u'f');

    // no launcher keys either, so open the applications directly
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_web) openApplication(@"https", nil);
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_mail) openApplication(@"mailto", nil);
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calendar) openApplication(nil, @"com.apple.iCal");
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_calculator) openApplication(nil, @"com.apple.calculator");
    else if (action_code == LocalActions::ACTION_KEYBOARD_desktop_files) openApplication(nil, @"com.apple.finder");

    // media keys go to the "Now Playing" application, and volume keys show the system OSD
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_playpause) postMediaKey(NX_KEYTYPE_PLAY);
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_next) postMediaKey(NX_KEYTYPE_NEXT);
    else if (action_code == LocalActions::ACTION_KEYBOARD_media_prev) postMediaKey(NX_KEYTYPE_PREVIOUS);
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_mute) postMediaKey(NX_KEYTYPE_MUTE);
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_up) postMediaKey(NX_KEYTYPE_SOUND_UP);
    else if (action_code == LocalActions::ACTION_KEYBOARD_volume_down) postMediaKey(NX_KEYTYPE_SOUND_DOWN);

    // computer_sleep, computer_poweroff and media_stop have no key equivalent
}

/* ************************************************************************** */

void Keyboard_cgevent::key(char32_t key_value)
{
    if (key_value == U'\n') { postKey(kVK_Return); return; }
    if (key_value == U'\t') { postKey(kVK_Tab); return; }
    if (key_value < 0x20 || key_value == 0x7f || key_value > 0x10FFFF) return;

    if (!m_source) setup();

    // characters outside the BMP take a UTF-16 surrogate pair
    UniChar text[2];
    UniCharCount length = 1;
    if (key_value > 0xFFFF)
    {
        const char32_t v = key_value - 0x10000;
        text[0] = UniChar(0xD800 + (v >> 10));
        text[1] = UniChar(0xDC00 + (v & 0x3FF));
        length = 2;
    }
    else
    {
        text[0] = UniChar(key_value);
    }

    for (const bool down : {true, false})
    {
        // the key code is irrelevant, the system uses the attached string as the typed text
        CGEventRef ev = CGEventCreateKeyboardEvent(m_source, 0, down);
        if (!ev) return;

        CGEventSetFlags(ev, 0);
        CGEventKeyboardSetUnicodeString(ev, length, text);

        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
    }
}

/* ************************************************************************** */
#endif // defined(ENABLE_KEYBOARD_CGEVENT)
