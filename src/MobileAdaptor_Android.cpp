/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include "Global.h"
#include "MobileAdaptor.h"
#include "geomaps/GeoMapProvider.h"
#include "traffic/TrafficDataProvider.h"


void MobileAdaptor::hideSplashScreen()
{

    if (splashScreenHidden) {
        return;
    }
    splashScreenHidden = true;
    QtAndroid::hideSplashScreen(200);

}


void MobileAdaptor::lockWifi(bool lock)
{

    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "lockWifi", "(Z)V", lock);

}


Q_INVOKABLE auto MobileAdaptor::missingPermissionsExist() -> bool
{

    // Check is required permissions have been granted
    foreach(auto permission, permissions) {
        if (QtAndroid::checkPermission(permission) == QtAndroid::PermissionResult::Denied) {
            return true;
        }
    }
    return false;

}


void MobileAdaptor::vibrateBrief()
{

    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateBrief");

}


auto MobileAdaptor::getSSID() -> QString
{
    QAndroidJniObject stringObject = QAndroidJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                                               "getSSID", "()Ljava/lang/String;");
    return stringObject.toString();
}


void MobileAdaptor::showDownloadNotification(bool show)
{

    QString text;
    if (show) {
        text = tr("Downloading map data…");
    }
    QAndroidJniObject jni_title   = QAndroidJniObject::fromString(text);
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "notifyDownload", "(Ljava/lang/String;)V", jni_title.object<jstring>());

}


void MobileAdaptor::showTrafficReceiverErrorNotification(QString text)
{

    QAndroidJniObject jni_title = QAndroidJniObject::fromString(tr("Warning: Traffic data receiver"));
    QAndroidJniObject jni_subject = QAndroidJniObject::fromString(text);
    QAndroidJniObject jni_text = QAndroidJniObject::fromString(text);
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "notifyTrafficReceiverError", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", jni_title.object<jstring>(), jni_subject.object<jstring>(), jni_text.object<jstring>());

}


extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onWifiConnected(JNIEnv* /*unused*/, jobject /*unused*/)
{

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (QCoreApplication::instance() == nullptr) {
        return;
    }

    Global::mobileAdaptor()->emitWifiConnected();

}

// This method is called from Java to indicate that the user has clicked into the Android
// notification for reporting traffic data receiver errors

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onNotificationClicked(JNIEnv* /*unused*/, jobject /*unused*/)
{

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (QCoreApplication::instance() == nullptr) {
        return;
    }

    Global::mobileAdaptor()->emitNotificationClicked();

}

}
