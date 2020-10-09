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
#include <QXmlStreamAttribute>

#include "Clock.h"
#include "Meteorologist_TAF.h"
#include "Meteorologist_WeatherStation.h"


Meteorologist::TAF::TAF(QXmlStreamReader &xml, Clock *clock, QObject *parent)
    : QObject(parent),
      _clock(clock)
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
    accepted = {"raw_text", "station_id", "issue_time",
                "valid_time_from", "valid_time_to"};

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == "station_id") {
            _ICAOCode = xml.readElementText();
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

        if (xml.isStartElement() && accepted.contains(name) )
            data.insert(name, xml.readElementText());
        else if (xml.isStartElement() && name == "forecast") {
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
            data.insert("forecast", QVariant(forecast));
        }
        else if (xml.isEndElement() && name == "TAF")
            break;
        else
            xml.skipCurrentElement();
    }

    // Generate DATA
    if (data.contains("raw_text"))
        dataStrings.push_back("RAW " + data.value("raw_text").toString());
    if (data.contains("issue_time"))
        dataStrings.push_back("TIME" + Meteorologist::WeatherStation::decodeTime(data.value("issue_time")));
    if (data.contains("valid_time_from"))
        dataStrings.push_back("FROM" + Meteorologist::WeatherStation::decodeTime(data.value("valid_time_from")));
    if (data.contains("valid_time_to"))
        dataStrings.push_back("TO  " + Meteorologist::WeatherStation::decodeTime(data.value("valid_time_to")));
    if (data.contains("forecast")) {
        for (int i = data.values("forecast").size() - 1; i >= 0; --i) {
            QMultiMap<QString, QVariant> forecast = data.values("forecast")[i].toMap();
            QString fcst;
            if (forecast.contains("fcst_time_from") && forecast.contains("fcst_time_to")) {
                if (!forecast.contains("change_indicator"))
                    fcst += "From " +Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_from")) + " to " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else if (forecast.value("change_indicator").toString() == "TEMPO")
                    fcst += "Temporary from " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_from")) + " to " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else if (forecast.value("change_indicator").toString() == "BECMG")
                    fcst += "Transition from " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_from")) + " to " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else
                    fcst += "From " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_from")) + " to " + Meteorologist::WeatherStation::decodeTime(forecast.value("fcst_time_to")) + "<br>";

            }
            if (forecast.contains("probability"))
                fcst.push_back(forecast.value("probability").toString() + "% probability<br>");
            if (forecast.contains("wind_dir_degrees") && forecast.contains("wind_speed_kt"))
                fcst.push_back("Wind " + Meteorologist::WeatherStation::decodeWind(forecast.value("wind_dir_degrees"), forecast.value("wind_speed_kt")) + "<br>");
            if (forecast.contains("visibility_statute_mi"))
                fcst.push_back("Visibility " + Meteorologist::WeatherStation::decodeVis(forecast.value("visibility_statute_mi")) + "<br>");
            if (forecast.contains("wx_string"))
                fcst.push_back(Meteorologist::WeatherStation::decodeWx(forecast.value("wx_string")) + "<br>");
            if (forecast.contains("sky_condition"))
                fcst.push_back(Meteorologist::WeatherStation::decodeClouds(forecast.values("sky_condition")));
            dataStrings.push_back("FCST" + fcst);
        }
    }

}


Meteorologist::TAF::TAF(QDataStream &inputStream, Clock *clock, QObject *parent)
    : QObject(parent),
      _clock(clock)
{
    inputStream >> _expirationTime;
    inputStream >> _ICAOCode;
    inputStream >> _issueTime;
    inputStream >> _location;
    inputStream >> _raw_text;
}


QString Meteorologist::TAF::decodedText() const
{
#warning not implemented
    return "This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF -- This is a clear text presentation of the TAF";
}


QDateTime Meteorologist::TAF::expiration() const
{
#warning not implemented
    return QDateTime();
}


bool Meteorologist::TAF::isExpired() const
{
    auto exp = expiration();
    if (!exp.isValid())
        return false;
    return QDateTime::currentDateTime() > exp;
}


bool Meteorologist::TAF::isValid() const
{
#warning not implemented
    return true;
}


QString Meteorologist::TAF::relativeIssueTime() const
{
#warning not implemented
    return "not implemented";
}


QDataStream &operator<<(QDataStream &outputStream, const Meteorologist::TAF &taf)
{
    outputStream << taf._expirationTime;
    outputStream << taf._ICAOCode;
    outputStream << taf._issueTime;
    outputStream << taf._location;
    outputStream << taf._raw_text;
    return outputStream;
}
