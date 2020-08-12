/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include "Meteorologist.h"
#include "WeatherReport.h"

#include <QtGlobal>
#include <QQmlEngine>
#include <QXmlStreamReader>

Meteorologist::Meteorologist(SatNav *sat, FlightRoute *route,
                             QNetworkAccessManager *networkAccessManager,
                             QObject *parent) :
                             QObject(parent), _sat(sat), _route(route),
                             _networkAccessManager(networkAccessManager),
                             _nReply(0), _nQuery(0), _processing(false),
                             _autoUpdate(false) {
    // Create a 30 minutes timer and connect it to the update method
    this->_timer = new QTimer(this);
    _timer->setInterval(1800000);
    connect(_timer, &QTimer::timeout, this, &Meteorologist::update);
}

Meteorologist::~Meteorologist()
{
    foreach(auto rep, _reports)
        delete rep;
    _reports.clear();
    foreach(auto rep, _replies)
        delete rep;
    _replies.clear();
    delete _timer;
}

QList<QObject *> Meteorologist::reports() const {
    QList<QObject *> reports;
    foreach(auto rep, _reports)
        reports.append(rep);
    return reports;
}

void Meteorologist::setAutoUpdate(bool autoUpdt) {
    // Sanity check
    if (autoUpdt == autoUpdate())
        return;
    // Set the property and start/stop the auto-update timer
    _autoUpdate = autoUpdt;
    if (_autoUpdate) {
        if (!_processing)
            this->update();
        _timer->start();
    }
    else
        _timer->stop();

    emit autoUpdateChanged();
}

void Meteorologist::update() {
    // Update flag
    _processing = true;
    emit processingChanged();
    // Clear old replies, if any
    foreach(auto rep, _replies)
        delete rep;
    _replies.clear();
    _nReply = 0;
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
    _nQuery = queries.size();
    // Fetch data
    foreach(auto query, queries) {
        QUrl url = QUrl(QString("https://www.aviationweather.gov/adds/dataserver_current/httpparam?requestType=retrieve&format=xml&hoursBeforeNow=1&mostRecentForEachStation=true&%1").arg(query));
        QNetworkRequest request(url);
        QPointer<QNetworkReply> reply = _networkAccessManager->get(request);
        _replies.push_back(reply);
        connect(reply, &QNetworkReply::finished, this, &Meteorologist::downloadFinished);
    }
}

void Meteorologist::downloadFinished() {
    // Start to process the data once ALL replies have been received
    _nReply += 1;
    if (_nReply == _nQuery)
        this->process();    
}

void Meteorologist::process() {
    // These maps associate the weather station ID to its METAR/TAF replies
    QMap<QString, QMultiMap<QString, QVariant>> metars;
    QMap<QString, QMultiMap<QString, QVariant>> tafs;
    // These lists contain the weather station ID and will be used to handle duplicate or unpaired stations
    QList<QString> mStations;
    QList<QString> tStations;
    // Read all replies and store the data in respective maps
    foreach(auto rep, _replies) {
        // Handle network error, if any
        if (rep->error() != QNetworkReply::NoError) {
            emit error(rep->errorString());
            break;
        }
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
                    auto metar = this->readReport(xml, "METAR");
                    if (metar.contains("station_id")) {
                        QString station = metar.value("station_id").toString();
                        if (!mStations.contains(station)) {
                            mStations.push_back(station);
                            metars.insert(station, metar);
                        }
                    }
                }
                // Read the TAF and get the data, if the station has not been encountered yet
                else if (xml.name() == "TAF") {
                    auto taf = this->readReport(xml, "TAF");
                    if (taf.contains("station_id")) {
                        QString station = taf.value("station_id").toString();
                        if (!tStations.contains(station)) {
                            tStations.push_back(station);
                            tafs.insert(station, taf);
                        }
                    }
                }
            }
        }
    }
    // Clear old reports, if any
    foreach(auto rep, _reports)
        delete rep;
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
            _reports.append(new WeatherReport(station, metars.value(station), QMultiMap<QString, QVariant>())); // empty TAF
    }
    // Stations only have TAF
    foreach(auto station, tStations)
        _reports.append(new WeatherReport(station, QMultiMap<QString, QVariant>(), tafs.value(station))); // empty METAR

    // Clear replies container
    foreach(auto rep, _replies)
        delete rep;
    _replies.clear();

    // Update flag and signals
    _processing = false;
    emit processingChanged();
    emit reportsChanged();
}

QMultiMap<QString, QVariant> Meteorologist::readReport(QXmlStreamReader &xml, const QString &type) {
    // Lambda to read sky condition
    auto readSky = [&] {
        QXmlStreamAttributes atrs = xml.attributes();
        QString sky;
        sky += atrs.value("sky_cover").toString();
        if (atrs.value("cloud_base_ft_agl").toString() != "")
            sky += "," + atrs.value("cloud_base_ft_agl").toString();
        if (atrs.value("cloud_type").toString() != "")
            sky += "," + atrs.value("cloud_type").toString();
        return sky;
    };

    // Read METAR/TAF and store in map
    QMultiMap<QString, QVariant> report;
    QList<QString> accepted; // fields to decode without special treatment
    if (type == "METAR")
        accepted = {"raw_text", "station_id", "observation_time", "temp_c",
                    "dewpoint_c", "wind_dir_degrees", "wind_speed_kt",
                    "wind_gust_kt", "visibility_statute_mi", "altim_in_hg",
                    "flight_category", "wx_string"};
    else if (type == "TAF")
        accepted = {"raw_text", "station_id", "issue_time",
                    "valid_time_from", "valid_time_to"};
    else
        throw std::runtime_error("Meteorologist::_readReport: type must be METAR or TAF!\n");

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        if (xml.isStartElement() && accepted.contains(name) )
            report.insert(name, xml.readElementText());
        else if (xml.isStartElement() && name == "sky_condition" && type == "METAR") {
            report.insert("sky_condition", readSky());
            xml.skipCurrentElement();
        }
        else if (xml.isStartElement() && name == "forecast" && type == "TAF") {
            QMultiMap<QString, QVariant> forecast;
            QList<QString> accepted2 = {"fcst_time_from", "fcst_time_to",
                                        "change_indicator", "probability",
                                        "wind_dir_degrees", "wind_speed_kt",
                                        "wind_gust_kt",
                                        "visibility_statute_mi",
                                        "wx_string"};

            while (true) {
                xml.readNextStartElement();
                QString name2 = xml.name().toString();
                
                if (xml.isStartElement() && accepted2.contains(name2))
                    forecast.insert(name2, xml.readElementText());
                else if (xml.isStartElement() && name2 == "sky_condition") {
                    forecast.insert("sky_condition", readSky());
                    xml.skipCurrentElement();
                }
                else if (xml.isEndElement() && name2 == "forecast")
                    break;
                else
                    xml.skipCurrentElement();
            }
            report.insert("forecast", QVariant(forecast));
        }
        else if (xml.isEndElement() && name == type)
            break;
        else
            xml.skipCurrentElement();   
    }
    return report;
}
