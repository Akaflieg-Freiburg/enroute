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

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QJniEnvironment>
#include <QJniObject>
#include <QMimeDatabase>
#include <QStandardPaths>

#include "platform/FileExchange_Android.h"
#include "platform/Notifier_Android.h"


Platform::FileExchange::FileExchange(QObject *parent)
    : Platform::FileExchange_Abstract(parent)
{
    // Android requires you to use a subdirectory within the AppDataLocation for
    // sending and receiving files. We create this and clear this directory on creation of the Share object -- even if the
    // app didn't exit gracefully, the directory is still cleared when starting
    // the app next time.
    fileExchangeDirectoryName = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/exchange/";
    QDir exchangeDir(fileExchangeDirectoryName);
    exchangeDir.removeRecursively();
    exchangeDir.mkpath(fileExchangeDirectoryName);


    // Start receiving file requests
    receiveOpenFileRequestsStarted = true;
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        QJniObject jniTempDir = QJniObject::fromString(fileExchangeDirectoryName);
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


void Platform::FileExchange::deferredInitialization()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
}


//
// Methods
//

void Platform::FileExchange::importContent()
{
}


void Platform::FileExchange::processFileOpenRequest(const QString& path)
{
    if (!receiveOpenFileRequestsStarted)
    {
        pendingReceiveOpenFileRequest = path;
        return;
    }
    Platform::FileExchange_Abstract::processFileOpenRequest(path);
}


auto Platform::FileExchange::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    // Avoids warnings on Linux/Desktop
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)
    (void)this;


    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);

    auto tmpPath = contentToTempFile(content, fileNameTemplate+"-%1."+mime.preferredSuffix());
    bool success = outgoingIntent(QStringLiteral("sendFile"), tmpPath, mimeType);
    if (success) {
        return {};
    }
    return tr("No suitable file sharing app could be found.");
}


auto Platform::FileExchange::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
    bool success = outgoingIntent(QStringLiteral("viewFile"), tmpPath, mimeType);
    if (success) {
        return {};
    }
    return tr("No suitable app for viewing this data could be found.");
}


//
// Private Methods
//

auto Platform::FileExchange::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate) -> QString
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


auto Platform::FileExchange::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType) -> bool
{
     QJniObject jsPath = QJniObject::fromString(filePath);
     QJniObject jsMimeType = QJniObject::fromString(mimeType);
     auto ok = QJniObject::callStaticMethod<jboolean>(
                 "de/akaflieg_freiburg/enroute/IntentLauncher",
                 methodName.toStdString().c_str(),
                 "(Ljava/lang/String;Ljava/lang/String;)Z",
                 jsPath.object<jstring>(),
                 jsMimeType.object<jstring>());
     return ok != 0U;
}


//
// C Methods
//

extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject /*unused*/, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    GlobalObject::fileExchange()->processFileOpenRequest(QString::fromUtf8(fname));
    env->ReleaseStringUTFChars(jfname, fname);
}


}
