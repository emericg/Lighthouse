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

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H
/* ************************************************************************** */

#include "local_controls/local_actions.h"

#include <QStringView>
#include <QLatin1StringView>

/* ************************************************************************** */

/*!
 * \brief Version of the wire protocol, announced by the server in its "welcome:<version>" frame.
 *
 * Every message travels as a QDataStream serialized QByteArray, holding UTF-8 text,
 * except the "media:art:<mime>;" message whose payload is the raw image bytes.
 * Bump it on any incompatible change, the client refuses a server with a different version.
 */
inline constexpr int kNetworkProtocolVersion = 3;

/* ************************************************************************** */

/*!
 * \brief A LocalActions code a client can trigger on the server, and its wire name.
 *
 * The client sends "press:<name>", the server maps it back to the action code.
 */
struct NetworkPressAction
{
    int action;
    const char *name;
};

inline constexpr NetworkPressAction kNetworkPressActions[] = {
    { LocalActions::ACTION_KEYBOARD_computer_lock,      "lock" },
    { LocalActions::ACTION_KEYBOARD_computer_sleep,     "sleep" },
    { LocalActions::ACTION_KEYBOARD_computer_poweroff,  "poweroff" },

    { LocalActions::ACTION_KEYBOARD_media_playpause,    "playpause" },
    { LocalActions::ACTION_KEYBOARD_media_stop,         "stop" },
    { LocalActions::ACTION_KEYBOARD_media_next,         "next" },
    { LocalActions::ACTION_KEYBOARD_media_prev,         "prev" },

    { LocalActions::ACTION_KEYBOARD_volume_mute,        "mute" },
    { LocalActions::ACTION_KEYBOARD_volume_up,          "volumeup" },
    { LocalActions::ACTION_KEYBOARD_volume_down,        "volumedown" },

    { LocalActions::ACTION_KEYBOARD_up,                 "up" },
    { LocalActions::ACTION_KEYBOARD_down,               "down" },
    { LocalActions::ACTION_KEYBOARD_left,               "left" },
    { LocalActions::ACTION_KEYBOARD_right,              "right" },
    { LocalActions::ACTION_KEYBOARD_enter,              "enter" },
    { LocalActions::ACTION_KEYBOARD_escape,             "escape" },
    { LocalActions::ACTION_KEYBOARD_backspace,          "backspace" },
};

/*!
 * \param action A LocalActions code.
 * \return The wire name of that action, or nullptr if it cannot be sent over the network.
 */
inline const char *networkPressActionName(int action)
{
    for (const NetworkPressAction &a : kNetworkPressActions)
    {
        if (a.action == action) return a.name;
    }
    return nullptr;
}

/*!
 * \param name The wire name of an action (the part after "press:").
 * \return The matching LocalActions code, or -1 if unknown.
 */
inline int networkPressActionFromName(QStringView name)
{
    for (const NetworkPressAction &a : kNetworkPressActions)
    {
        if (name == QLatin1StringView(a.name)) return a.action;
    }
    return -1;
}

/* ************************************************************************** */
#endif // NETWORK_PROTOCOL_H
