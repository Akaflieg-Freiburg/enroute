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

#include <QtGlobal>
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

#include "GlobalObject.h"
#include "geomaps/CUP.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/MBTILES.h"
#include "platform/PlatformAdaptor.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_File.h"



Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
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

    getSSID();

    // Don't forget the deferred initialization
    QTimer::singleShot(0, this, &PlatformAdaptor::deferredInitialization);
}


void Platform::PlatformAdaptor::hideSplashScreen()
{
}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    Q_UNUSED(lock)
}


auto Platform::PlatformAdaptor::manufacturer() -> QString
{
    return {};
}


auto Platform::PlatformAdaptor::missingPermissionsExist() -> bool
{
    Q_UNUSED(this);
    return false;
}


void Platform::PlatformAdaptor::vibrateBrief()
{
}


auto Platform::PlatformAdaptor::getSSID() -> QString
{
    return QStringLiteral("<unknown ssid>");
}
void Platform::PlatformAdaptor::importContent()
{
    auto fileNameX = QFileDialog::getOpenFileName(nullptr,
                                                  tr("Import data"),
                                                  QDir::homePath(),
                                                  tr("All files (*)")
                                                  );
    if (!fileNameX.isEmpty()) {
        processFileOpenRequest(fileNameX);
    }
}


auto Platform::PlatformAdaptor::exportContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    // Avoids warnings on Linux/Desktop
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)
    (void)this;


    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);

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
}


auto Platform::PlatformAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

    QString tmpPath = contentToTempFile(content, fileNameTemplate);
    bool success = QDesktopServices::openUrl(QUrl("file://" + tmpPath, QUrl::TolerantMode));
    if (success) {
        return {};
    }
    return tr("Unable to open data in other app.");
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



void Platform::PlatformAdaptor::startReceiveOpenFileRequests()
{
    receiveOpenFileRequestsStarted = true;

    if (!pendingReceiveOpenFileRequest.isEmpty()) {
        processFileOpenRequest(pendingReceiveOpenFileRequest);
    }
    pendingReceiveOpenFileRequest = QString();
}


void Platform::PlatformAdaptor::processFileOpenRequest(const QByteArray &path)
{
    processFileOpenRequest(QString::fromUtf8(path).simplified());
}


void Platform::PlatformAdaptor::processFileOpenRequest(const QString &path)
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


#endif // defined(Q_OS_LINUX)
