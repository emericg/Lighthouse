cmake_minimum_required(VERSION 3.20)

project(Lighthouse VERSION 0.7 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(APP_NAME ${PROJECT_NAME})
set(APP_VERSION ${CMAKE_PROJECT_VERSION})

################################################################################

option(ENABLE_MBEDTLS "mbedTLS for PoGo+" ON)
option(ENABLE_MEDIA_MPRIS "MPRIS media controller" ON)
option(ENABLE_VOLUME_PIPEWIRE "Pipewire volume controller" ON)
option(ENABLE_VOLUME_PULSEAUDIO "Pulseaudio volume controller" OFF)
option(ENABLE_KEYBOARD_XTEST "Virtual keyboard using XTEST" OFF)
option(ENABLE_KEYBOARD_UINPUT "Virtual keyboard using UInput" ON)

################################################################################

message(STATUS ">>> BUILDING ${PROJECT_NAME} v${PROJECT_VERSION} >>>")
message(STATUS "[CMAKE] CMAKE_VERSION               = ${CMAKE_VERSION}")
message(STATUS "[CMAKE] CMAKE_PREFIX_PATH           = ${CMAKE_PREFIX_PATH}")
message(STATUS "[BUILD] QT_HOST_PATH                = ${QT_HOST_PATH}")
message(STATUS "[BUILD] TOOLCHAIN_FILE              = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[BUILD] BUILD_TYPE                  = ${CMAKE_BUILD_TYPE}")
message(STATUS "[SYSTEM] (HOST) SYSTEM NAME         = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (HOST) SYSTEM VERSION      = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (HOST) SYSTEM PROCESSOR    = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM NAME       = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM VERSION    = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM PROCESSOR  = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_NAME    = ${CMAKE_PROJECT_NAME}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_VERSION = ${CMAKE_PROJECT_VERSION}")
message(STATUS "[TOP PROJECT] CMAKE_SOURCE_DIR      = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] PROJECT_NAME              = ${PROJECT_NAME}")
message(STATUS "[PROJECT] PROJECT_VERSION           = ${PROJECT_VERSION}")
message(STATUS "[PROJECT] PROJECT_SOURCE_DIR        = ${PROJECT_SOURCE_DIR}")
message(STATUS "[PROJECT] CMAKE_CURRENT_SOURCE_DIR  = ${CMAKE_CURRENT_SOURCE_DIR}")

################################################################################

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Qml Quick QuickControls2)
find_package(Qt6 REQUIRED COMPONENTS Sql Bluetooth Network Multimedia Quick3D)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_standard_project_setup(REQUIRES 6.8
                          I18N_SOURCE_LANGUAGE en
                          I18N_TRANSLATED_LANGUAGES en)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0003 NEW)
qt_policy(SET QTP0004 NEW)

# Set app desktop icons
#if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
#    set(app_icon_windows "${PROJECT_SOURCE_DIR}/assets/windows/${PROJECT_NAME}.rc")
#elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#    set(app_icon_macos "${PROJECT_SOURCE_DIR}/assets/macos/${PROJECT_NAME}.icns")
#endif()

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

    ${app_icon_macos}
    ${app_icon_windows}
    assets/assets.qrc
    thirdparty/IconLibrary/IconLibrary_material.qrc
)

if(IOS OR ANDROID)
    set(QAPPLICATION_CLASS QGuiApplication)
endif()

if(NOT IOS AND NOT ANDROID)
    set(QAPPLICATION_CLASS QApplication)
    set(SOURCES_DESKTOP
        src/MenubarManager.cpp src/MenubarManager.h
        src/SystrayManager.cpp src/SystrayManager.h
        src/local_controls/volume.h
        src/local_controls/volume_keyboard.cpp src/local_controls/volume_keyboard.h
        src/local_controls/mouse.h
        src/local_controls/keyboard.h
        src/local_controls/gamepad.h
    )
    if(LINUX)
        set(SOURCES_DESKTOP ${SOURCES_DESKTOP}
            src/local_controls/mouse_uinput.cpp src/local_controls/mouse_uinput.h
            src/local_controls/keyboard_uinput.cpp src/local_controls/keyboard_uinput.h
            src/local_controls/gamepad_uinput.cpp src/local_controls/gamepad_uinput.h
            src/local_controls/mpris_dbus.cpp src/local_controls/mpris_dbus.h
        )
        if(ENABLE_VOLUME_PIPEWIRE)
            set(SOURCES_DESKTOP ${SOURCES_DESKTOP}
                src/local_controls/volume_pipewire.cpp src/local_controls/volume_pipewire.h
            )
        endif()
        if(ENABLE_VOLUME_PULSEAUDIO)
            set(SOURCES_DESKTOP ${SOURCES_DESKTOP}
                src/local_controls/volume_pulseaudio.cpp src/local_controls/volume_pulseaudio.h
            )
        endif()
    endif()
endif()

qt_add_executable(${PROJECT_NAME} ${SOURCES} ${SOURCES_DESKTOP} ${SOURCES_MOBILE})

qt_add_ios_ffmpeg_libraries(${PROJECT_NAME})

qt_add_qml_module(${PROJECT_NAME}
    URI ${PROJECT_NAME}
    NO_PLUGIN
    QML_FILES
        qml/MobileApplication.qml
        qml/MobileHeader.qml
        qml/MobileDrawer.qml
        qml/MobilePermissions.qml
        qml/MobileMenu.qml
        qml/MobileExit.qml

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

qt_add_translations(${PROJECT_NAME}
    TS_FILE_BASE ${PROJECT_NAME}
    TS_FILE_DIR ${PROJECT_SOURCE_DIR}/i18n/
    RESOURCE_PREFIX "/i18n"
)

################################################################################

# "Other Files" macro
function(add_other_files target)
    foreach(file IN LISTS ARGN)
        target_sources(${target} PRIVATE ${file})
        source_group("Other Files" FILES ${file})
    endforeach()
endfunction()

add_other_files(${PROJECT_NAME}
    README.md LICENSE.md
    .gitignore
)

################################################################################

if(IOS OR ANDROID)
    # why
    find_package(Qt6 REQUIRED COMPONENTS Location)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Location)
endif()

if(NOT IOS AND NOT ANDROID)
    # for proper systray and menubar support
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DENABLE_MBEDTLS)
    add_definitions(-DENABLE_KEYBOARD_UINPUT)
    add_definitions(-DENABLE_MEDIA_MPRIS)
    if(ENABLE_VOLUME_PIPEWIRE)
        add_definitions(-DENABLE_VOLUME_PIPEWIRE)
    elseif(ENABLE_VOLUME_PULSEAUDIO)
        add_definitions(-DENABLE_VOLUME_PULSEAUDIO)
    endif()

    # D-Bus is used by the MPRIS media controller and the PulseAudio volume controller
    find_package(Qt6 REQUIRED COMPONENTS DBus)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::DBus)

    target_link_libraries(${PROJECT_NAME} PUBLIC "-lmbedtls -lmbedx509 -lmbedcrypto")
endif()

################################################################################

# AppUtils
set(UTILS_DOCK_ENABLED true)
set(UTILS_WIFI_ENABLED true)
add_subdirectory(thirdparty/AppUtils)
target_link_libraries(${PROJECT_NAME} PRIVATE AppUtils)

# SingleApplication
add_subdirectory(thirdparty/SingleApplication)
target_link_libraries(${PROJECT_NAME} PRIVATE SingleApplication)

# MobileUI
add_subdirectory(thirdparty/MobileUI)
target_link_libraries(${PROJECT_NAME} PRIVATE MobileUI MobileUI_plugin)

# ComponentLibrary
add_subdirectory(thirdparty/ComponentLibrary)
target_link_libraries(${PROJECT_NAME} PRIVATE ComponentLibrary_plugin)

# zxing-cpp
add_subdirectory(thirdparty/zxing-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ZXing)

# zxing-cpp Qt wrapper
add_definitions(-DENABLE_ZXING)
add_subdirectory(thirdparty/zxing-cpp/wrappers/qt/)
target_link_libraries(${PROJECT_NAME} PRIVATE ZXingQt)

# Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Bluetooth
    Qt6::Network
    Qt6::Multimedia
    Qt6::Quick3D
)

target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty/)

# Qt official hacks...
set(QML_IMPORT_PATH
    "${CMAKE_BINARY_DIR}/thirdparty/"
    "${CMAKE_BINARY_DIR}/thirdparty/MobileUI"
    "${CMAKE_BINARY_DIR}/thirdparty/ComponentLibrary"
    CACHE STRING "QML Modules import paths" FORCE)

# Qt official hacks...
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

################################################################################

## Release build? Set "no debug" macros
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:QT_NO_DEBUG>
    $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

################################################################################

## Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # System install rules
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/)
    install(FILES ${PROJECT_SOURCE_DIR}/assets/linux/${PROJECT_NAME}.desktop DESTINATION share/applications/)
    #install(FILES ${PROJECT_SOURCE_DIR}/assets/linux/${PROJECT_NAME}.appdata.xml DESTINATION share/appdata/)
    #install(FILES ${PROJECT_SOURCE_DIR}/assets/linux/${PROJECT_NAME}.svg DESTINATION share/pixmaps/)
    #install(FILES ${PROJECT_SOURCE_DIR}/assets/linux/${PROJECT_NAME}.svg DESTINATION share/icons/hicolor/scalable/apps/)
    #install(FILES ${PROJECT_SOURCE_DIR}/LICENSE.md DESTINATION share/licenses/${PROJECT_NAME}/)
endif()

## macOS
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")

    #set(app_icon_macos "${PROJECT_SOURCE_DIR}/assets/macos/${PROJECT_NAME}.icns")
    #set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/assets/macos/Info.plist"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "io.emeric.lighthouse"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS             "${PROJECT_SOURCE_DIR}/assets/macos/${PROJECT_NAME}.entitlements"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  AppIcon
    )
endif()

################################################################################

## Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

## Android
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        #QT_ANDROID_ABIS "armeabi-v7a;arm64-v8a;x86;x86_64"
        #QT_ANDROID_BUILD_ALL_ABIS ON
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 36
        QT_ANDROID_PACKAGE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/assets/android/"
    )
endif()

## iOS
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    #set(app_icon_ios "${PROJECT_SOURCE_DIR}/assets/ios/Assets.xcassets")
    #target_sources(${PROJECT_NAME} PRIVATE ${app_icon_ios})
    #set_source_files_properties(${app_icon_ios} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/assets/ios/Info.plist"
        # Qt
        QT_IOS_LAUNCH_SCREEN "${PROJECT_SOURCE_DIR}/assets/ios/LaunchScreen.storyboard"
        # New Xcode attributes (???)
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER           "io.emeric.lighthouse"
        XCODE_ATTRIBUTE_PRODUCT_NAME                        ${PROJECT_NAME}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION             ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_MARKETING_VERSION                   ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET          "16.0"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY              "1,2"   # iPhone,iPad
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  "AppIcon"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS      "YES"
    )
endif()

################################################################################

# Output (build)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Output (install)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

################################################################################
