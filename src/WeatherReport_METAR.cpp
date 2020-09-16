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

#include <QDebug>
#include <QTimer>

#include "WeatherReport_METAR.h"


WeatherReport::METAR::METAR(QXmlStreamReader &xml, QObject *parent) : QObject(parent)
{

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
    QList<QString> accepted; // fields to decode without special treatment
    accepted = {"station_id", "observation_time", "temp_c",
                "dewpoint_c", "wind_dir_degrees", "wind_speed_kt",
                "wind_gust_kt", "visibility_statute_mi",
                "flight_category", "wx_string"};

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == "station_id") {
            _station_id = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == "latitude") {
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "longitude") {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "elevation_m") {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == "raw_text") {
            _raw_text = xml.readElementText();
            data.insert("raw_text", _raw_text);
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == "altim_in_hg") {
            auto content = xml.readElementText();
            data.insert("altim_in_hg", content);
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200))
                _qnh = 0;
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == "observation_time") {
            auto content = xml.readElementText();
            data.insert("observation_time", content);
            _observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Other data fields
        if (xml.isStartElement() && accepted.contains(name) ) {
            data.insert(name, xml.readElementText());
            continue;
        }

        if (xml.isStartElement() && name == "sky_condition") {
            data.insert("sky_condition", readSky());
            xml.skipCurrentElement();
            continue;
        }

        if (xml.isEndElement() && name == "METAR")
            break;

        xml.skipCurrentElement();
    }

    //
    // Generate DATA
    //
    if (data.contains("raw_text"))
        dataStrings.push_back("RAW " + data.value("raw_text").toString());
    if (data.contains("observation_time"))
        dataStrings.push_back("TIME" + WeatherReport::decodeTime(data.value("observation_time")));
    if (data.contains("wind_dir_degrees") && data.contains("wind_speed_kt")) {
        if (data.contains("wind_gust_kt"))
            dataStrings.push_back("WIND" + WeatherReport::decodeWind(data.value("wind_dir_degrees"), data.value("wind_speed_kt"), data.value("wind_gust_kt")));
        else
            dataStrings.push_back("WIND" + WeatherReport::decodeWind(data.value("wind_dir_degrees"), data.value("wind_speed_kt")));
    }
    if (data.contains("visibility_statute_mi"))
        dataStrings.push_back("VIS " + WeatherReport::decodeVis(data.value("visibility_statute_mi")));
    if (data.contains("wx_string"))
        dataStrings.push_back("WX  " + WeatherReport::decodeWx(data.value("wx_string")));
    if (data.contains("sky_condition"))
        dataStrings.push_back("CLDS" + WeatherReport::decodeClouds(data.values("sky_condition")));
    if (data.contains("temp_c"))
        dataStrings.push_back("TEMP" + WeatherReport::decodeTemp(data.value("temp_c")));
    if (data.contains("dewpoint_c"))
        dataStrings.push_back("DEWP" + WeatherReport::decodeTemp(data.value("dewpoint_c")));
    if (data.contains("altim_in_hg"))
        dataStrings.push_back("QNH " + WeatherReport::decodeQnh(data.value("altim_in_hg")));

    //
    // Set up self-destruction timer
    //

    if (!_observationTime.isValid())
        deleteLater();
    else {
        // Time out is 1.5 hours after observation time, unless raw text contains "NOSIG", then it is 3 hours
        auto timeOut = QDateTime::currentDateTimeUtc().msecsTo(_observationTime.addSecs(1.5*60*60));
        if (_raw_text.contains("NOSIG")) {
            timeOut = QDateTime::currentDateTimeUtc().msecsTo(_observationTime.addSecs(3*60*60));
            qWarning() << "NOSIG" << _station_id;
        }
        qWarning() << "time out" << timeOut/(1000*60);
        if (timeOut < 1000)
            deleteLater();
        else
//            QTimer::singleShot(timeOut, this, SLOT(deleteLater()));
#warning Needs to be fixed
            QTimer::singleShot(1000*20, this, SLOT(deleteLater()));
    }
}

