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

Meteorologist::Meteorologist(const QGeoCoordinate& position, QObject *parent) : QObject(parent), _updated(false) {
    if (position.isValid() && true) // true to be replaced by privacy permission
        this->update(position);
}

/* todo where to free memory */
/*Meteorologist::~Meteorologist()
{
    for (auto report : _reports)
        delete report;
}*/

QList<QObject *> Meteorologist::reports() const {
    QList<QObject *> reports;
    for (auto rep : _reports)
        reports.append(rep);
    return reports;
}

void Meteorologist::update(const QGeoCoordinate& position) {

    // Generate dummy XML report
    QList<QString> responses;
    responses.push_back(this->dummyMetars());
    responses.push_back(this->dummyTafs());

    QList<QMultiMap<QString, QVariant>> metars;
    QList<QMultiMap<QString, QVariant>> tafs;
    
    // Decode METAR and TAF
    for (auto response : responses) {
        QXmlStreamReader xml(response);
        while (!xml.atEnd() && !xml.hasError())
        {
            auto token = xml.readNext();
            if (!token)
                break;

            // IF METAR and TAF contains duplicate => lambda read(QString METAR || TAF)
            if (xml.isStartElement() && xml.name() == "METAR") {

                QMultiMap<QString, QVariant> metar;
                QList<QString> accepted = {"raw_text", "station_id", "observation_time",
                                            "temp_c", "dewpoint_c", "wind_dir_degrees",
                                            "wind_speed_kt", "visibility_statute_mi",
                                            "altim_in_hg", "flight_category",
                                            "wx_string"};
                while (true) {
                    xml.readNextStartElement();
                    QString name = xml.name().toString();

                    if (xml.isStartElement() && accepted.contains(name) )
                        metar.insert(name, xml.readElementText());
                    else if (xml.isStartElement() && name == "sky_condition") {
                        auto atrs = xml.attributes();
                        metar.insert("sky_condition", atrs.value("sky_cover").toString() + ", " + atrs.value("cloud_base_ft_agl").toString());
                        xml.skipCurrentElement();
                    }
                    else if (xml.isEndElement() && name == "METAR")
                        break;
                    else
                        xml.skipCurrentElement();   
                }

                metars.push_back(metar);
            }
            else if (xml.isStartElement() && xml.name() == "TAF") {

                QMultiMap<QString, QVariant> taf;
                QList<QString> accepted = {"raw_text", "station_id", "issue_time",
                                            "valid_time_from", "valid_time_to"};

                while (true) {
                    xml.readNextStartElement();
                    QString name = xml.name().toString();

                    if (xml.isStartElement() && accepted.contains(name) )
                        taf.insert(name, xml.readElementText());
                    else if (xml.isStartElement() && name == "forecast") {
                        
                        QMultiMap<QString, QVariant> forecast;
                        QList<QString> accepted2 = {"fcst_time_from", "fcst_time_to",
                                                    "wind_dir_degrees", 
                                                    "wind_speed_kt",
                                                    "visibility_statute_mi",
                                                    "wx_string", "probability",
                                                    "change_indicator"};

                        while (true) {
                            xml.readNextStartElement();
                            QString name2 = xml.name().toString();
                            
                            if (xml.isStartElement() && accepted2.contains(name2))
                                forecast.insert(name2, xml.readElementText());
                            else if (xml.isStartElement() && name2 == "sky_condition") {
                                auto atrs = xml.attributes();
                                forecast.insert("sky_condition", atrs.value("sky_cover").toString() + ", " + atrs.value("cloud_base_ft_agl").toString());
                                xml.skipCurrentElement();
                            }
                            else if (xml.isEndElement() && name2 == "forecast")
                                break;
                            else
                                xml.skipCurrentElement();
                        }
                        taf.insert("forecast", QVariant(forecast));
                    }
                    else if (xml.isEndElement() && name == "TAF")
                        break;
                    else
                        xml.skipCurrentElement();
                }
                tafs.push_back(taf);
            }
        }
    }
    // Clear old reports, if any
    for (auto rep : _reports)
        delete rep;
    _reports.clear();

    // Add new reports and handle duplicates
    /*todo associate METAR and TAF for all stations -> use QMultiMap instead of QList */
    std::string errorm = "METAR and TAF not associated\n";
    if (metars.size() == tafs.size()) {
        QList<QString> stations;
        for (size_t i = 0; i < metars.size(); ++i) {
            if (metars[i].value("station_id") == tafs[i].value("station_id")) {
                QString station = metars[i].value("station_id").toString();
                if (!stations.contains(station)) {
                    stations.push_back(station);
                    _reports.append(new WeatherReport(metars[i], tafs[i]));
                }
            }
            else
                throw std::runtime_error(errorm);
        }
    }
    else
        throw std::runtime_error(errorm);

    // Update signals and flags
    _updated = true;
    emit reportsChanged();
}

/* These methods are dummies that simulate the result of a sucessfull request to the aviation weather server */

QString Meteorologist::dummyMetars() {
    return QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XML-Schema-instance\" version=\"1.2\" xsi:noNamespaceSchemaLocation=\"http://aviationweather.gov/adds/schema/metar1_2.xsd\">"
  "<request_index>175178316</request_index>"
  "<data_source name=\"metars\" />"
  "<request type=\"retrieve\" />"
  "<errors />"
  "<warnings />"
  "<time_taken_ms>169</time_taken_ms>"
  "<metar num_results=\"6\">"
    "<METAR>"
      "<raw_text>EBFN 160925Z 32010KT 9999 FEW014 SCT020 BKN040 16/14 Q1018 WHT WHT TEMPO BLU</raw_text>"
      "<station_id>EBFN</station_id>"
      "<observation_time>2020-07-16T09:25:00Z</observation_time>"
      "<latitude>51.07</latitude>"
      "<longitude>2.63</longitude>"
      "<temp_c>16.0</temp_c>"
      "<dewpoint_c>14.0</dewpoint_c>"
      "<wind_dir_degrees>320</wind_dir_degrees>"
      "<wind_speed_kt>10</wind_speed_kt>"
      "<visibility_statute_mi>6.21</visibility_statute_mi>"
      "<altim_in_hg>30.059055</altim_in_hg>"
      "<sky_condition sky_cover=\"FEW\" cloud_base_ft_agl=\"1400\" />"
      "<sky_condition sky_cover=\"SCT\" cloud_base_ft_agl=\"2000\" />"
      "<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"4000\" />"
      "<flight_category>VFR</flight_category>"
      "<metar_type>METAR</metar_type>"
      "<elevation_m>9.0</elevation_m>"
    "</METAR>"
    "<METAR>"
      "<raw_text>EBOS 160920Z 32012KT 9999 FEW012 SCT049 17/13 Q1019 NOSIG</raw_text>"
      "<station_id>EBOS</station_id>"
      "<observation_time>2020-07-16T09:20:00Z</observation_time>"
      "<latitude>51.2</latitude>"
      "<longitude>2.87</longitude>"
      "<temp_c>17.0</temp_c>"
      "<dewpoint_c>13.0</dewpoint_c>"
      "<wind_dir_degrees>320</wind_dir_degrees>"
      "<wind_speed_kt>12</wind_speed_kt>"
      "<visibility_statute_mi>6.21</visibility_statute_mi>"
      "<altim_in_hg>30.088583</altim_in_hg>"
      "<quality_control_flags>"
      "  <no_signal>TRUE</no_signal>"
      "</quality_control_flags>"
      "<sky_condition sky_cover=\"FEW\" cloud_base_ft_agl=\"1200\" />"
      "<sky_condition sky_cover=\"SCT\" cloud_base_ft_agl=\"4900\" />"
      "<flight_category>VFR</flight_category>"
      "<metar_type>METAR</metar_type>"
      "<elevation_m>5.0</elevation_m>"
    "</METAR>"
    "<METAR>"
      "<raw_text>ELLX 160920Z 25010KT 220V280 3000 -DZ BKN008 BKN036 14/12 Q1019 NOSIG</raw_text>"
      "<station_id>ELLX</station_id>"
      "<observation_time>2020-07-16T09:20:00Z</observation_time>"
      "<latitude>49.63</latitude>"
      "<longitude>6.2</longitude>"
      "<temp_c>14.0</temp_c>"
      "<dewpoint_c>12.0</dewpoint_c>"
      "<wind_dir_degrees>250</wind_dir_degrees>"
      "<wind_speed_kt>10</wind_speed_kt>"
      "<visibility_statute_mi>1.86</visibility_statute_mi>"
      "<altim_in_hg>30.088583</altim_in_hg>"
      "<quality_control_flags>"
        "<no_signal>TRUE</no_signal>"
      "</quality_control_flags>"
      "<wx_string>-DZ</wx_string>"
      "<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"800\" />"
      "<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"3600\" />"
      "<flight_category>IFR</flight_category>"
      "<metar_type>METAR</metar_type>"
      "<elevation_m>379.0</elevation_m>"
    "</METAR>"
  "</metar>"
"</response>");
}

QString Meteorologist::dummyTafs() {
    return QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<response xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XML-Schema-instance\" version=\"1.2\" xsi:noNamespaceSchemaLocation=\"http://aviationweather.gov/adds/schema/taf1_2.xsd\">"
"<request_index>175171098</request_index>"
"<data_source name=\"tafs\" />"
"<request type=\"retrieve\" />"
"<errors />"
"<warnings />"
"<time_taken_ms>140</time_taken_ms>"
"<data num_results=\"6\">"
"<TAF>"
"<raw_text>TAF EBFN 160541Z 1607/1616 29010KT 9999 BKN013 OVC020 TEMPO 1607/1612 5000 -RA BKN010 OVC013 BECMG 1613/1615 32007KT 9999 SCT015 BKN020</raw_text>"
"<station_id>EBFN</station_id>"
"<issue_time>2020-07-16T05:41:00Z</issue_time>"
"<bulletin_time>2020-07-16T05:52:00Z</bulletin_time>"
"<valid_time_from>2020-07-16T07:00:00Z</valid_time_from>"
"<valid_time_to>2020-07-16T16:00:00Z</valid_time_to>"
"<latitude>51.07</latitude>"
"<longitude>2.63</longitude>"
"<elevation_m>9.0</elevation_m>"
"<forecast>"
"<fcst_time_from>2020-07-16T07:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T13:00:00Z</fcst_time_to>"
"<wind_dir_degrees>290</wind_dir_degrees>"
"<wind_speed_kt>10</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1300\" />"
"<sky_condition sky_cover=\"OVC\" cloud_base_ft_agl=\"2000\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T07:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T12:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<visibility_statute_mi>3.11</visibility_statute_mi>"
"<wx_string>-RA</wx_string>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1000\" />"
"<sky_condition sky_cover=\"OVC\" cloud_base_ft_agl=\"1300\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T13:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T16:00:00Z</fcst_time_to>"
"<change_indicator>BECMG</change_indicator>"
"<time_becoming>2020-07-16T15:00:00Z</time_becoming>"
"<wind_dir_degrees>320</wind_dir_degrees>"
"<wind_speed_kt>7</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"SCT\" cloud_base_ft_agl=\"1500\" />"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"2000\" />"
"</forecast>"
"</TAF>"
"<TAF>"
"<raw_text>TAF EBOS 160520Z 1606/1712 30011KT 9999 BKN028 TEMPO 1606/1615 4000 RA RADZ BKN012 PROB30 TEMPO 1622/1704 24005KT PROB30 TEMPO 1708/1710 BKN012</raw_text>"
"<station_id>EBOS</station_id>"
"<issue_time>2020-07-16T05:20:00Z</issue_time>"
"<bulletin_time>2020-07-16T05:00:00Z</bulletin_time>"
"<valid_time_from>2020-07-16T06:00:00Z</valid_time_from>"
"<valid_time_to>2020-07-17T12:00:00Z</valid_time_to>"
"<latitude>51.2</latitude>"
"<longitude>2.87</longitude>"
"<elevation_m>5.0</elevation_m>"
"<forecast>"
"<fcst_time_from>2020-07-16T06:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-17T12:00:00Z</fcst_time_to>"
"<wind_dir_degrees>300</wind_dir_degrees>"
"<wind_speed_kt>11</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"2800\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T06:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T15:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<visibility_statute_mi>2.49</visibility_statute_mi>"
"<wx_string>RA RA DZ</wx_string>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1200\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T22:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-17T04:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<probability>30</probability>"
"<wind_dir_degrees>240</wind_dir_degrees>"
"<wind_speed_kt>5</wind_speed_kt>"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-17T08:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-17T10:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<probability>30</probability>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1200\" />"
"</forecast>"
"</TAF>"
"<TAF>"
"<raw_text>TAF ELLX 160500Z 1606/1712 25005KT 9999 BKN030 BECMG 1609/1611 BKN015 TEMPO 1609/1618 4500 -RADZ BKN010 BECMG 1618/1620 33005KT SCT030 TEMPO 1700/1708 6000 BKN008</raw_text>"
"<station_id>ELLX</station_id>"
"<issue_time>2020-07-16T05:00:00Z</issue_time>"
"<bulletin_time>2020-07-16T05:00:00Z</bulletin_time>"
"<valid_time_from>2020-07-16T06:00:00Z</valid_time_from>"
"<valid_time_to>2020-07-17T12:00:00Z</valid_time_to>"
"<latitude>49.63</latitude>"
"<longitude>6.2</longitude>"
"<elevation_m>379.0</elevation_m>"
"<forecast>"
"<fcst_time_from>2020-07-16T06:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T09:00:00Z</fcst_time_to>"
"<wind_dir_degrees>250</wind_dir_degrees>"
"<wind_speed_kt>5</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"3000\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T09:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T18:00:00Z</fcst_time_to>"
"<change_indicator>BECMG</change_indicator>"
"<time_becoming>2020-07-16T11:00:00Z</time_becoming>"
"<wind_dir_degrees>250</wind_dir_degrees>"
"<wind_speed_kt>5</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1500\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T09:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-16T18:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<visibility_statute_mi>2.8</visibility_statute_mi>"
"<wx_string>-RA -DZ</wx_string>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"1000\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-16T18:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-17T12:00:00Z</fcst_time_to>"
"<change_indicator>BECMG</change_indicator>"
"<time_becoming>2020-07-16T20:00:00Z</time_becoming>"
"<wind_dir_degrees>330</wind_dir_degrees>"
"<wind_speed_kt>5</wind_speed_kt>"
"<visibility_statute_mi>6.21</visibility_statute_mi>"
"<sky_condition sky_cover=\"SCT\" cloud_base_ft_agl=\"3000\" />"
"</forecast>"
"<forecast>"
"<fcst_time_from>2020-07-17T00:00:00Z</fcst_time_from>"
"<fcst_time_to>2020-07-17T08:00:00Z</fcst_time_to>"
"<change_indicator>TEMPO</change_indicator>"
"<visibility_statute_mi>3.73</visibility_statute_mi>"
"<sky_condition sky_cover=\"BKN\" cloud_base_ft_agl=\"800\" />"
"</forecast>"
"</TAF>"
"</data>"
"</response>"
    );
}
