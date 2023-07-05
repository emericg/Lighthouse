/*!
 * Copyright (c) 2016 J-P Nurmi
 * Copyright (c) 2022 Emeric Grange
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MOBILEUI_H
#define MOBILEUI_H
/* ************************************************************************** */

#include <QObject>
#include <QColor>

/* ************************************************************************** */

class MobileUI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable CONSTANT)

    Q_PROPERTY(Theme deviceTheme READ getDeviceTheme NOTIFY devicethemeUpdated)

    Q_PROPERTY(QColor statusbarColor READ getStatusbarColor WRITE setStatusbarColor NOTIFY statusbarUpdated)
    Q_PROPERTY(Theme statusbarTheme READ getStatusbarTheme WRITE setStatusbarTheme NOTIFY statusbarUpdated)
    Q_PROPERTY(int statusbarHeight READ getStatusbarHeight NOTIFY safeAreaUpdated)

    Q_PROPERTY(QColor navbarColor READ getNavbarColor WRITE setNavbarColor NOTIFY navbarUpdated)
    Q_PROPERTY(Theme navbarTheme READ getNavbarTheme WRITE setNavbarTheme NOTIFY navbarUpdated)
    Q_PROPERTY(int navbarHeight READ getNavbarHeight NOTIFY navbarUpdated)

    Q_PROPERTY(int safeAreaTop READ getSafeAreaTop NOTIFY safeAreaUpdated)
    Q_PROPERTY(int safeAreaLeft READ getSafeAreaLeft NOTIFY safeAreaUpdated)
    Q_PROPERTY(int safeAreaRight READ getSafeAreaRight NOTIFY safeAreaUpdated)
    Q_PROPERTY(int safeAreaBottom READ getSafeAreaBottom NOTIFY safeAreaUpdated)

    Q_PROPERTY(bool screenAlwaysOn READ getScreenKeepOn WRITE setScreenKeepOn)

Q_SIGNALS:
    void devicethemeUpdated();
    void statusbarUpdated();
    void navbarUpdated();
    void safeAreaUpdated();

public:
    explicit MobileUI(QObject *parent = nullptr) : QObject(parent) {}

    static void registerQML();

    static bool isAvailable();

    enum Theme {
        Light,  //!< Light application theme, usually light background and dark texts.
        Dark    //!< Dark application theme, usually dark background and light texts.
    };
    Q_ENUM(Theme)

    static Theme getDeviceTheme();

    // Status bar
    static QColor getStatusbarColor();
    static void setStatusbarColor(const QColor &color);

    static Theme getStatusbarTheme();
    static void setStatusbarTheme(const MobileUI::Theme theme);

    // Navigation bar
    static QColor getNavbarColor();
    static void setNavbarColor(const QColor &color);

    static Theme getNavbarTheme();
    static void setNavbarTheme(const MobileUI::Theme theme);

    // Refresh UI statusbar/navigationbar themes/colors
    Q_INVOKABLE static void refreshUI();

    // Screen safe areas
    static int getStatusbarHeight();
    static int getNavbarHeight();

    static int getSafeAreaTop();
    static int getSafeAreaLeft();
    static int getSafeAreaRight();
    static int getSafeAreaBottom();

    // Screen helpers
    static bool getScreenKeepOn();
    Q_INVOKABLE static void setScreenKeepOn(const bool on);

    enum ScreenOrientation {
        Unlocked = 0,

        Portrait              = (1 << 0),
        Portrait_upsidedown   = (1 << 1),
        Landscape             = (1 << 2),
        Landscape_right       = (1 << 3),
    };
    Q_ENUM(ScreenOrientation)

    /*!
     * \brief Complex orientation locker.
     * \note Work in progress.
     * \param orientation: see ScreenOrientation enum.
     * \param autoRotate: false to disable auto-rotation completely, true to let some degree of auto-rotation.
     *
     * You can also achieve similar functionality through application manifest or plist:
     * - https://developer.android.com/guide/topics/manifest/activity-element.html#screen
     * - https://developer.apple.com/documentation/bundleresources/information_property_list/uisupportedinterfaceorientations
     */
    Q_INVOKABLE static void lockScreenOrientation(const MobileUI::ScreenOrientation orientation, const bool autoRotate);

    // Other mobile related feature
    Q_INVOKABLE static void vibrate();
};

/* ************************************************************************** */
#endif // MOBILEUI_H
