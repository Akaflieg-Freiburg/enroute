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

Meteorologist::Meteorologist(QNetworkAccessManager *networkAccessManager,
                             QObject *parent) :
                             QObject(parent), _networkAccessManager(networkAccessManager),
                             _replyCount(0), _replyTotal(0), _processing(false) {
}

Meteorologist::~Meteorologist()
{
    for (auto rep : _reports)
        delete rep;
    _reports.clear();
    for (auto rep : _replies)
        delete rep;
    _replies.clear();
}

QList<QObject *> Meteorologist::reports() const {
    QList<QObject *> reports;
    for (auto rep : _reports)
        reports.append(rep);
    return reports;
}

void Meteorologist::update(const QGeoCoordinate& position, const QVariantList& steerpts) {
    // Update signals
    _processing = true;
    emit processingChanged();
    // Clear old replies, if any
    for (auto rep : _replies)
        delete rep;
    _replies.clear();
    _replyCount = 0;
    // Generate queries
    QList<QString> queries;
    if (position.isValid()) {
        queries.push_back(QString("dataSource=metars&radialDistance=85;%1,%2").arg(position.longitude()).arg(position.latitude()));
        queries.push_back(QString("dataSource=tafs&radialDistance=85;%1,%2").arg(position.longitude()).arg(position.latitude()));
    }
    if (!steerpts.empty()) {
        QString qpos;
        for (auto var : steerpts) {
            QGeoCoordinate posit = var.value<QGeoCoordinate>();
            qpos += ";" + QString::number(posit.longitude()) + "," + QString::number(posit.latitude());
        }
        queries.push_back(QString("dataSource=metars&flightPath=85%1").arg(qpos));
        queries.push_back(QString("dataSource=tafs&flightPath=85%1").arg(qpos));
    }
    _replyTotal = queries.size();
    // Fetch
    for (auto query : queries) {
        QUrl url = QUrl(QString("https://www.aviationweather.gov/adds/dataserver_current/httpparam?requestType=retrieve&format=xml&hoursBeforeNow=1&mostRecentForEachStation=true&%1").arg(query));
        QNetworkRequest request(url);
        QPointer<QNetworkReply> reply = _networkAccessManager->get(request);
        _replies.push_back(reply);
        connect(reply, &QNetworkReply::finished, this, &Meteorologist::downloadFinished);
    }
}

void Meteorologist::decode() {
    // Decode METAR and TAF and handle duplicates
    QMap<QString, QMultiMap<QString, QVariant>> metars;
    QMap<QString, QMultiMap<QString, QVariant>> tafs;
    QList<QString> mStations;
    QList<QString> tStations;
    for (auto rep : _replies) {
        if (rep->error() != QNetworkReply::NoError) {
            emit error(rep->errorString()); // todo Reference error: message not defined in QML
            break;
        }
        
        QXmlStreamReader xml(rep);
        while (!xml.atEnd() && !xml.hasError())
        {
            auto token = xml.readNext();
            if (!token)
                break;
            
            if (xml.isStartElement()) {
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
    for (auto rep : _reports)
        delete rep;
    _reports.clear();

    // Add new reports and handle unpaired METAR/TAF
    mStations.sort();
    tStations.sort();
    for (auto station : mStations) {
        if (tafs.contains(station)) {
            _reports.append(new WeatherReport(station, metars.value(station), tafs.value(station)));
            int i = tStations.indexOf(station);
            tStations.removeAt(i);
        }
        else
            _reports.append(new WeatherReport(station, metars.value(station), QMultiMap<QString, QVariant>())); // empty TAF
    }
    for (auto station : tStations)
        _reports.append(new WeatherReport(station, QMultiMap<QString, QVariant>(), tafs.value(station))); // empty METAR

    // Clear replies container
    for (auto rep: _replies)
        delete rep;
    _replies.clear();

    // Update signals
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

    // Read METAR/TAF
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

void Meteorologist::downloadFinished() {
    _replyCount += 1;
    if (_replyCount == _replyTotal)
        this->decode();    
}
