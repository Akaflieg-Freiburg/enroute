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
#include <QProcess>
#include <QScreen>
#include <QtCore/private/qandroidextras_p.h>

#include "platform/PlatformAdaptor_Android.h"


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : Platform::PlatformAdaptor_Abstract(parent)
{
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
    foreach(auto permission, requiredPermissions) {
        auto resultFuture = QtAndroidPrivate::checkPermission(permission);
        resultFuture.waitForFinished();
        if (resultFuture.result() == QtAndroidPrivate::PermissionResult::Denied) {
            qWarning() << "Required permission missing" << permission;
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
    QStringList permissions;
    permissions << requiredPermissions << optionalPermissions;
    foreach(auto permission, permissions) {
        auto resultFuture = QtAndroidPrivate::requestPermission(permission);
        resultFuture.waitForFinished();
    }
}


QString Platform::PlatformAdaptor::sysLog()
{
    QProcess proc;
    proc.startCommand(u"logcat -t 300"_qs);
    proc.waitForFinished();
    return proc.readAllStandardOutput();
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
