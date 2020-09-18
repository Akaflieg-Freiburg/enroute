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

#include "Clock.h"
#include "Meteorologist.h"
#include "WeatherReport.h"

#include <QtGlobal>
#include <QQmlEngine>
#include <QXmlStreamReader>
#include "sunset.h"

Meteorologist::Meteorologist(Clock *clock,
                             SatNav *sat,
                             FlightRoute *route,
                             GlobalSettings *globalSettings,
                             QNetworkAccessManager *networkAccessManager,
                             QObject *parent) :
    QObject(parent), _sat(sat), _route(route), _globalSettings(globalSettings),
    _networkAccessManager(networkAccessManager)
{
    // Connect the timer to the update method. This will set backgroundUpdate to the default value,
    // which is true. So these updates happen in the background.
    connect(&_updateTimer, &QTimer::timeout, [=](){ this->update();});

    // Schedule the first update in 10 seconds from now
    _updateTimer.setInterval(10*1000);
    _updateTimer.start();

    // Update the description text when needed
    connect(this, &Meteorologist::reportsChanged, this, &Meteorologist::QNHInfoChanged);
    connect(clock, &Clock::timeChanged, this, &Meteorologist::QNHInfoChanged);
    connect(sat, &SatNav::statusChanged, this, &Meteorologist::QNHInfoChanged);
    connect(clock, &Clock::timeChanged, this, &Meteorologist::SunInfoChanged);
    connect(sat, &SatNav::statusChanged, this, &Meteorologist::SunInfoChanged);
}


Meteorologist::~Meteorologist()
{
    qDeleteAll(_reports);
    _reports.clear();

    qDeleteAll(_replies);
    _replies.clear();
}


QList<QObject *> Meteorologist::reports() const {

    // Produce a list of reports, without nullpointers
    QList<WeatherReport *> sortedReports;
    foreach(auto rep, _reports)
        if (!rep.isNull())
            sortedReports += rep;

    // Sort list
    auto compare = [&](WeatherReport *a, WeatherReport *b) {
        auto here = _sat->lastValidCoordinate();
        return here.distanceTo(a->location()) < here.distanceTo(b->location());
    };
    std::sort(sortedReports.begin(), sortedReports.end(), compare);

    // Convert to QObjectList
    QList<QObject *> result;
    foreach(auto rep, sortedReports)
        result += rep;
    return result;
}


void Meteorologist::update(bool isBackgroundUpdate) {
    // Paranoid safety checks
    if (_globalSettings.isNull())
        return;
    if (_route.isNull())
        return;
    if (_sat.isNull())
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
    qDeleteAll(_replies);
    _replies.clear();

    // Generate queries
    const QGeoCoordinate& position = _sat->lastValidCoordinate();
    const QVariantList& steerpts = _route->geoPath();
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
        _replies.push_back(reply);
        connect(reply, &QNetworkReply::finished, this, &Meteorologist::downloadFinished);
        connect(reply, &QNetworkReply::errorOccurred, this, &Meteorologist::downloadFinished);
    }

    // Emit "downloading" and handle the case if none of the requests have started (e.g. because
    // no internet is available at all)
    downloadFinished();
}


void Meteorologist::downloadFinished() {
    // Update flag
    emit downloadingChanged();

    // Start to process the data only once ALL replies have been received. So, we check here if there are any running
    // download processes and abort if indeed there are some.
    if (downloading())
        return;
    process();
}


void Meteorologist::process() {
    // Check if some network replies contain errors. If so, emit error, completely ignore everything contained in any of the replies and abort
    bool hasError = false;
    foreach(auto rep, _replies)
        if (rep->error() != QNetworkReply::NoError) {
            emit error(rep->errorString());
            hasError = true;
            break;
        }
    if (hasError) {
        qDeleteAll(_replies);
        _replies.clear();
        return;
    }


    // These maps associate the weather station ID to its METAR/TAF replies
    QMap<QString, QPointer<WeatherReport::METAR>> metars;
    QMap<QString, QPointer<WeatherReport::TAF>> tafs;
    // These lists contain the weather station ID and will be used to handle duplicate or unpaired stations
    QList<QString> mStations;
    QList<QString> tStations;
    // Read all replies and store the data in respective maps
    foreach(auto rep, _replies) {

        // Decode XML
        QXmlStreamReader xml(rep);
        while (!xml.atEnd() && !xml.hasError())
        {
            auto token = xml.readNext();
            if (!token)
                break;
            
            if (xml.isStartElement()) {
                // Read the METAR and get the data, if the station has not been encountered yet
                if (xml.name() == "METAR") {

                    auto metar = new WeatherReport::METAR(xml, this);
                    if (!mStations.contains(metar->_station_id)) {
                        mStations.push_back(metar->_station_id);
                        metars.insert(metar->_station_id, metar);
                    } else
                        delete metar;
                }

                // Read the TAF and get the data, if the station has not been encountered yet
                if (xml.name() == "TAF") {

                    auto taf = new WeatherReport::TAF(xml, this);
                    if (!tStations.contains(taf->_station_id)) {
                        tStations.push_back(taf->_station_id);
                        tafs.insert(taf->_station_id, taf);
                    } else
                        delete taf;
                }

            }
        }
    }

    // Clear old reports, if any. We disconnect first, to avoid repeated emissions of the signal reportsChanged
    foreach(auto report, _reports) {
        if (report.isNull())
            continue;
        disconnect(report, &QObject::destroyed, this, &Meteorologist::reportsChanged);
        report->deleteLater();
    }
    _reports.clear();

    // Add new reports and handle unpaired METAR/TAF
    mStations.sort();
    tStations.sort();
    foreach(auto station, mStations) {
        // Station has both METAR and TAF
        if (tafs.contains(station)) {
            _reports.append(new WeatherReport(station, metars.value(station), tafs.value(station)));
            int i = tStations.indexOf(station);
            tStations.removeAt(i);
        }
        // Station only has METAR
        else
            _reports.append(new WeatherReport(station, metars.value(station), nullptr)); // empty TAF
    }
    // Stations only have TAF
    foreach(auto station, tStations)
        _reports.append(new WeatherReport(station, nullptr, tafs.value(station))); // empty METAR

    // Report change of reports when weather reports start to auto-delete themsleves
    foreach(auto report, _reports) {
        if (report.isNull())
            continue;
        connect(report, &QObject::destroyed, this, &Meteorologist::reportsChanged);
    }

    // Clear replies container
    qDeleteAll(_replies);
    _replies.clear();

    // Update flag and signals
    emit reportsChanged();

    _lastUpdate = QDateTime::currentDateTimeUtc();
    emit QNHInfoChanged();
}


bool Meteorologist::downloading() const
{
    foreach(auto reply, _replies)
        if (reply->isRunning())
            return true;
    return false;
}


QString Meteorologist::QNHInfo() const
{
    // Paranoid safety checks
    if (_sat.isNull())
        return QString();

    // Find QNH of nearest airfield
    WeatherReport *closestReportWithQNH = nullptr;
    foreach(auto report, _reports) {
        if (report.isNull())
            continue;
        if (report->qnh() == 0)
            continue;
        if (!report->location().isValid())
            continue;
        if (closestReportWithQNH == nullptr) {
            closestReportWithQNH = report;
            continue;
        }

        QGeoCoordinate here = _sat->lastValidCoordinate();
        if (here.distanceTo(report->location()) < here.distanceTo(closestReportWithQNH->location()))
            closestReportWithQNH = report;
    }
    if (closestReportWithQNH) {
        return tr("QNH: %1 hPa in %2, %3").arg(closestReportWithQNH->qnh())
                .arg(closestReportWithQNH->station_id())
                .arg(Clock::describeTimeDifference(closestReportWithQNH->metar()->_observationTime));
    }
    return QString();
}


QString Meteorologist::SunInfo() const
{
    // Paranoid safety checks
    if (_sat.isNull())
        return QString();
    if (_sat->status() != SatNav::OK)
        return tr("Waiting for precise position…");

    // Describe next sunset/sunrise
    QDateTime sunrise, sunset, sunriseTomorrow;

    SunSet sun;
    auto coord = _sat->coordinate();
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
    qWarning() << "A" << localTime;
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
            return tr("SR %1, %2").arg(Clock::describePointInTime(sunrise, coord), Clock::describeTimeDifference(sunrise));
        if (currentTime < sunset.addSecs(40*60))
            return tr("SS %1, %2").arg(Clock::describePointInTime(sunset, coord), Clock::describeTimeDifference(sunset));
        return tr("SR %1, %2").arg(Clock::describePointInTime(sunriseTomorrow, coord), Clock::describeTimeDifference(sunriseTomorrow));
    }
    return QString();
}
