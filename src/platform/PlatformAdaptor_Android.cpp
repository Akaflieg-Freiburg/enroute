/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QGuiApplication>
#include <QHash>
#include <QJniEnvironment>
#include <QJniObject>
#include <QScreen>
#include <QtCore/private/qandroidextras_p.h>
#include <QTimer>
#include <chrono>

#include "platform/PlatformAdaptor_Android.h"

using namespace std::chrono_literals;


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : Platform::PlatformAdaptor_Abstract(parent)
{
#warning
// MUST NOT CALL FROM CONSTRUCTOR    updateSafeInsets();

    // Update the properties safeInsets* 100ms after the screen orientation changes.
    // Experience shows that the system calls on Android do not always
    // reflect updates in the safeInsets* immediately.
    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(100ms);
    connect(timer, &QTimer::timeout, this, &PlatformAdaptor::updateSafeInsets);
    timer->start();

    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, timer, [timer]{ timer->start(); });
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, this, &PlatformAdaptor::updateSafeInsets);
}


void Platform::PlatformAdaptor::updateSafeInsets()
{
    auto safeInsetBottom {_safeInsetBottom};
    auto safeInsetLeft {_safeInsetLeft};
    auto safeInsetRight {_safeInsetRight};
    auto safeInsetTop {_safeInsetTop};

    auto devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
    if ( qIsFinite(devicePixelRatio) && (devicePixelRatio > 0.0))
    {
        double inset = 0.0;

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetBottom"));
        if ( qIsFinite(safeInsetBottom) && (safeInsetBottom >= 0.0) )
        {
            safeInsetBottom = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetLeft"));
        if ( qIsFinite(safeInsetLeft) && (safeInsetLeft >= 0.0) )
        {
            safeInsetLeft = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetRight"));
        if ( qIsFinite(safeInsetRight) && (safeInsetRight >= 0.0) )
        {
            safeInsetRight = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetTop"));
        if ( qIsFinite(safeInsetTop) && (safeInsetTop >= 0.0) )
        {
            safeInsetTop = inset/devicePixelRatio;
        }
#warning Stimmt noch nicht. Vermutlich ist alles ein bischen komplizierter
//        safeInsetBottom += (QGuiApplication::primaryScreen()->availableSize().height()-QGuiApplication::inputMethod()->keyboardRectangle().top())/devicePixelRatio;

        auto* inputMethod = QGuiApplication::inputMethod();
        if ((inputMethod != nullptr) && inputMethod->isVisible())
        {
            auto keyboardRectangle = inputMethod->keyboardRectangle();
            safeInsetBottom = QGuiApplication::primaryScreen()->availableSize().height() - keyboardRectangle.top()/devicePixelRatio;
//            safeInsetBottom = (QGuiApplication::inputMethod()->keyboardRectangle().height())/devicePixelRatio;
            qWarning() << "ScHeight" << QGuiApplication::primaryScreen()->availableSize().height();
            qWarning() << "KrHeight" << QGuiApplication::inputMethod()->keyboardRectangle().height()/devicePixelRatio;
            qWarning() << "KrTop   " << QGuiApplication::inputMethod()->keyboardRectangle().top()/devicePixelRatio;
        }
    }
    qWarning() << "safeInsetBottom" << safeInsetBottom;

    // Update properties and emit notification signals
    if (safeInsetBottom != _safeInsetBottom)
    {
        _safeInsetBottom = safeInsetBottom;
        emit safeInsetBottomChanged();
    }
    if (safeInsetLeft != _safeInsetLeft)
    {
        _safeInsetLeft = safeInsetLeft;
        emit safeInsetLeftChanged();
    }
    if (safeInsetRight != _safeInsetRight)
    {
        _safeInsetRight = safeInsetRight;
        emit safeInsetRightChanged();
    }
    if (safeInsetTop != _safeInsetTop)
    {
        _safeInsetTop = safeInsetTop;
        emit safeInsetTopChanged();
    }
}

void Platform::PlatformAdaptor::deferredInitialization()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
}


//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    QJniObject stringObject = QJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                                               "getSSID", "()Ljava/lang/String;");
    return stringObject.toString();
}


void Platform::PlatformAdaptor::disableScreenSaver()
{
    bool on=true;
    // Implementation follows a suggestion found in https://stackoverflow.com/questions/27758499/how-to-keep-the-screen-on-in-qt-for-android
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on]{
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid()) {
            QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

            if (window.isValid()) {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on) {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                } else {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
        QJniEnvironment env;
        if (env->ExceptionCheck() != 0u) {
            env->ExceptionClear();
        }
    });
}


auto Platform::PlatformAdaptor::hasRequiredPermissions() -> bool
{
    // Check is required permissions have been granted
    foreach(auto permission, permissions) {
#warning not optimal, may lead to startup delay
        auto resultFuture = QtAndroidPrivate::checkPermission(permission);
        resultFuture.waitForFinished();
        if (resultFuture.result() == QtAndroidPrivate::PermissionResult::Denied) {
            return false;
        }
    }
    return true;
}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "lockWiFi", "(Z)V", lock);
}


void Platform::PlatformAdaptor::onGUISetupCompleted()
{
    if (splashScreenHidden)
    {
        return;
    }

    splashScreenHidden = true;

    // Hide Splash Screen
    QNativeInterface::QAndroidApplication::hideSplashScreen(200);
}


void Platform::PlatformAdaptor::requestPermissionsSync()
{
#warning implementation blocks GUI thread
    foreach(auto permission, permissions) {
        auto resultFuture = QtAndroidPrivate::requestPermission(permission);
        resultFuture.waitForFinished();
    }
#warning Not implemented
    //QtAndroid::requestPermissionsSync(permissions);
}



void Platform::PlatformAdaptor::vibrateBrief()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateBrief");
}



//
// C Methods
//

extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onWifiConnected(JNIEnv* /*unused*/, jobject /*unused*/)
{
    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        emit GlobalObject::platformAdaptor()->wifiConnected();
    }
}

}
