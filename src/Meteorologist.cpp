/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include <QDataStream>
#include <QLockFile>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSaveFile>
#include <QtGlobal>
#include <QQmlEngine>
#include <QXmlStreamReader>

#include "sunset.h"

#include "Clock.h"
#include "GeoMapProvider.h"
#include "GlobalSettings.h"
#include "FlightRoute.h"
#include "Meteorologist.h"
#include "SatNav.h"
#include "Weather_METAR.h"


Meteorologist::Meteorologist(FlightRoute *route,
                             GeoMapProvider *geoMapProvider,
                             QNetworkAccessManager *networkAccessManager,
                             QObject *parent) :
    QObject(parent),
    _flightRoute(route),
    _geoMapProvider(geoMapProvider),
    _networkAccessManager(networkAccessManager)
{
    // Connect the timer to the update method. This will set backgroundUpdate to the default value,
    // which is true. So these updates happen in the background.
    // Schedule the first update in 1 seconds from now
    connect(&_updateTimer, &QTimer::timeout, [=](){ this->update(); });
    _updateTimer.setInterval(1*1000);
    _updateTimer.start();

    // Connect the timer to check for expired messages
    connect(&_deleteExiredMessagesTimer, &QTimer::timeout, this, &Meteorologist::deleteExpiredMesages);
    _deleteExiredMessagesTimer.setInterval(10*60*1000);
    _deleteExiredMessagesTimer.start();

    // Update the description text when needed
    connect(this, &Meteorologist::weatherStationsChanged, this, &Meteorologist::QNHInfoChanged);
#warning move this to extra function
    auto _satNav = SatNav::globalInstance();
    if (_satNav) {
        connect(_satNav, &SatNav::statusChanged, this, &Meteorologist::QNHInfoChanged);
        connect(_satNav, &SatNav::statusChanged, this, &Meteorologist::sunInfoChanged);
    }

#warning move this to extra function
    auto _clock = Clock::globalInstance();
    if (_clock) {
        connect(_clock, &Clock::timeChanged, this, &Meteorologist::QNHInfoChanged);
        connect(_clock, &Clock::timeChanged, this, &Meteorologist::sunInfoChanged);
    }

#warning make sure that METAR/TAFs are not immediately reloaded if recent versions exist in the file

    // Read METAR/TAF from "weather.dat"
    load();
}


Meteorologist::~Meteorologist()
{

}


void Meteorologist::deleteExpiredMesages()
{
    QVector<QString> ICAOCodesToDelete;

    foreach(auto weatherStation, _weatherStationsByICAOCode) {
        if (weatherStation->hasMETAR()) {
            if (weatherStation->metar()->expiration() < QDateTime::currentDateTime())
                weatherStation->setMETAR(nullptr);
        }
        if (weatherStation->hasTAF()) {
            if (weatherStation->taf()->expiration() < QDateTime::currentDateTime())
                weatherStation->setTAF(nullptr);
        }

        if (!weatherStation->hasMETAR() && !weatherStation->hasTAF()) {
            ICAOCodesToDelete << weatherStation->ICAOCode();
            weatherStation->deleteLater();
        }
    }

    // If there is nothing to delete, wonderful
    if (ICAOCodesToDelete.isEmpty())
        return;

    // Otherwiese, remove the weather stations from the list, and let the world know
    foreach(auto ICAOCodeToDelete, ICAOCodesToDelete)
        _weatherStationsByICAOCode.remove(ICAOCodeToDelete);
    emit weatherStationsChanged();
    save();
}


bool Meteorologist::downloading() const
{
    foreach(auto networkReply, _networkReplies) {
        if (networkReply.isNull())
            continue;
        if (networkReply->isRunning())
            return true;
    }

    return false;
}


void Meteorologist::downloadFinished() {

    // Update flag
    emit downloadingChanged();

    // Start to process the data only once ALL replies have been received. So, we check here if there are any running
    // download processes and abort if indeed there are some.
    if (downloading())
        return;

    // Read all replies and store the data in respective maps
    foreach(auto networkReply, _networkReplies) {
        // Paranoid safety checks
        if (networkReply.isNull())
            continue;
        if (networkReply->error() != QNetworkReply::NoError) {
            emit error(networkReply->errorString());
            continue;
        }

        // Decode XML
        QXmlStreamReader xml(networkReply);
        while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();

            // Read METAR
            if (xml.isStartElement() && (xml.name() == "METAR")) {
                auto metar = new Weather::METAR(xml, this);
                findOrConstructWeatherStation(metar->ICAOCode())->setMETAR(metar);
            }

            // Read TAF
            if (xml.isStartElement() && (xml.name() == "TAF")) {
                auto taf = new Weather::TAF(xml, this);
                findOrConstructWeatherStation(taf->ICAOCode())->setTAF(taf);
            }

        }
    }

    // Clear replies container
    qDeleteAll(_networkReplies);
    _networkReplies.clear();

    // Update flag and signals
    emit weatherStationsChanged();
    emit QNHInfoChanged();
    save();
}


Weather::Station *Meteorologist::findOrConstructWeatherStation(const QString &ICAOCode)
{
    auto weatherStationPtr = _weatherStationsByICAOCode.value(ICAOCode, nullptr);

    if (!weatherStationPtr.isNull())
        return weatherStationPtr;

    auto newWeatherStation = new Weather::Station(ICAOCode, _geoMapProvider, this);
    _weatherStationsByICAOCode.insert(ICAOCode, newWeatherStation);
    return newWeatherStation;
}


void Meteorologist::load()
{
    auto stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/weather.dat";

    // Use LockFile. If lock could not be obtained, do nothing.
    QLockFile lockFile(stdFileName+".lock");
    if (!lockFile.tryLock())
        return;

    // Open file
    auto inputFile = QFile(stdFileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        lockFile.unlock();
        return;
    }

    // Generate input stream
    QDataStream inputStream(&inputFile);
    inputStream.setVersion(QDataStream::Qt_4_0);
    // Check magic number and version
    quint32 magic;
    inputStream >> magic;
    if (magic != (quint32)0x31415) {
        lockFile.unlock();
        return;
    }
    quint32 version;
    inputStream >> version;
    if (version != (quint32)1) {
        lockFile.unlock();
        return;
    }

    while (!inputStream.atEnd() && (inputStream.status() == QDataStream::Ok)) {
        QChar type;
        inputStream >> type;

        if (type == 'M') {
            // Read METAR
            auto metar = new Weather::METAR(inputStream, this);
            findOrConstructWeatherStation(metar->ICAOCode())->setMETAR(metar);
            continue;
        }
        if (type == 'T') {
            // Read TAF
            auto taf = new Weather::TAF(inputStream, this);
            findOrConstructWeatherStation(taf->ICAOCode())->setTAF(taf);
            continue;
        }

        // Other type? That's bad! Quit immediately!
        break;
    }

    // Ok, done
    lockFile.unlock();
    deleteExpiredMesages();
    emit weatherStationsChanged();
}


void Meteorologist::save()
{
    auto stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/weather.dat";

    // Use LockFile. If lock could not be obtained, do nothing.
    QLockFile lockFile(stdFileName+".lock");
    if (!lockFile.tryLock())
        return;

    // Open file
    auto outputFile = QSaveFile(stdFileName);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        lockFile.unlock();
        return;
    }

    // Generate output stream
    QDataStream outputStream(&outputFile);
    outputStream.setVersion(QDataStream::Qt_4_0);

    // Write magic number and version
    outputStream << (quint32)0x31415;
    outputStream << (quint32)1;

    // Write data
    foreach(auto weatherStation, _weatherStationsByICAOCode) {
        if (weatherStation.isNull())
            continue;

        if (weatherStation->hasMETAR()) {
            // Save only valid METARs that are not yet expired
            if (weatherStation->metar()->isValid() && !weatherStation->metar()->isExpired()) {
                outputStream << QChar('M');
                weatherStation->metar()->write(outputStream);
            }
        }

        if (weatherStation->hasTAF()) {
            // Save only valid TAFs that are not yet expired
            if (weatherStation->taf()->isValid() && !weatherStation->taf()->isExpired()) {
                outputStream << QChar('T');
                weatherStation->taf()->write(outputStream);
            }
        }
    }

    outputFile.commit();
    lockFile.unlock();
}


QString Meteorologist::sunInfo() const
{
    // Paranoid safety checks
    auto _satNav = SatNav::globalInstance();
    if (_satNav == nullptr)
        return QString();
    if (_satNav->status() != SatNav::OK)
        return tr("Waiting for precise position…");

    // Describe next sunset/sunrise
    QDateTime sunrise, sunset, sunriseTomorrow;

    SunSet sun;
    auto coord = _satNav->coordinate();
    auto timeZone = qRound(coord.longitude()/15.0);

    auto currentTime = QDateTime::currentDateTimeUtc();
    auto localTime = currentTime.toOffsetFromUtc(timeZone*60*60);
    auto localDate = localTime.date();

    sun.setPosition(coord.latitude(), coord.longitude(), timeZone);
    sun.setCurrentDate(localDate.year(), localDate.month(), localDate.day());

    auto sunriseTimeInMin = sun.calcSunrise();
    if (qIsFinite(sunriseTimeInMin)) {
        sunrise = localTime;
        sunrise.setTime(QTime::fromMSecsSinceStartOfDay(sunriseTimeInMin*60*1000));
        sunrise = sunrise.toOffsetFromUtc(0);
        sunrise.setTimeSpec(Qt::UTC);
    }

    auto sunsetTimeInMin = sun.calcSunset();
    if (qIsFinite(sunsetTimeInMin)) {
        sunset = localTime;
        sunset.setTime(QTime::fromMSecsSinceStartOfDay(sunsetTimeInMin*60*1000));
        sunset = sunset.toOffsetFromUtc(0);
        sunset.setTimeSpec(Qt::UTC);
    }

    localTime = localTime.addDays(1);
    localDate = localTime.date();
    sun.setCurrentDate(localDate.year(), localDate.month(), localDate.day());
    auto sunriseTomorrowTimeInMin = sun.calcSunrise();
    if (qIsFinite(sunriseTomorrowTimeInMin)) {
        sunriseTomorrow = localTime;
        sunriseTomorrow.setTime(QTime::fromMSecsSinceStartOfDay(sunriseTomorrowTimeInMin*60*1000));
        sunriseTomorrow = sunriseTomorrow.toOffsetFromUtc(0);
        sunriseTomorrow.setTimeSpec(Qt::UTC);
    }

    if (sunrise.isValid() && sunset.isValid() && sunriseTomorrow.isValid()) {
        if (currentTime < sunrise)
            return tr("SR %1, %2").arg(Clock::describePointInTime(sunrise), Clock::describeTimeDifference(sunrise));
        if (currentTime < sunset.addSecs(40*60))
            return tr("SS %1, %2").arg(Clock::describePointInTime(sunset), Clock::describeTimeDifference(sunset));
        return tr("SR %1, %2").arg(Clock::describePointInTime(sunriseTomorrow), Clock::describeTimeDifference(sunriseTomorrow));
    }
    return QString();
}


QString Meteorologist::QNHInfo() const
{
    // Paranoid safety checks
    auto _satNav = SatNav::globalInstance();
    if (_satNav == nullptr)
        return QString();

    // Find QNH of nearest airfield
    Weather::Station *closestReportWithQNH = nullptr;
    int QNH = 0;
    foreach(auto weatherStationPtr, _weatherStationsByICAOCode) {
        if (weatherStationPtr.isNull())
            continue;
        if (weatherStationPtr->metar() == nullptr)
            continue;
        QNH = weatherStationPtr->metar()->QNH();
        if (QNH == 0)
            continue;
        if (!weatherStationPtr->coordinate().isValid())
            continue;
        if (closestReportWithQNH == nullptr) {
            closestReportWithQNH = weatherStationPtr;
            continue;
        }

        QGeoCoordinate here = _satNav->lastValidCoordinate();
        if (here.distanceTo(weatherStationPtr->coordinate()) < here.distanceTo(closestReportWithQNH->coordinate()))
            closestReportWithQNH = weatherStationPtr;
    }
    if (closestReportWithQNH) {
        return tr("QNH: %1 hPa in %2, %3").arg(QNH)
                .arg(closestReportWithQNH->ICAOCode())
                .arg(Clock::describeTimeDifference(closestReportWithQNH->metar()->observationTime()));
    }
    return QString();
}


void Meteorologist::update(bool isBackgroundUpdate) {
    // Paranoid safety checks
    auto _globalSettings = GlobalSettings::globalInstance();
    if (_globalSettings == nullptr)
        return;
    if (_flightRoute.isNull())
        return;
    auto _satNav = SatNav::globalInstance();
    if (_satNav == nullptr)
        return;

    // Refuse to do anything if we are not allowed to connect to the Aviation Weather Center
    if (!_globalSettings->acceptedWeatherTerms())
        return;

    // Schedule the next update in 30 minutes from now
    _updateTimer.setInterval(30*60*1000);
    _updateTimer.start();

    // If a request is currently running, then do not update
    if (downloading()) {
        if (_backgroundUpdate && !isBackgroundUpdate) {
            _backgroundUpdate = false;
            emit backgroundUpdateChanged();
        }
        return;
    }

    // Set _backgroundUpdate and emit signal if appropriate
    if (_backgroundUpdate != isBackgroundUpdate) {
        _backgroundUpdate = isBackgroundUpdate;
        emit backgroundUpdateChanged();
    }

    // Clear old replies, if any
    qDeleteAll(_networkReplies);
    _networkReplies.clear();

    // Generate queries
    const QGeoCoordinate& position = _satNav->lastValidCoordinate();
    const QVariantList& steerpts = _flightRoute->geoPath();
    QList<QString> queries;
    if (position.isValid()) {
        queries.push_back(QString("dataSource=metars&radialDistance=85;%1,%2").arg(position.longitude()).arg(position.latitude()));
        queries.push_back(QString("dataSource=tafs&radialDistance=85;%1,%2").arg(position.longitude()).arg(position.latitude()));
    }
    if (!steerpts.empty()) {
        QString qpos;
        foreach(auto var, steerpts) {
            QGeoCoordinate posit = var.value<QGeoCoordinate>();
            qpos += ";" + QString::number(posit.longitude()) + "," + QString::number(posit.latitude());
        }
        queries.push_back(QString("dataSource=metars&flightPath=85%1").arg(qpos));
        queries.push_back(QString("dataSource=tafs&flightPath=85%1").arg(qpos));
    }

    // Fetch data
    foreach(auto query, queries) {
        QUrl url = QUrl(QString("https://www.aviationweather.gov/adds/dataserver_current/httpparam?requestType=retrieve&format=xml&hoursBeforeNow=1&mostRecentForEachStation=true&%1").arg(query));
        QNetworkRequest request(url);
        QPointer<QNetworkReply> reply = _networkAccessManager->get(request);
        _networkReplies.push_back(reply);
        connect(reply, &QNetworkReply::finished, this, &Meteorologist::downloadFinished);
        connect(reply, &QNetworkReply::errorOccurred, this, &Meteorologist::downloadFinished);
    }

    // Emit "downloading" and handle the case if none of the requests have started (e.g. because
    // no internet is available at all)
    downloadFinished();
}


QList<Weather::Station *> Meteorologist::weatherStations() const {

    // Produce a list of reports, without nullpointers
    QList<Weather::Station *> sortedReports;
    foreach(auto stations, _weatherStationsByICAOCode)
        if (!stations.isNull())
            sortedReports += stations;

    // Sort list
    auto _satNav = SatNav::globalInstance();
    if (_satNav) {
        auto compare = [&](const Weather::Station *a, const Weather::Station *b) {
            auto here = _satNav->lastValidCoordinate();
            return here.distanceTo(a->coordinate()) < here.distanceTo(b->coordinate());
        };
        std::sort(sortedReports.begin(), sortedReports.end(), compare);
    }

    return sortedReports;
}

