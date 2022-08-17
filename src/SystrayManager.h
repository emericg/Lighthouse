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

#ifndef SYSTRAY_MANAGER_H
#define SYSTRAY_MANAGER_H
/* ************************************************************************** */

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QApplication)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)

/* ************************************************************************** */

/*!
 * \brief The SystrayManager class
 */
class SystrayManager: public QObject
{
    Q_OBJECT

    QQuickWindow *m_saved_view = nullptr;
    QApplication *m_saved_app = nullptr;

    QSystemTrayIcon *m_sysTray = nullptr;
    QIcon *m_sysTrayIcon = nullptr;
    QMenu *m_sysTrayMenu = nullptr;
    QAction *m_actionShow = nullptr;
    QAction *m_actionDeviceList = nullptr;
    QAction *m_actionSettings = nullptr;
    QAction *m_actionExit = nullptr;

    QTimer m_retryTimer;
    int retryCount = 6;

    static SystrayManager *instance;

    SystrayManager();
    ~SystrayManager();

    void initSystray();

signals:
    void showClicked();
    void hideClicked();
    void sensorsClicked();
    void settingsClicked();
    void quitClicked();

public:
    static SystrayManager *getInstance();
    void setupSystray(QApplication *app, QQuickWindow *view);

public slots:
    bool installSystray();
    void REinstallSystray();
    void removeSystray();
    void sendNotification(QString &text);

private slots:
    void trayClicked(QSystemTrayIcon::ActivationReason r);
    void showHideButton();
    void sensorsButton();
    void settingsButton();

    void visibilityChanged();
    void aboutToBeDestroyed();
};

/* ************************************************************************** */
#endif // SYSTRAY_MANAGER_H
