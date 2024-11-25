// UtilsActions.js
// Version 0

.pragma library

//.import ComponentLibrary 1.0 as ThemeEngine
.import LocalActions 1.0 as LocalActions

/* ************************************************************************** */

function getActionIcon(action) {

    if (action === LocalActions.LocalActions.ACTION_NO_ACTION)
        return "qrc:/IconLibrary/material-icons/duotone/touch_app.svg"

    if (action === LocalActions.LocalActions.ACTION_keys)
        return "qrc:/assets/gfx/icons/keyboard-variant.svg"

    if (action === LocalActions.LocalActions.ACTION_commands)
        return "qrc:/IconLibrary/material-icons/duotone/launch.svg"

    if (action > LocalActions.LocalActions.ACTION_MEDIA_START &&
        action < LocalActions.LocalActions.ACTION_MEDIA_STOP)
        return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"

    if (action > LocalActions.LocalActions.ACTION_KEYBOARD_START &&
        action < LocalActions.LocalActions.ACTION_KEYBOARD_STOP)
        return "qrc:/assets/gfx/icons/keyboard-variant.svg"

    if (action > LocalActions.LocalActions.ACTION_MPRIS_START &&
        action < LocalActions.LocalActions.ACTION_MPRIS_STOP)
        return "qrc:/IconLibrary/material-symbols/media/slideshow.svg"

    return ""
}

function getActionCategory(action) {
    var name = ""

    if (action === LocalActions.LocalActions.ACTION_NO_ACTION) name = qsTr("no action")

    if (action === LocalActions.LocalActions.ACTION_keys)
        name = qsTr("keyboard shortcut")
    if (action === LocalActions.LocalActions.ACTION_commands)
        name = qsTr("launch app")

    if (action > LocalActions.LocalActions.ACTION_KEYBOARD_START &&
        action < LocalActions.LocalActions.ACTION_KEYBOARD_STOP)
        name = qsTr("keyboard action")
    if (action > LocalActions.LocalActions.ACTION_MPRIS_START &&
        action < LocalActions.LocalActions.ACTION_MPRIS_STOP)
        name = qsTr("media action")

    return name
}

function getActionName(action) {
    var name = ""

    if (action === LocalActions.LocalActions.ACTION_NO_ACTION) name = qsTr("no action")

    if (action === LocalActions.LocalActions.ACTION_keys) name = qsTr("keyboard shortcut")
    if (action === LocalActions.LocalActions.ACTION_commands) name = qsTr("launch app")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_computer_lock) name = qsTr("computer lock")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_computer_sleep) name = qsTr("computer sleep")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_computer_poweroff) name = qsTr("computer power off")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_monitor_brightness_up) name = qsTr("monitor_brightness_up")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_monitor_brightness_down) name = qsTr("monitor_brightness_down")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_keyboard_brightness_onoff) name = qsTr("keyboard_brightness_onoff")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_keyboard_brightness_up) name = qsTr("keyboard_brightness_up")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_keyboard_brightness_down) name = qsTr("keyboard_brightness_down")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_backward) name = qsTr("backward")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_forward) name = qsTr("forward")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_refresh) name = qsTr("refresh")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_stop) name = qsTr("stop")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_fullscreen) name = qsTr("fullscreen")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_web) name = qsTr("web browser")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_mail) name = qsTr("mail client")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_calendar) name = qsTr("calendar app")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_calculator) name = qsTr("calculator")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_desktop_files) name = qsTr("file manager")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_media_playpause) name = qsTr("play / pause")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_media_stop) name = qsTr("stop")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_media_next) name = qsTr("next")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_media_prev) name = qsTr("previous")

    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_volume_mute) name = qsTr("mute")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_volume_up) name = qsTr("volume up")
    if (action === LocalActions.LocalActions.ACTION_KEYBOARD_volume_down) name = qsTr("volume down")

    return name
}

/* ************************************************************************** */
