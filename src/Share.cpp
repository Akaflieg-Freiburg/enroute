/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// #include <jni.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>

#include "Share.h"

namespace {

// must match cache-path in res/xml/filepaths.xml
const QString kFileProviderPath = "/shared_routes";

} // namespace

Share* Share::mInstance = nullptr;

Share::Share(QObject* parent) : QObject(parent)
{
    // create a save directory, if it doesn't already exist
    mSavePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + kFileProviderPath + "/";
    QDir tempDir(mSavePath);
    if (!tempDir.exists()) {
        QDir("").mkpath(mSavePath);
    }

    // we need the instance for the JNI Call
    mInstance = this;

    // android requires you to use a subdirectory within the AppDataLocation for
    // sending and receiving files. Because of this, when necessary we do a deep
    // copy of the file content.
    //
    // We clear this directory on creation of the Share object -- even if the
    // app didn't exit gracefully, the directory is still cleared when starting
    // the app next time.
    //
    clearTempDir();
}

void Share::sendContent(const QString& content, const QString& mimeType, const QString& suffix)
{
    QString tmpPath = contentToTempFile(content, suffix);
    outgoingIntent("sendFile", tmpPath, mimeType);
}

void Share::viewContent(const QString& content, const QString& mimeType, const QString& suffix)
{
    QString tmpPath = contentToTempFile(content, suffix);
    outgoingIntent("viewFile", tmpPath, mimeType);
}

void Share::saveContent(const QString& content, const QString& mimeType, const QString& suffix)
{
    QString tmpPath = contentToTempFile(content, suffix);
    outgoingIntent("saveFile", tmpPath, mimeType);
}

void Share::importFile(const QString& mimeType)
{
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);

    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/IntentLauncher",
        "openFile",
        "(Ljava/lang/String;)Z",
        jsMimeType.object<jstring>());
    if (!ok) {
        qWarning() << "Unable to resolve activity from Java";
        emit shareNoAppAvailable();
    }
}

void Share::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType)
{
    if (filePath == nullptr)
        return;

    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/IntentLauncher",
        methodName.toStdString().c_str(),
        "(Ljava/lang/String;Ljava/lang/String;)Z",
        jsPath.object<jstring>(),
        jsMimeType.object<jstring>());
    if (!ok) {
        qWarning() << "Unable to resolve activity from Java";
        emit shareNoAppAvailable();
    }
}

// helper method which saves content to temporary file in the app's private cache dir
//
QString Share::contentToTempFile(const QString& content, const QString& suffix)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = now.toString("yyyy-MM-dd_hh.mm.ss") + "." + suffix;

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = tempDir() + fname;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug() << "Share::share " << filePath;
        return nullptr;
    }

    QTextStream out(&file);
    out << content;
    out.flush();
    file.close();

    return filePath;
}

void Share::checkPendingIntents()
{
    QAndroidJniObject activity = QtAndroid::androidActivity();

    if (activity.isValid()) {

        QAndroidJniObject jniTempDir = QAndroidJniObject::fromString(tempDir());
        if (!jniTempDir.isValid()) {
            return;
        }

        activity.callMethod<void>("checkPendingIntents", "(Ljava/lang/String;)V", jniTempDir.object<jstring>());
    }
}

// remove temporary files which are older than 1 week from cache directory
//
void Share::clearTempDir()
{
    const QDate today = QDate::currentDate();
    QDir cachePath(mSavePath);

    for (const auto &fileInfo : cachePath.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
        if (fileInfo.lastModified().date().daysTo(today) > 7) {
            QString filepath = fileInfo.absoluteFilePath();
            cachePath.remove(filepath);
        }
    }
}

QString Share::tempDir()
{
    return mSavePath;
}

void Share::setFileReceived(const QString& fname)
{
    // emit signal to connected slots like fromGpx()
    //
    emit fileReceived(fname);
}

Share* Share::getInstance()
{
    if (!mInstance) {
        mInstance = new Share;
    }

    return mInstance;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    Share::getInstance()->setFileReceived(fname);
    env->ReleaseStringUTFChars(jfname, fname);
}

#ifdef __cplusplus
}
#endif
