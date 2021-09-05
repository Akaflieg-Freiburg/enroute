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
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>

#include "Global.h"
#include "MobileAdaptor.h"
#include "geomaps/GeoMapProvider.h"
#include "traffic/TrafficDataProvider.h"


#if defined(Q_OS_ANDROID)
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#endif



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
    permissions << "android.permission.WRITE_EXTERNAL_STORAGE";
    permissions << "android.permission.READ_EXTERNAL_STORAGE";
    QtAndroid::requestPermissionsSync(permissions);
#endif

    // Disable the screen saver so that the screen does not switch off automatically
#if defined(Q_OS_ANDROID)
    bool on=true;
    // Implementation follows a suggestion found in https://stackoverflow.com/questions/27758499/how-to-keep-the-screen-on-in-qt-for-android
    QtAndroid::runOnAndroidThread([on]{
        QAndroidJniObject activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            QAndroidJniObject window =
                    activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

            if (window.isValid()) {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on) {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                } else {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
        QAndroidJniEnvironment env;
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
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
#endif

    connect(Global::mapManager()->geoMaps(), &GeoMaps::DownloadableGroup::downloadingChanged, this, &MobileAdaptor::showDownloadNotification);
    connect(Global::trafficDataProvider(), &Traffic::TrafficDataProvider::trafficReceiverSelfTestErrorChanged, this,
            [this](QString message){ showNotification(TrafficReceiverSelfTestError, message); } );
}


MobileAdaptor::~MobileAdaptor()
{
    // Close all pending notifications
    showDownloadNotification(false);
    hideNotification(TrafficReceiverSelfTestError);
    hideNotification(TrafficReceiverProblem);
}
