TARGET  = Lighthouse

VERSION = 0.6
DEFINES+= APP_NAME=\\\"$$TARGET\\\"
DEFINES+= APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++17
QT     += core bluetooth network sql
QT     += multimedia # to play sound effects on actions
QT     += qml quick quickcontrols2 svg
QT     += widgets # for proper systray support

# Validate Qt version
!versionAtLeast(QT_VERSION, 6.5) : error("You need at least Qt version 6.5 for $${TARGET}")

# Project features #############################################################

# Use Qt Quick compiler
ios | android { CONFIG += qtquickcompiler }

win32 { DEFINES += _USE_MATH_DEFINES }

DEFINES += ENABLE_MBEDTLS
#linux:!android { DEFINES += ENABLE_XTEST }
linux:!android { DEFINES += ENABLE_UINPUT }
linux:!android { DEFINES += ENABLE_MPRIS }

# MobileUI for mobile OS
include(src/thirdparty/MobileUI/MobileUI.pri)

# SingleApplication for desktop OS
include(src/thirdparty/SingleApplication/SingleApplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

# Various utils
include(src/thirdparty/AppUtils/AppUtils.pri)

# QmlRadialBar
include(src/thirdparty/QmlRadialBar/QmlRadialBar.pri)

# Project files ################################################################

SOURCES  += src/main.cpp \
            src/SettingsManager.cpp \
            src/DatabaseManager.cpp \
            src/SystrayManager.cpp \
            src/MenubarManager.cpp \
            src/NotificationManager.cpp \
            src/DeviceManager.cpp \
            src/DeviceManager_advertisement.cpp \
            src/DeviceManager_nearby.cpp \
            src/DeviceFilter.cpp \
            src/device.cpp \
            src/device_beacon.cpp \
            src/device_light.cpp \
            src/devices/device_mipow.cpp \
            src/devices/device_pokeballplus.cpp \
            src/devices/device_pokemongoplus.cpp \
            src/devices/device_ylai003.cpp \
            src/devices/device_ylkg07yl.cpp \
            src/devices/device_ylyk01yl.cpp \
            src/network_controls/network_server.cpp \
            src/network_controls/network_client.cpp \
            src/local_controls/local_controls.cpp \
            src/local_controls/mouse.cpp \
            src/local_controls/keyboard.cpp \
            src/local_controls/gamepad.cpp

HEADERS  += src/SettingsManager.h \
            src/DatabaseManager.h \
            src/MenubarManager.h \
            src/SystrayManager.h \
            src/NotificationManager.h \
            src/DeviceManager.h \
            src/DeviceFilter.h \
            src/device.h \
            src/device_utils.h \
            src/device_beacon.h \
            src/device_light.h \
            src/devices/device_mipow.h \
            src/devices/device_pokeballplus.h \
            src/devices/device_pokemongoplus.h \
            src/devices/device_ylai003.h \
            src/devices/device_ylkg07yl.h \
            src/devices/device_ylyk01yl.h \
            src/network_controls/network_server.h \
            src/network_controls/network_client.h \
            src/local_controls/local_controls.h \
            src/local_controls/local_actions.h \
            src/local_controls/mouse.h \
            src/local_controls/keyboard.h \
            src/local_controls/gamepad.h

INCLUDEPATH += src/ src/thirdparty/

RESOURCES   += qml/qml.qrc \
               qml/components.qrc \
               i18n/i18n.qrc \
               assets/assets.qrc

lupdate_only {
    SOURCES += qml/*.qml qml/*.js \
               qml/components/*.qml qml/components_generic/*.qml qml/components_js/*.js
}

# Build settings ###############################################################

CONFIG(release, debug|release) : DEFINES += NDEBUG QT_NO_DEBUG QT_NO_DEBUG_OUTPUT

# Build artifacts ##############################################################

OBJECTS_DIR = build/$${QT_ARCH}/
MOC_DIR     = build/$${QT_ARCH}/
RCC_DIR     = build/$${QT_ARCH}/
UI_DIR      = build/$${QT_ARCH}/

DESTDIR     = bin/

################################################################################
# Application deployment and installation steps

linux:!android {
    TARGET = $$lower($${TARGET})

    QT += dbus

    # needed for ylkg07yl device
    contains(DEFINES, ENABLE_MBEDTLS) {
        LIBS += -lmbedtls -lmbedx509 -lmbedcrypto
    }

    # needed for keyboard_xtest
    contains(DEFINES, ENABLE_XTEST) {
        SOURCES += src/local_controls/keyboard_xtest.cpp
        HEADERS += src/local_controls/keyboard_xtest.h
        LIBS += -lX11 -lXtst
    }

    SOURCES += src/local_controls/mouse_uinput.cpp \
               src/local_controls/keyboard_uinput.cpp \
               src/local_controls/gamepad_uinput.cpp \
               src/local_controls/mpris_dbus.cpp

    HEADERS += src/local_controls/mouse_uinput.h \
               src/local_controls/keyboard_uinput.h \
               src/local_controls/gamepad_uinput.h \
               src/local_controls/mpris_dbus.h

    # Automatic application packaging # Needs linuxdeployqt installed
    #system(linuxdeployqt $${OUT_PWD}/$${DESTDIR}/ -qmldir=qml/)

    # Application packaging # Needs linuxdeployqt installed
    #deploy.commands = $${OUT_PWD}/$${DESTDIR}/ -qmldir=qml/
    #install.depends = deploy
    #QMAKE_EXTRA_TARGETS += install deploy

    # Installation steps
    isEmpty(PREFIX) { PREFIX = /usr/local }
    target_app.files       += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    target_app.path         = $${PREFIX}/bin/
    target_appentry.files  += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).desktop
    target_appentry.path    = $${PREFIX}/share/applications
    target_appdata.files   += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).appdata.xml
    target_appdata.path     = $${PREFIX}/share/appdata
    target_icon_appimage.files += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).svg
    target_icon_appimage.path   = $${PREFIX}/share/pixmaps/
    target_icon_flatpak.files  += $${OUT_PWD}/assets/linux/$$lower($${TARGET}).svg
    target_icon_flatpak.path    = $${PREFIX}/share/icons/hicolor/scalable/apps/
    INSTALLS += target_app target_appentry target_appdata target_icon_appimage target_icon_flatpak

    # Clean appdir/ and bin/ directories
    #QMAKE_CLEAN += $${OUT_PWD}/$${DESTDIR}/$$lower($${TARGET})
    #QMAKE_CLEAN += $${OUT_PWD}/appdir/
}

macx {
    # Bundle name
    QMAKE_TARGET_BUNDLE_PREFIX = io.emeric
    QMAKE_BUNDLE = lighthouse

    # OS icons
    #ICON = $${PWD}/assets/macos/$$lower($${TARGET}).icns

    # OS infos
    QMAKE_INFO_PLIST = $${PWD}/assets/macos/Info.plist

    # OS entitlement (sandbox and stuff)
    ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
    ENTITLEMENTS.value = $${PWD}/assets/macos/$$lower($${TARGET}).entitlements
    QMAKE_MAC_XCODE_SETTINGS += ENTITLEMENTS

    # Target architecture(s)
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

    # Target OS
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

    # Automatic bundle packaging

    # Deploy step (app bundle packaging)
    deploy.commands = macdeployqt $${OUT_PWD}/$${DESTDIR}/$${TARGET}.app -qmldir=qml/ -appstore-compliant
    install.depends = deploy
    QMAKE_EXTRA_TARGETS += install deploy

    # Installation step (note: app bundle packaging)
    isEmpty(PREFIX) { PREFIX = /usr/local }
    target.files += $${OUT_PWD}/${DESTDIR}/${TARGET}.app
    target.path = $$(HOME)/Applications
    INSTALLS += target

    # Clean step
    QMAKE_DISTCLEAN += -r $${OUT_PWD}/${DESTDIR}/${TARGET}.app
}

win32 {
    # OS icon
    #RC_ICONS = $${PWD}/assets/windows/$$lower($${TARGET}).ico

    # Deploy step
    deploy.commands = $$quote(windeployqt $${OUT_PWD}/$${DESTDIR}/ --qmldir qml/)
    install.depends = deploy
    QMAKE_EXTRA_TARGETS += install deploy

    # Installation step
    # TODO

    # Clean step
    # TODO
}

android {
    # ANDROID_TARGET_ARCH: [x86_64, armeabi-v7a, arm64-v8a]
    #message("ANDROID_TARGET_ARCH: $$ANDROID_TARGET_ARCH")

    # Bundle name
    QMAKE_TARGET_BUNDLE_PREFIX = io.emeric
    QMAKE_BUNDLE = lighthouse

    DISTFILES += $${PWD}/assets/android/AndroidManifest.xml \
                 $${PWD}/assets/android/gradle.properties \
                 $${PWD}/assets/android/build.gradle

    ANDROID_PACKAGE_SOURCE_DIR = $${PWD}/assets/android
}

ios {
    #QMAKE_IOS_DEPLOYMENT_TARGET = 11.0
    #message("QMAKE_IOS_DEPLOYMENT_TARGET: $$QMAKE_IOS_DEPLOYMENT_TARGET")

    CONFIG += no_autoqmake

    # Bundle name
    QMAKE_TARGET_BUNDLE_PREFIX = io.emeric
    QMAKE_BUNDLE = lighthouse

    # OS infos
    QMAKE_INFO_PLIST = $${PWD}/assets/ios/Info.plist
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2 # 1: iPhone / 2: iPad / 1,2: Universal

    # OS icons
    #QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
    #QMAKE_ASSET_CATALOGS = $${PWD}/assets/ios/Images.xcassets

    # iOS launch screen
    #AppLaunchScreen.files += $${PWD}/assets/ios/AppLaunchScreen.storyboard
    #QMAKE_BUNDLE_DATA += AppLaunchScreen

    # iOS developer settings
    exists($${PWD}/assets/ios/ios_signature.pri) {
        # Must contain values for:
        # QMAKE_DEVELOPMENT_TEAM
        # QMAKE_PROVISIONING_PROFILE
        include($${PWD}/assets/ios/ios_signature.pri)
    }
}
