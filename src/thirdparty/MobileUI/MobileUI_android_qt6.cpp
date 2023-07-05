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

#include "MobileUI_private.h"

#include <QGuiApplication>
#include <QScreen>
#include <QJniObject>

/* ************************************************************************** */

// WindowManager.LayoutParams
#define FLAG_KEEP_SCREEN_ON                     0x00000080
#define FLAG_TRANSLUCENT_STATUS                 0x04000000
#define FLAG_TRANSLUCENT_NAVIGATION             0x08000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS       0x80000000

// View
#define SYSTEM_UI_FLAG_LAYOUT_STABLE            0x00000100
#define SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION   0x00000200
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR         0x00002000
#define SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR     0x00000010

// UI modes
#define UI_MODE_NIGHT_UNDEFINED                 0x00000000
#define UI_MODE_NIGHT_NO                        0x00000010
#define UI_MODE_NIGHT_YES                       0x00000020
#define UI_MODE_NIGHT_MASK                      0x00000030

// WindowInsetsController
#define APPEARANCE_OPAQUE_STATUS_BARS           0x00000001
#define APPEARANCE_OPAQUE_NAVIGATION_BARS       0x00000002
#define APPEARANCE_LOW_PROFILE_BARS             0x00000004
#define APPEARANCE_LIGHT_STATUS_BARS            0x00000008
#define APPEARANCE_LIGHT_NAVIGATION_BARS        0x00000010
#define APPEARANCE_SEMI_TRANSPARENT_STATUS_BARS 0x00000020
#define APPEARANCE_SEMI_TRANSPARENT_NAVIGATION_BARS 0x0030

// VibrationEffect
#define DEFAULT_AMPLITUDE                       0xffffffff
#define EFFECT_CLICK                            0x00000000
#define EFFECT_DOUBLE_CLICK                     0x00000001
#define EFFECT_HEAVY_CLICK                      0x00000005
#define EFFECT_TICK                             0x00000002

/* ************************************************************************** */

bool MobileUIPrivate::isAvailable_sys()
{
    return true; // Qt6 must be built with Android SDK 23 anyway, enough for everything MobileUI use
}

[[maybe_unused]] static bool isQColorLight(QColor color)
{
    double darkness = 1.0 - (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255.0;
    return (darkness < 0.2);
}

static QJniObject getAndroidWindow()
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

    window.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
    window.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);

    return window;
}

static QJniObject getDisplayCutout()
{
    // DisplayCutout has been added in API level 28

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
    QJniObject decorview = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
    QJniObject insets = decorview.callObjectMethod("getRootWindowInsets", "()Landroid/view/WindowInsets;");
    QJniObject cutout = insets.callObjectMethod("getDisplayCutout", "()Landroid/view/DisplayCutout;");

    return cutout;
}

void updatePreferredStatusBarStyle()
{
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 30)
    {
        MobileUI::setStatusbarTheme(MobileUIPrivate::statusbarTheme);
    }
}

/* ************************************************************************** */

int MobileUIPrivate::getDeviceTheme_sys()
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject rsc = activity.callObjectMethod("getResources", "()Landroid/content/res/Resources;");
    QJniObject conf = rsc.callObjectMethod("getConfiguration", "()Landroid/content/res/Configuration;");

    int uiMode = (conf.getField<int>("uiMode") & UI_MODE_NIGHT_MASK);

    return (uiMode == UI_MODE_NIGHT_YES) ? MobileUI::Theme::Dark : MobileUI::Theme::Light;
}

/* ************************************************************************** */

void MobileUIPrivate::setColor_statusbar(const QColor &color)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject window = getAndroidWindow();
        QJniObject view = window.callObjectMethod("getDecorView", "()Landroid/view/View;");

        window.callMethod<void>("setStatusBarColor", "(I)V", color.rgba());

        int visibility = view.callMethod<int>("getSystemUiVisibility", "()I");
        if (isQColorLight(color))
            visibility |= SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
        else
            visibility &= ~SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;

        view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);
    });
}

void MobileUIPrivate::setTheme_statusbar(MobileUI::Theme theme)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject window = getAndroidWindow();

        if (QNativeInterface::QAndroidApplication::sdkVersion() < 30)
        {
            // Added in API level 23
            // Deprecated in API level 30

            QJniObject view = window.callObjectMethod("getDecorView", "()Landroid/view/View;");

            int visibility = view.callMethod<int>("getSystemUiVisibility", "()I");
            if (theme == MobileUI::Theme::Light)
                visibility |= SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            else
                visibility &= ~SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;

            view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);
        }
        else if (QNativeInterface::QAndroidApplication::sdkVersion() >= 30)
        {
            // Added in API level 30

            QJniObject inset = window.callObjectMethod("getInsetsController",
                                                       "()Landroid/view/WindowInsetsController;");

            int visibility = inset.callMethod<int>("getSystemBarsAppearance", "()I");
            if (theme == MobileUI::Theme::Light)
                visibility |= APPEARANCE_LIGHT_STATUS_BARS;
            else
                visibility &= ~APPEARANCE_LIGHT_STATUS_BARS;

            inset.callMethod<void>("setSystemBarsAppearance", "(II)V",
                                   visibility, APPEARANCE_LIGHT_STATUS_BARS);

            if (!MobileUIPrivate::areRefreshSlotsConnected)
            {
                QScreen *screen = qApp->primaryScreen();
                if (screen) {
                    QObject::connect(screen, &QScreen::orientationChanged,
                                     qApp, [](Qt::ScreenOrientation) { updatePreferredStatusBarStyle(); });
                }

                MobileUIPrivate::areRefreshSlotsConnected = true;
            }
        }
    });
}

/* ************************************************************************** */

void MobileUIPrivate::setColor_navbar(const QColor &color)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject window = getAndroidWindow();
        QJniObject view = window.callObjectMethod("getDecorView", "()Landroid/view/View;");

        window.callMethod<void>("setNavigationBarColor", "(I)V", color.rgba());

        int visibility = view.callMethod<int>("getSystemUiVisibility", "()I");
        if (isQColorLight(color))
            visibility |= SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
        else
            visibility &= ~SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;

        view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);
    });
}

void MobileUIPrivate::setTheme_navbar(MobileUI::Theme theme)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject window = getAndroidWindow();
        QJniObject view = window.callObjectMethod("getDecorView", "()Landroid/view/View;");

        int visibility = view.callMethod<int>("getSystemUiVisibility", "()I");
        if (theme == MobileUI::Theme::Light)
            visibility |= SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
        else
            visibility &= ~SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;

        view.callMethod<void>("setSystemUiVisibility", "(I)V", visibility);
    });
}

/* ************************************************************************** */

int MobileUIPrivate::getStatusbarHeight()
{
    return 24; // TODO
}

int MobileUIPrivate::getNavbarHeight()
{
    return 48; // TODO
}

int MobileUIPrivate::getSafeAreaTop()
{
    // DisplayCutout has been added in API level 28
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 28)
    {
        QJniObject cutout = getDisplayCutout();
        if (cutout.isValid())
        {
            return cutout.callMethod<int>("getSafeInsetTop", "()I") / qApp->devicePixelRatio();
        }
    }

    return 0;
}

int MobileUIPrivate::getSafeAreaLeft()
{
    // DisplayCutout has been added in API level 28
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 28)
    {
        QJniObject cutout = getDisplayCutout();
        if (cutout.isValid())
        {
            return cutout.callMethod<int>("getSafeInsetLeft", "()I") / qApp->devicePixelRatio();
        }
    }

    return 0;
}

int MobileUIPrivate::getSafeAreaRight()
{
    // DisplayCutout has been added in API level 28
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 28)
    {
        QJniObject cutout = getDisplayCutout();
        if (cutout.isValid())
        {
            return cutout.callMethod<int>("getSafeInsetRight", "()I") / qApp->devicePixelRatio();
        }
    }

    return 0;
}

int MobileUIPrivate::getSafeAreaBottom()
{
    // DisplayCutout has been added in API level 28
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 28)
    {
        QJniObject cutout = getDisplayCutout();
        if (cutout.isValid())
        {
            return cutout.callMethod<int>("getSafeInsetBottom", "()I") / qApp->devicePixelRatio();
        }
    }

    return 0;
}

/* ************************************************************************** */

void MobileUIPrivate::setScreenKeepOn(bool on)
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject window = getAndroidWindow();

        if (on)
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
        else
            window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
    });
}

void MobileUIPrivate::lockScreenOrientation(int orientation, bool autoRotate)
{
    // For reference, the enum values from Android SDK:
/*
SCREEN_ORIENTATION_BEHIND 3 (0x00000003)
Constant corresponding to behind in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_FULL_SENSOR 10 (0x0000000a)
Constant corresponding to fullSensor in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_FULL_USER 13 (0x0000000d)
Constant corresponding to fullUser in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_LANDSCAPE 0 (0x00000000)
Constant corresponding to landscape in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_LOCKED 14 (0x0000000e)
Constant corresponding to locked in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_NOSENSOR 5 (0x00000005)
Constant corresponding to nosensor in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_PORTRAIT 1 (0x00000001)
Constant corresponding to portrait in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_REVERSE_LANDSCAPE 8 (0x00000008)
Constant corresponding to reverseLandscape in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_REVERSE_PORTRAIT 9 (0x00000009)
Constant corresponding to reversePortrait in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_SENSOR 4 (0x00000004)
Constant corresponding to sensor in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_SENSOR_LANDSCAPE 6 (0x00000006)
Constant corresponding to sensorLandscape in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_SENSOR_PORTRAIT 7 (0x00000007)
Constant corresponding to sensorPortrait in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_UNSPECIFIED -1 (0xffffffff)
Constant corresponding to unspecified in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_USER 2 (0x00000002)
Constant corresponding to user in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_USER_LANDSCAPE 11 (0x0000000b)
Constant corresponding to userLandscape in the R.attr.screenOrientation attribute.

SCREEN_ORIENTATION_USER_PORTRAIT 12 (0x0000000c)
Constant corresponding to userPortrait in the R.attr.screenOrientation attribute.
*/
    int value = -1; // SCREEN_ORIENTATION_UNSPECIFIED

    if (orientation)
    {
        if (autoRotate)
        {
            if (orientation == MobileUI::Portrait || orientation == MobileUI::Portrait_upsidedown) value = 7; // SCREEN_ORIENTATION_SENSOR_PORTRAIT
            else if (orientation == MobileUI::Landscape || orientation == MobileUI::Landscape_right) value = 6; // SCREEN_ORIENTATION_SENSOR_LANDSCAPE
        }
        else
        {
            if (orientation == MobileUI::Portrait) value = 1; // SCREEN_ORIENTATION_PORTRAIT
            else if (orientation == MobileUI::Portrait_upsidedown) value = 9; // SCREEN_ORIENTATION_REVERSE_PORTRAIT
            else if (orientation == MobileUI::Landscape) value = 0; // SCREEN_ORIENTATION_LANDSCAPE
            else if (orientation == MobileUI::Landscape_right) value = 8; // SCREEN_ORIENTATION_REVERSE_LANDSCAPE
        }
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid())
    {
        activity.callMethod<void>("setRequestedOrientation", "(I)V", value);
    }
}

/* ************************************************************************** */

void MobileUIPrivate::vibrate()
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid())
        {
            QJniObject vibratorString = QJniObject::fromString("vibrator");
            QJniObject vibratorService = activity.callObjectMethod("getSystemService",
                                                                   "(Ljava/lang/String;)Ljava/lang/Object;",
                                                                   vibratorString.object<jstring>());
            if (vibratorService.callMethod<jboolean>("hasVibrator", "()Z"))
            {
                if (QNativeInterface::QAndroidApplication::sdkVersion() >= 26)
                {
                    // vibrate(VibrationEffect vibe) // Added in API level 26

                    jint effect = EFFECT_TICK;
                    QJniObject vibrationEffect = QJniObject::callStaticObjectMethod("android/os/VibrationEffect",
                                                                                    "createPredefined",
                                                                                    "(I)Landroid/os/VibrationEffect;",
                                                                                    effect);

                    vibratorService.callMethod<void>("vibrate",
                                                     "(Landroid/os/VibrationEffect;)V",
                                                     vibrationEffect.object<jobject>());
                }
                else
                {
                    // vibrate(long milliseconds) // Deprecated in API level 26

                    jlong ms = 25;
                    vibratorService.callMethod<void>("vibrate", "(J)V", ms);
                }
            }
        }
        QJniEnvironment env;
        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
        }
    });
}

/* ************************************************************************** */
