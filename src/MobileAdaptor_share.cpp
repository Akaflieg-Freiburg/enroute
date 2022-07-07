/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner                                *
 *   johannes@zellner.org                                                  *
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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


#include "GlobalObject.h"
#include "MobileAdaptor.h"
#include "geomaps/CUP.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/MBTILES.h"
#include "navigation/FlightRoute.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_File.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMimeDatabase>
#include <QUrl>

#if defined(Q_OS_ANDROID)
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#else
#include <QFileDialog>
#include <QProcess>
#endif


void MobileAdaptor::importContent()
{
#if !defined(Q_OS_ANDROID)
    auto fileNameX = QFileDialog::getOpenFileName(nullptr,
                                                  tr("Import data"),
                                                  QDir::homePath(),
                                                  tr("All files (*)")
                                                  );
    if (!fileNameX.isEmpty()) {
        processFileOpenRequest(fileNameX);
    }
#endif
}


auto MobileAdaptor::exportContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    // Avoids warnings on Linux/Desktop
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)
    (void)this;


    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);

#if defined(Q_OS_ANDROID)
    auto tmpPath = contentToTempFile(content, fileNameTemplate+"-%1."+mime.preferredSuffix());
    bool success = outgoingIntent("sendFile", tmpPath, mimeType);
    if (success) {
        return QString();
    }
    return tr("No suitable file sharing app could be found.");
#else
    auto fileNameX = QFileDialog::getSaveFileName(nullptr,
                                                  tr("Export flight route"),
                                                  QDir::homePath()+"/"+fileNameTemplate+"."+mime.preferredSuffix(),
                                                  tr("%1 (*.%2);;All files (*)").arg(mime.comment(), mime.preferredSuffix())
                                                  );
    if (fileNameX.isEmpty()) {
        return QStringLiteral("abort");
    }
    QFile file(fileNameX);
    if (!file.open(QIODevice::WriteOnly)) {
        return tr("Unable to open file <strong>%1</strong>.").arg(fileNameX);
    }

    if (file.write(content) != content.size()) {
        return tr("Unable to write to file <strong>%1</strong>.").arg(fileNameX);
    }
    file.close();
    return {};
#endif
}


auto MobileAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
#if defined(Q_OS_ANDROID)
    bool success = outgoingIntent("viewFile", tmpPath, mimeType);
    if (success) {
        return QString();
    }
    return tr("No suitable app for viewing this data could be found.");
#else
    bool success = QDesktopServices::openUrl(QUrl("file://" + tmpPath, QUrl::TolerantMode));
    if (success) {
        return {};
    }
    return tr("Unable to open data in other app.");
#endif
}


auto MobileAdaptor::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate) -> QString
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



void MobileAdaptor::startReceiveOpenFileRequests()
{
    receiveOpenFileRequestsStarted = true;

#if defined(Q_OS_ANDROID)
    QAndroidJniObject activity = QtAndroid::androidActivity();

    if (activity.isValid()) {
        QAndroidJniObject jniTempDir = QAndroidJniObject::fromString(fileExchangeDirectoryName);
        if (!jniTempDir.isValid()) {
            return;
        }
        activity.callMethod<void>("checkPendingIntents", "(Ljava/lang/String;)V", jniTempDir.object<jstring>());
    }
#endif

    if (!pendingReceiveOpenFileRequest.isEmpty()) {
        processFileOpenRequest(pendingReceiveOpenFileRequest);
    }
    pendingReceiveOpenFileRequest = QString();
}


void MobileAdaptor::processFileOpenRequest(const QByteArray &path)
{
    processFileOpenRequest(QString::fromUtf8(path).simplified());
}


void MobileAdaptor::processFileOpenRequest(const QString &path)
{
    if (!receiveOpenFileRequestsStarted) {
        pendingReceiveOpenFileRequest = path;
        return;
    }

    QString myPath;
    if (path.startsWith(u"file:")) {
        QUrl url(path.trimmed());
        myPath = url.toLocalFile();
    } else {
        myPath = path;
    }

    QMimeDatabase db;
    auto mimeType = db.mimeTypeForFile(myPath);

    /*
     * Check for various possible file formats/contents
     */

    // Flight Route in GPX format
    if ((mimeType.inherits(QStringLiteral("application/xml"))) || (mimeType.name() == u"application/x-gpx+xml")) {
        emit openFileRequest(myPath, FlightRouteOrWaypointLibrary);
        return;
    }

    // GeoJSON file
    auto fileContent = GeoMaps::GeoJSON::inspect(myPath);
    if (fileContent == GeoMaps::GeoJSON::flightRoute)
    {
        emit openFileRequest(myPath, FlightRoute);
        return;
    }
    if (fileContent == GeoMaps::GeoJSON::waypointLibrary)
    {
        emit openFileRequest(myPath, WaypointLibrary);
        return;
    }
    if (fileContent == GeoMaps::GeoJSON::valid)
    {
        emit openFileRequest(myPath, FlightRouteOrWaypointLibrary);
        return;
    }

    // FLARM Simulator file
    if (Traffic::TrafficDataSource_File::containsFLARMSimulationData(myPath)) {
        auto *source = new Traffic::TrafficDataSource_File(myPath);
        GlobalObject::trafficDataProvider()->addDataSource(source); // Will take ownership of source
        source->connectToTrafficReceiver();
        return;
    }

    // MBTiles containing a vector map
    GeoMaps::MBTILES mbtiles(myPath);
    if (mbtiles.format() == GeoMaps::MBTILES::Vector)
    {
        emit openFileRequest(myPath, VectorMap);
        return;
    }

    // MBTiles containing a raster map
    if (mbtiles.format() == GeoMaps::MBTILES::Raster)
    {
        emit openFileRequest(myPath, RasterMap);
        return;
    }

    // CUP file
    if (GeoMaps::CUP::isValid(myPath))
    {
        emit openFileRequest(myPath, WaypointLibrary);
        return;
    }

    emit openFileRequest(myPath, UnknownFunction);
}


#if defined(Q_OS_ANDROID)
auto MobileAdaptor::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType) -> bool
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


extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived(JNIEnv* env, jobject /*unused*/, jstring jfname)
{
    const char* fname = env->GetStringUTFChars(jfname, nullptr);
    GlobalObject::mobileAdaptor()->processFileOpenRequest(QString::fromUtf8(fname));
    env->ReleaseStringUTFChars(jfname, fname);
}


}
#endif
