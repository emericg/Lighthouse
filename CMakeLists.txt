cmake_minimum_required(VERSION 3.20)

project(Lighthouse VERSION 0.7 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

string(TIMESTAMP CURRENT_TIMESTAMP "%s" UTC)
set(APP_NAME ${CMAKE_PROJECT_NAME})
set(APP_VERSION ${CMAKE_PROJECT_VERSION})
set(APP_VERSION_CODE ${TIMESTAMP})

################################################################################

message(STATUS "[CMAKE] CMAKE_VERSION               = ${CMAKE_VERSION}")
message(STATUS "[CMAKE] CMAKE_PREFIX_PATH           = ${CMAKE_PREFIX_PATH}")
message(STATUS "[SYSTEM] (HOST) SYSTEM NAME         = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (HOST) SYSTEM VERSION      = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM NAME       = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM VERSION    = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[PROJECT] SOURCE_DIR                = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] BUILD_TYPE                = ${CMAKE_BUILD_TYPE}")
message(STATUS "[PROJECT] TOOLCHAIN_FILE            = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[PROJECT] QT_HOST_PATH              = ${QT_HOST_PATH}")

################################################################################

find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth Network Sql Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Gui Svg Qml Quick QuickControls2 Quick3D)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_standard_project_setup(REQUIRES 6.7)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0003 NEW)

set(SOURCES
    src/main.cpp
    src/SettingsManager.cpp src/SettingsManager.h
    src/DatabaseManager.cpp src/DatabaseManager.h
    src/NotificationManager.cpp src/NotificationManager.h
    src/DeviceManager.cpp src/DeviceManager.h
    src/DeviceManager_advertisement.cpp src/DeviceManager_nearby.cpp
    src/DeviceFilter.cpp src/DeviceFilter.h
    src/device.cpp src/device.h src/device_utils.h
    src/device_beacon.cpp src/device_beacon.h
    src/device_light.cpp src/device_light.h
        src/devices/device_mipow.cpp src/devices/device_mipow.h
        src/devices/device_pokeballplus.cpp src/devices/device_pokeballplus.h
        src/devices/device_pokemongoplus.cpp src/devices/device_pokemongoplus.h
        src/devices/device_ylai003.cpp src/devices/device_ylai003.h
        src/devices/device_ylkg07yl.cpp src/devices/device_ylkg07yl.h
        src/devices/device_ylyk01yl.cpp src/devices/device_ylyk01yl.h
    src/local_controls/local_actions.h
    src/local_controls/local_controls.cpp src/local_controls/local_controls.h
    src/network_controls/network_server.cpp src/network_controls/network_server.h
    src/network_controls/network_client.cpp src/network_controls/network_client.h
    src/crypto/RC4/rc4.cpp
    src/crypto/pgp/aes.c
    src/crypto/pgp/pgp-cert.c
    src/crypto/pgp/secrets.c

    assets/assets.qrc
    thirdparty/IconLibrary/IconLibrary.qrc
)

if(IOS OR ANDROID)
    set(QAPPLICATION_CLASS QGuiApplication)
endif()
if(NOT IOS AND NOT ANDROID)
    set(QAPPLICATION_CLASS QApplication)
    set(SOURCES_DESKTOP
        src/MenubarManager.cpp src/MenubarManager.h
        src/SystrayManager.cpp src/SystrayManager.h
        src/local_controls/mouse.cpp src/local_controls/mouse.h
        src/local_controls/keyboard.cpp src/local_controls/keyboard.h
        src/local_controls/gamepad.cpp src/local_controls/gamepad.h
    )
    if(LINUX)
        set(SOURCES_DESKTOP ${SOURCES_DESKTOP}
            src/local_controls/mouse_uinput.cpp src/local_controls/mouse_uinput.h
            src/local_controls/keyboard_uinput.cpp src/local_controls/keyboard_uinput.h
            src/local_controls/gamepad_uinput.cpp src/local_controls/gamepad_uinput.h
            src/local_controls/mpris_dbus.cpp src/local_controls/mpris_dbus.h
        )
    endif()
endif()

qt_add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${SOURCES_DESKTOP} ${SOURCES_MOBILE})

qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI ${CMAKE_PROJECT_NAME}
    QML_FILES
        qml/MobileApplication.qml
        qml/MobileHeader.qml
        qml/MobileDrawer.qml
        qml/MobileMenu.qml
        qml/MobileExit.qml
        qml/MobilePermissions.qml
        qml/DesktopApplication.qml
        qml/DesktopHeader.qml
        qml/ScreenTutorial.qml
        qml/ScreenDeviceList.qml
        qml/ScreenSettings.qml
        qml/ScreenAbout.qml
        qml/DeviceWidget.qml
        qml/DeviceBeacon.qml
        qml/DevicePBP.qml
        qml/DevicePGP.qml
        qml/DeviceRemote.qml
        qml/DeviceLight.qml
        qml/VirtualInputs.qml
        qml/VirtualInput_touchpad.qml
        qml/VirtualInput_gamepad.qml
        qml/VirtualInput_remote.qml
        qml/VirtualInput_media.qml

        qml/popups/PopupActions.qml
        qml/popups/PopupBeaconKey.qml
        qml/popups/PopupDeleteDevice.qml

        qml/components/SettingsQrReader.qml
        qml/components/SettingsQrGenerator.qml

        qml/components/ActionbarBluetooth.qml
        qml/components/ActionbarSelection.qml
        qml/components/ButtonRowElement.qml
        qml/components/MediaButtonRow.qml
        qml/components/VolumeButtonRow.qml
        qml/components/AnalogPad.qml

        qml/components/ItemKey.qml
        qml/components/RemoteButtonKey.qml
        qml/components/RemoteButtonWidget.qml
        qml/components/SchematicButton.qml
        qml/components/SchematicDimmer.qml
        qml/components/SchematicPokeball.qml
        qml/components/SchematicPokemonGoPlus.qml
        qml/components/SchematicRemote.qml
        qml/components/Pokeball3dView.qml
        qml/components/Pokeball3dModel.qml

        qml/components/ActionMenuFixed.qml
        qml/components/ItemLoadData.qml
        qml/components/ItemNoDevice.qml
        qml/components/ItemNoBluetooth.qml
        qml/components/ItemNoPermissions.qml
        qml/components/SliderGradientLuminosity.qml
        qml/components/SliderGradientColor.qml

        qml/components_js/UtilsActions.js
        qml/components_js/UtilsDeviceSensors.js
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src/)

################################################################################

qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES fr
)
qt_add_translations(${CMAKE_PROJECT_NAME}
    TS_FILE_BASE ${CMAKE_PROJECT_NAME}
    TS_FILE_DIR ${CMAKE_SOURCE_DIR}/i18n/
    RESOURCE_PREFIX "/i18n"
)

################################################################################

# Qt libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Bluetooth
    Qt6::Network
    Qt6::Sql
    Qt6::Gui
    Qt6::Svg
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Quick3D
)

if(IOS OR ANDROID)
    # why
    find_package(Qt6 REQUIRED COMPONENTS Location)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Location)
endif()

if(NOT IOS AND NOT ANDROID)
    # for proper systray and menubar support
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Widgets)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(ENABLE_MBEDTLS "" ON)
    option(ENABLE_XTEST "" OFF)
    option(ENABLE_UINPUT "" ON)
    option(ENABLE_MPRIS "" ON)
    add_definitions(-DENABLE_MBEDTLS)
    add_definitions(-DENABLE_UINPUT)
    add_definitions(-DENABLE_MPRIS)
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC "-lmbedtls -lmbedx509 -lmbedcrypto")
endif()

################################################################################

# AppUtils
set(UTILS_DOCK_ENABLED true)
set(UTILS_WIFI_ENABLED true)
add_subdirectory(thirdparty/AppUtils)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE AppUtils)

# SingleApplication
add_subdirectory(thirdparty/SingleApplication)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SingleApplication)

# MobileUI
add_subdirectory(thirdparty/MobileUI)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE MobileUI)

# ComponentLibrary
add_subdirectory(thirdparty/ComponentLibrary)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ComponentLibraryplugin)

# zxing-cpp
add_subdirectory(thirdparty/zxing-cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ZXing)

# zxing-cpp Qt wrapper
add_definitions(-DENABLE_ZXING)
add_subdirectory(thirdparty/zxing-cpp/wrappers/qt/)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ZXingQt)

################################################################################

## Android
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        #QT_ANDROID_ABIS "armeabi-v7a;arm64-v8a;x86;x86_64"
        #QT_ANDROID_BUILD_ALL_ABIS ON
        QT_ANDROID_MIN_SDK_VERSION 23
        QT_ANDROID_TARGET_SDK_VERSION 35
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets/android/"
    )
endif()

## macOS
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/assets/macos/Info.plist"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "io.emeric.lighthouse"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${CMAKE_PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${CMAKE_PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS              "${CMAKE_SOURCE_DIR}/assets/macos/${CMAKE_PROJECT_NAME}.entitlements"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  AppIcon
    )
endif()

## iOS
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    #set(assets_catalog_path "${CMAKE_SOURCE_DIR}/assets/ios/Assets.xcassets")
    #target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${assets_catalog_path})
    #set_source_files_properties(${assets_catalog_path} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/assets/ios/Info.plist"
        # Qt
        QT_IOS_LAUNCH_SCREEN "${CMAKE_SOURCE_DIR}/assets/ios/LaunchScreen.storyboard"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "io.emeric.lighthouse"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${CMAKE_PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${CMAKE_PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET          "14.0"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY              "1,2"   # iPhone,iPad
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  "AppIcon"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS      "YES"
    )
endif()

## Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

## Release build? Set "no debug" macros
if (CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        NDEBUG
        QT_NO_DEBUG
        QT_NO_DEBUG_OUTPUT
    )
endif()

################################################################################

# Output?
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Output?
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION bin/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

################################################################################
