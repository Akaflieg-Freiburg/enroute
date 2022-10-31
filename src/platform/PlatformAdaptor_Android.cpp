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

#include <QtGlobal>
#if defined(Q_OS_ANDROID)

#include <QAndroidJniEnvironment>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QtAndroid>

#include "GlobalObject.h"
#include "platform/PlatformAdaptor_Android.h"
#include "platform/Notifier_Android.h"


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : Platform::PlatformAdaptor_Abstract(parent)
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

    // Ask for permissions
    permissions << "android.permission.ACCESS_COARSE_LOCATION";
    permissions << "android.permission.ACCESS_FINE_LOCATION";
    permissions << "android.permission.ACCESS_NETWORK_STATE";
    permissions << "android.permission.ACCESS_WIFI_STATE";
    permissions << "android.permission.READ_EXTERNAL_STORAGE";
    permissions << "android.permission.WRITE_EXTERNAL_STORAGE";
    permissions << "android.permission.WAKE_LOCK";
    QtAndroid::requestPermissionsSync(permissions);

    // Disable the screen saver so that the screen does not switch off automatically
    bool on=true;
    // Implementation follows a suggestion found in https://stackoverflow.com/questions/27758499/how-to-keep-the-screen-on-in-qt-for-android
    QtAndroid::runOnAndroidThread([on]{
        QAndroidJniObject activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            QAndroidJniObject window =
                    activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

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
        QAndroidJniEnvironment env;
        if (env->ExceptionCheck() != 0u) {
            env->ExceptionClear();
        }
    });

    currentSSID();

    // Don't forget the deferred initialization
    QTimer::singleShot(0, this, &PlatformAdaptor::deferredInitialization);
}


void Platform::PlatformAdaptor::deferredInitialization()
{
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
}


void Platform::PlatformAdaptor::onGUISetupCompleted()
{
    if (splashScreenHidden)
    {
        return;
    }

    splashScreenHidden = true;

    // Hide Splash Screen
    QtAndroid::hideSplashScreen(200);

    // Start receiving file requests
    receiveOpenFileRequestsStarted = true;
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        QAndroidJniObject jniTempDir = QAndroidJniObject::fromString(fileExchangeDirectoryName);
        if (!jniTempDir.isValid()) {
            return;
        }
        activity.callMethod<void>("checkPendingIntents", "(Ljava/lang/String;)V", jniTempDir.object<jstring>());
    }
    if (!pendingReceiveOpenFileRequest.isEmpty()) {
        processFileOpenRequest(pendingReceiveOpenFileRequest);
    }
    pendingReceiveOpenFileRequest = QString();

}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/PlatformAdaptor", "lockWiFi", "(Z)V", lock);

}


auto Platform::PlatformAdaptor::hasMissingPermissions() -> bool
{

    // Check is required permissions have been granted
    foreach(auto permission, permissions)
    {
        if (QtAndroid::checkPermission(permission) == QtAndroid::PermissionResult::Denied)
        {
            return true;
        }
    }
    return false;

}


void Platform::PlatformAdaptor::vibrateBrief()
{
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/PlatformAdaptor", "vibrateBrief");
}


auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    QAndroidJniObject stringObject = QAndroidJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/PlatformAdaptor",
                                                                               "getSSID", "()Ljava/lang/String;");
    return stringObject.toString();
}


void Platform::PlatformAdaptor::importContent()
{
}


auto Platform::PlatformAdaptor::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    // Avoids warnings on Linux/Desktop
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)
    (void)this;


    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);

    auto tmpPath = contentToTempFile(content, fileNameTemplate+"-%1."+mime.preferredSuffix());
    bool success = outgoingIntent("sendFile", tmpPath, mimeType);
    if (success) {
        return QString();
    }
    return tr("No suitable file sharing app could be found.");
}


auto Platform::PlatformAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
    bool success = outgoingIntent("viewFile", tmpPath, mimeType);
    if (success) {
        return QString();
    }
    return tr("No suitable app for viewing this data could be found.");
}


auto Platform::PlatformAdaptor::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate) -> QString
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = fileNameTemplate.arg(now.toString(QStringLiteral("yyyy-MM-dd_hh.mm.ss")));

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = fileExchangeDirectoryName + fname;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return {};
    }

    file.write(content);
    file.close();

    return filePath;
}


auto Platform::PlatformAdaptor::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType) -> bool
{
    if (filePath == nullptr) {
        return false;
    }

    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    auto ok = QAndroidJniObject::callStaticMethod<jboolean>(
                "de/akaflieg_freiburg/enroute/IntentLauncher",
                methodName.toStdString().c_str(),
                "(Ljava/lang/String;Ljava/lang/String;)Z",
                jsPath.object<jstring>(),
                jsMimeType.object<jstring>());
    return ok != 0U;
}


void Platform::PlatformAdaptor::processFileOpenRequest(const QString& path)
{
    if (!receiveOpenFileRequestsStarted)
    {
        pendingReceiveOpenFileRequest = path;
        return;
    }
    Platform::PlatformAdaptor_Abstract::processFileOpenRequest(path);
}



extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject /*unused*/, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    GlobalObject::platformAdaptor()->processFileOpenRequest(QString::fromUtf8(fname));
    env->ReleaseStringUTFChars(jfname, fname);
}


JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_PlatformAdaptor_onWifiConnected(JNIEnv* /*unused*/, jobject /*unused*/)
{
    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        GlobalObject::platformAdaptor()->wifiConnected();
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
    if (!GlobalObject::canConstruct())
    {
        return;
    }
    auto* ptr = qobject_cast<Platform::Notifier_Android*>(GlobalObject::notifier());

    if (ptr == nullptr)
    {
        return;
    }
    ptr->onNotificationClicked((Platform::Notifier::NotificationTypes)notifyID, actionID);

}

}

#endif // defined(Q_OS_ANDROID)
