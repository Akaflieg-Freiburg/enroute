/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

using namespace Qt::Literals::StringLiterals;


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


void Platform::FileExchange::onGUISetupCompleted()
{
    // Start receiving file requests. This needs to be in deferredInitialization because
    // it has side effects and calls constructors of other GlobalObjects.
    receiveOpenFileRequestsStarted = true;
    QJniObject const activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid())
    {
        QJniObject const jniTempDir = QJniObject::fromString(fileExchangeDirectoryName);
        if (!jniTempDir.isValid())
        {
            return;
        }
        activity.callMethod<void>("checkPendingIntents", "(Ljava/lang/String;)V", jniTempDir.object<jstring>());
    }

    if (!pendingReceiveOpenFileRequest.isEmpty())
    {
        processFileOpenRequest(pendingReceiveOpenFileRequest, pendingReceiveOpenFileRequestUnmingledName);
    }
    pendingReceiveOpenFileRequest = {};
}


void Platform::FileExchange::processFileOpenRequest(const QString& path, const QString& unmingledFilename)
{
    if (!receiveOpenFileRequestsStarted)
    {
        pendingReceiveOpenFileRequest = path;
        pendingReceiveOpenFileRequestUnmingledName = unmingledFilename;
        return;
    }
    Platform::FileExchange_Abstract::processFileOpenRequest(path, unmingledFilename);
}


QString Platform::FileExchange::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameSuffix, const QString& fileNameTemplate)
{
    QMimeDatabase const db;
    QMimeType const mime = db.mimeTypeForName(mimeType);

    auto tmpPath = contentToTempFile(content, fileNameTemplate + u"."_s + fileNameSuffix);
    bool const success = outgoingIntent(QStringLiteral("sendFile"), tmpPath, mimeType);
    if (success)
    {
        return {};
    }
    return tr("No suitable file sharing app could be found.");
}


QString Platform::FileExchange::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameSuffix, const QString& fileNameTemplate)
{
    QString const tmpPath = contentToTempFile(content, fileNameTemplate + u"."_s + fileNameSuffix);
    bool const success = outgoingIntent(QStringLiteral("viewFile"), tmpPath, mimeType);
    if (success)
    {
        return {};
    }
    return tr("No suitable app for viewing this data could be found.");
}


void Platform::FileExchange::openFilePicker(const QString& mime)
{
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        const QJniObject mimeTypeStr = QJniObject::fromString(mime);
        activity.callMethod<void>("openFilePicker", "(Ljava/lang/String;)V", mimeTypeStr.object<jstring>());
    }
}


//
// Private Methods
//

QString Platform::FileExchange::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate)
{
    QDateTime const now = QDateTime::currentDateTimeUtc();
    QString const fname = fileNameTemplate.arg(now.toString(QStringLiteral("yyyy-MM-dd_hh.mm.ss")));

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = fileExchangeDirectoryName + fname;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        return {};
    }

    file.write(content);
    file.close();

    return filePath;
}


bool Platform::FileExchange::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType)
{
    QJniObject const jsPath = QJniObject::fromString(filePath);
    QJniObject const jsMimeType = QJniObject::fromString(mimeType);
    auto ok = QJniObject::callStaticMethod<jboolean>("de/akaflieg_freiburg/enroute/MobileAdaptor",
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

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_setFileReceived(JNIEnv* env, jobject /*unused*/, jstring jfname, jstring junmingled)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    const char* unmingled = env->GetStringUTFChars(junmingled, nullptr);

    // A little complicated because GlobalObject::fileExchange() lives in a different thread
    QMetaObject::invokeMethod( GlobalObject::fileExchange(),
                              "processFileOpenRequest",
                              Qt::QueuedConnection,
                              Q_ARG( QString, QString::fromUtf8(fname)),
                              Q_ARG( QString, QString::fromUtf8(unmingled))
                              );
    env->ReleaseStringUTFChars(jfname, fname);
    env->ReleaseStringUTFChars(junmingled, unmingled);
}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject /*unused*/, jstring jfname, jstring junmingled)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    const char* unmingled = env->GetStringUTFChars(junmingled, nullptr);

    // A little complicated because GlobalObject::fileExchange() lives in a different thread
    QMetaObject::invokeMethod( GlobalObject::fileExchange(),
                              "processFileOpenRequest",
                              Qt::QueuedConnection,
                              Q_ARG( QString, QString::fromUtf8(fname)),
                              Q_ARG( QString, QString::fromUtf8(unmingled))
                              );
    env->ReleaseStringUTFChars(jfname, fname);
    env->ReleaseStringUTFChars(junmingled, unmingled);
}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setTextReceived(JNIEnv* env, jobject /*unused*/, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);

    // A little complicated because GlobalObject::fileExchange() lives in a different thread
    QMetaObject::invokeMethod( GlobalObject::fileExchange(),
                              "processText",
                              Qt::QueuedConnection,
                              Q_ARG( QString, QString::fromUtf8(fname)) );
    env->ReleaseStringUTFChars(jfname, fname);
}


}
