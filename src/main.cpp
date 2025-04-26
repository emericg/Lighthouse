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
 * \date      2018
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "DatabaseManager.h"
#include "SettingsManager.h"
#include "SystrayManager.h"
#include "MenubarManager.h"
#include "NotificationManager.h"
#include "DeviceManager.h"

#include "utils_app.h"
#include "utils_wifi.h"
#include "utils_screen.h"
#include "utils_language.h"
#if defined(Q_OS_MACOS)
#include "utils_os_macos_dock.h"
#endif

#include "network_controls/network_server.h"
#include "network_controls/network_client.h"
#include "local_controls/local_controls.h"
#include "local_controls/local_actions.h"
#include "local_controls/mpris_dbus.h"

#include <MobileUI>
#include <SingleApplication>
#if defined(ENABLE_ZXING)
#include <ZXingQt>
#endif

#include <QtGlobal>
#include <QLibraryInfo>
#include <QVersionNumber>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSurfaceFormat>

/* ************************************************************************** */

int main(int argc, char *argv[])
{
    // Arguments parsing ///////////////////////////////////////////////////////

    bool start_minimized = false;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i])
        {
            //qDebug() << "> arg >" << argv[i];

            if (QString::fromLocal8Bit(argv[i]) == "--start-minimized")
                start_minimized = true;
        }
    }

    // Hacks ///////////////////////////////////////////////////////////////////

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // NVIDIA suspend&resume hack
    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);
#endif

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    // Qt 6.6+ mouse wheel hack
    qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "2500");
#endif

    // GUI application /////////////////////////////////////////////////////////

    SingleApplication app(argc, argv, false);

    // Application name
    app.setApplicationName("Lighthouse");
    app.setApplicationDisplayName("Lighthouse");
    app.setOrganizationName("Lighthouse");
    app.setOrganizationDomain("Lighthouse");

    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    DatabaseManager *db = DatabaseManager::getInstance();
    NotificationManager *nm = NotificationManager::getInstance();
    DeviceManager *dm = new DeviceManager;
    if (!sm || !db || !nm || !dm)
    {
        qWarning() << "Cannot init Lighthouse components!";
        return EXIT_FAILURE;
    }

    NetworkServer *networkServer = nullptr;
    NetworkClient *networkClient = nullptr;

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) // desktop section
    app.setApplicationDisplayName("Lighthouse");
    app.setWindowIcon(QIcon(":/assets/gfx/logos/logo.svg"));

    SystrayManager *st = SystrayManager::getInstance();
    MenubarManager *mb = MenubarManager::getInstance();
    LocalControls *localControls = LocalControls::getInstance();

#if defined(ENABLE_MPRIS)
    MprisController *mprisControls = MprisController::getInstance();
    mprisControls->select_player();
#endif
#endif // desktop section

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) // mobile section
    networkClient = new NetworkClient();
    networkClient->connectToServer();
#else
    networkServer = new NetworkServer();

    dm->listenDevices_start();
#endif // mobile section

    // Init generic utils
    UtilsApp *utilsApp = UtilsApp::getInstance();
    UtilsWiFi *utilsWiFi = UtilsWiFi ::getInstance();
    UtilsScreen *utilsScreen = UtilsScreen::getInstance();
    UtilsLanguage *utilsLanguage = UtilsLanguage::getInstance();
    if (!utilsScreen || !utilsApp || !utilsLanguage)
    {
        qWarning() << "Cannot init Lighthouse utils!";
        return EXIT_FAILURE;
    }

    // Translate the application
    utilsLanguage->loadLanguage(sm->getAppLanguage());

    MobileUI::registerQML();
    DeviceUtils::registerQML();
    LocalActions::registerQML();

    // Then we start the UI
    QQmlApplicationEngine engine;
    engine.addImportPath(":/Lighthouse");
    engine.addImportPath(":/ComponentLibrary");

    QQmlContext *engine_context = engine.rootContext();
    engine_context->setContextProperty("settingsManager", sm);
    engine_context->setContextProperty("deviceManager", dm);

    engine_context->setContextProperty("utilsApp", utilsApp);
    engine_context->setContextProperty("utilsWiFi", utilsWiFi);
    engine_context->setContextProperty("utilsScreen", utilsScreen);
    engine_context->setContextProperty("utilsLanguage", utilsLanguage);

    engine_context->setContextProperty("networkServer", networkServer);
    engine_context->setContextProperty("networkClient", networkClient);
    engine_context->setContextProperty("networkControls", networkClient);

#if defined(ENABLE_ZXING)
    // Barcode (zxing-cpp Qt wrapper)
    ZXingQt::registerQMLTypes();
    ZXingQt::registerQMLImageProvider(engine);
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(FORCE_MOBILE_UI)
    // Load the main view
    engine.loadFromModule("Lighthouse", "MobileApplication");
#else
    // desktop specific stuff
    engine_context->setContextProperty("startMinimized", (start_minimized || sm->getMinimized()));
    engine_context->setContextProperty("systrayManager", st);
    engine_context->setContextProperty("menubarManager", mb);
    engine_context->setContextProperty("localControls", localControls);
#if defined(ENABLE_MPRIS)
    engine_context->setContextProperty("mprisControls", mprisControls);
#endif

    // Load the main view
    engine.loadFromModule("Lighthouse", "DesktopApplication");
#endif

    if (engine.rootObjects().isEmpty())
    {
        qWarning() << "Cannot init QmlApplicationEngine!";
        return EXIT_FAILURE;
    }

    // For i18n retranslate
    utilsLanguage->setQmlEngine(&engine);

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) // desktop section

    // QQuickWindow must be valid at this point
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));

    // React to secondary instances
    QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::show);
    QObject::connect(&app, &SingleApplication::instanceStarted, window, &QQuickWindow::raise);

    // Systray?
    st->setupSystray(window);
    if (sm->getSysTray()) st->installSystray();

    // Menu bar
    mb->setupMenubar(window, dm);

#if defined(Q_OS_MACOS)
    // dock
    MacOSDockHandler *dockIconHandler = MacOSDockHandler::getInstance();
    dockIconHandler->setupDock(window);
    engine_context->setContextProperty("utilsDock", dockIconHandler);
#endif

#endif // desktop section

#if defined(Q_OS_ANDROID)
    QNativeInterface::QAndroidApplication::hideSplashScreen(333);
#endif

    return app.exec();
}

/* ************************************************************************** */
