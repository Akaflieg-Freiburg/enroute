/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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
#include <QPermissions>
#include <QProcess>
#include <QScreen>
#include <QtCore/private/qandroidextras_p.h>

#include "platform/PlatformAdaptor_Android.h"

using namespace Qt::Literals::StringLiterals;


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
    QJniObject const stringObject
        = QJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                             "getSSID",
                                             "()Ljava/lang/String;");
    return stringObject.toString();
}


void Platform::PlatformAdaptor::disableScreenSaver()
{
    bool const on = true;
    // Implementation follows a suggestion found in https://stackoverflow.com/questions/27758499/how-to-keep-the-screen-on-in-qt-for-android
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on] {
        QJniObject const activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid())
        {
            QJniObject const window = activity.callObjectMethod("getWindow",
                                                                "()Landroid/view/Window;");

            if (window.isValid())
            {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on)
                {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
                else
                {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
        QJniEnvironment const env;
        if (env->ExceptionCheck() != 0U) {
            env->ExceptionClear();
        }
    });
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

void Platform::PlatformAdaptor::openSatView(const QGeoCoordinate& coordinate)
{
    auto st = u"geo:%1,%2?z=10"_s.arg(coordinate.latitude()).arg(coordinate.longitude());
    QJniObject const c = QJniObject::fromString(st);
    auto ok = QJniObject::callStaticMethod<jboolean>("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                     "openInGoogleEarth",
                                                     "(Ljava/lang/String;)Z",
                                                     c.object<jstring>());
    if (!ok)
        PlatformAdaptor_Abstract::openSatView(coordinate);
}


QString Platform::PlatformAdaptor::systemInfo()
{
    auto result = Platform::PlatformAdaptor_Abstract::systemInfo();

    // Device Name
    QJniObject const stringObject
        = QJniObject::callStaticObjectMethod<jstring>("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                      "deviceName");
    result += u"<h3>Device</h3>\n"_s;
    result += stringObject.toString();

    // System Log
    QProcess proc;
    proc.startCommand(u"logcat -t 300"_s);
    proc.waitForFinished();
    result += u"<h3>System Log</h3>\n"_s;
    result += u"<pre>\n"_s + proc.readAllStandardOutput() + u"\n</pre>\n"_s;

    return result;
}


void Platform::PlatformAdaptor::vibrateBrief()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateBrief");
}


void Platform::PlatformAdaptor::vibrateLong()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateLong");
}



//
// C Methods
//

extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onLanguageChanged(JNIEnv* /*unused*/, jobject /*unused*/)
{
    exit(0);
}

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
