/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner                                *
 *   johannes@zellner.org                                                  *
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


#include "MobileAdaptor.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QMimeDatabase>
#include <QUrl>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

#if defined(Q_OS_LINUX)
#include <QProcess>
#endif


bool MobileAdaptor::sendContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate)
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
#if defined(Q_OS_ANDROID)
    return outgoingIntent("sendFile", tmpPath, mimeType);
#endif

#if defined(Q_OS_LINUX)
    QProcess xdgEmail;
    xdgEmail.start("xdg-email", QStringList() << "--attach" << tmpPath);
    if (!xdgEmail.waitForStarted())
        return false;
    if (!xdgEmail.waitForFinished())
        return false;
    return true;
#endif
    return false;
}


bool MobileAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate)
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
#if defined(Q_OS_ANDROID)
    return outgoingIntent("viewFile", tmpPath, mimeType);
#else
    return QDesktopServices::openUrl(QUrl("file://" + tmpPath, QUrl::TolerantMode));
#endif
}


QString MobileAdaptor::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = fileNameTemplate.arg(now.toString("yyyy-MM-dd_hh.mm.ss"));

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = fileExchangeDirectoryName + fname;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return QString();
    }

    file.write(content);
    file.close();

    return filePath;
}



void MobileAdaptor::startReceiveOpenFileRequests()
{
    receiveOpenFileRequestsStarted = true;

#if defined(Q_OS_ANDROID)
    QAndroidJniObject activity = QtAndroid::androidActivity();

    if (activity.isValid()) {
        QAndroidJniObject jniTempDir = QAndroidJniObject::fromString(fileExchangeDirectoryName);
        if (!jniTempDir.isValid())
            return;
        activity.callMethod<void>("checkPendingIntents", "(Ljava/lang/String;)V", jniTempDir.object<jstring>());
    }
#endif

    if (!pendingReceiveOpenFileRequest.isEmpty())
        processFileOpenRequest(pendingReceiveOpenFileRequest);
    pendingReceiveOpenFileRequest = QString();
}


void MobileAdaptor::processFileOpenRequest(const QString &path)
{
    if (!receiveOpenFileRequestsStarted) {
        pendingReceiveOpenFileRequest = path;
        return;
    }

    QString myPath;
    if (path.startsWith("file:")) {
        QUrl url(path.trimmed());
        myPath = url.toLocalFile();
    } else
        myPath = path;

    QMimeDatabase db;
    auto mimeType = db.mimeTypeForFile(myPath);
    if ((mimeType.name() == "application/xml")
            || (mimeType.name() == "application/x-gpx+xml")) {
        // We assume that the file contains a flight route in GPX format
        emit openFileRequest(myPath, FlightRoute_GPX);
        return;
    }
    if ((mimeType.name() == "text/plain")
            || (mimeType.name() == "application/geo+json")) {
        // We assume that the file contains a flight route in GeoJson format
        emit openFileRequest(myPath, FlightRoute_GeoJson);
        return;
    }
    emit openFileRequest(myPath, UnknownFunction);
}


#if defined(Q_OS_ANDROID)
MobileAdaptor* MobileAdaptor::getInstance()
{
    if (!mInstance) {
        mInstance = new MobileAdaptor;
    }

    return mInstance;
}


bool MobileAdaptor::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType)
{
    if (filePath == nullptr)
        return false;

    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    auto ok = QAndroidJniObject::callStaticMethod<jboolean>(
                "de/akaflieg_freiburg/enroute/IntentLauncher",
                methodName.toStdString().c_str(),
                "(Ljava/lang/String;Ljava/lang/String;)Z",
                jsPath.object<jstring>(),
                jsMimeType.object<jstring>());
    return ok;
}


extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    MobileAdaptor::getInstance()->processFileOpenRequest(fname);
    env->ReleaseStringUTFChars(jfname, fname);
}


}
#endif

