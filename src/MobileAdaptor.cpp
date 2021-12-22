/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

#if defined(Q_OS_ANDROID)
#include <QHash>
#include <QJniEnvironment>
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif

#include "MobileAdaptor.h"


MobileAdaptor::MobileAdaptor(QObject *parent)
    : QObject(parent)
{

    // Do all the set-up required for sharing files
    // Android requires you to use a subdirectory within the AppDataLocation for
    // sending and receiving files. We create this and clear this directory on creation of the Share object -- even if the
    // app didn't exit gracefully, the directory is still cleared when starting
    // the app next time.
    fileExchangeDirectoryName = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/exchange/";
    QDir exchangeDir(fileExchangeDirectoryName);
    exchangeDir.removeRecursively();
    exchangeDir.mkpath(fileExchangeDirectoryName);

#if defined (Q_OS_ANDROID)
    // Ask for permissions
    permissions << "android.permission.ACCESS_COARSE_LOCATION";
    permissions << "android.permission.ACCESS_FINE_LOCATION";
    permissions << "android.permission.ACCESS_NETWORK_STATE";
    permissions << "android.permission.ACCESS_WIFI_STATE";
    permissions << "android.permission.READ_EXTERNAL_STORAGE";
    permissions << "android.permission.WRITE_EXTERNAL_STORAGE";
    permissions << "android.permission.WAKE_LOCK";
#warning implementation blocks GUI thread
    foreach(auto permission, permissions) {
        auto resultFuture = QtAndroidPrivate::requestPermission(permission);
        resultFuture.waitForFinished();
    }
#warning Not implemented
    //QtAndroid::requestPermissionsSync(permissions);
#endif

    // Disable the screen saver so that the screen does not switch off automatically
#if defined(Q_OS_ANDROID)
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
#endif

    getSSID();

    // Don't forget the deferred initialization
    QTimer::singleShot(0, this, &MobileAdaptor::deferredInitialization);

}


void MobileAdaptor::deferredInitialization()
{
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
#endif
}
