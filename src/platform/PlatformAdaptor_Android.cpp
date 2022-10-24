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

#include <QAndroidJniEnvironment>
#include <QCoreApplication>
#include <QDir>
#include <QPointer>
#include <QStandardPaths>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QTimer>

#include "GlobalObject.h"
#include "platform/PlatformAdaptor.h"
#include "geomaps/GeoMapProvider.h"
#include "platform/Notifier_Android.h"
#include "traffic/TrafficDataProvider.h"


void PlatformAdaptor::hideSplashScreen()
{

    if (splashScreenHidden) {
        return;
    }
    splashScreenHidden = true;
    QtAndroid::hideSplashScreen(200);

}


void PlatformAdaptor::lockWifi(bool lock)
{

    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/PlatformAdaptor", "lockWiFi", "(Z)V", lock);

}


Q_INVOKABLE auto PlatformAdaptor::missingPermissionsExist() -> bool
{

    // Check is required permissions have been granted
    foreach(auto permission, permissions) {
        if (QtAndroid::checkPermission(permission) == QtAndroid::PermissionResult::Denied) {
            return true;
        }
    }
    return false;

}


void PlatformAdaptor::vibrateBrief()
{
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/PlatformAdaptor", "vibrateBrief");
}


auto PlatformAdaptor::getSSID() -> QString
{
    QAndroidJniObject stringObject = QAndroidJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/PlatformAdaptor",
                                                                               "getSSID", "()Ljava/lang/String;");
    return stringObject.toString();
}



auto PlatformAdaptor::manufacturer() -> QString
{
    QAndroidJniObject stringObject = QAndroidJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/PlatformAdaptor",
                                                                               "manufacturer", "()Ljava/lang/String;");
    return stringObject.toString();
}


extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_PlatformAdaptor_onWifiConnected(JNIEnv* /*unused*/, jobject /*unused*/)
{

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct()) {
        GlobalObject::platformAdaptor()->emitWifiConnected();
    }

}

// This method is called from Java to indicate that the user has clicked into the Android
// notification for reporting traffic data receiver errors

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_PlatformAdaptor_onNotificationClicked(JNIEnv* /*unused*/, jobject /*unused*/, jint notifyID, jint actionID)
{

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (!GlobalObject::canConstruct()) {
        return;
    }
    auto* ptr = qobject_cast<Platform::Notifier_Android*>(GlobalObject::notifier());

    if (ptr == nullptr) {
        return;
    }
    ptr->onNotificationClicked((Platform::Notifier::NotificationTypes)notifyID, actionID);

}

} // extern "C"
