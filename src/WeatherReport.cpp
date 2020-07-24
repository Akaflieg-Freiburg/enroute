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

#include "WeatherReport.h"

#include <QQmlEngine>
#include <QDateTime>
#include <cmath>

WeatherReport::WeatherReport(const QMultiMap<QString, QVariant> &metar, const QMultiMap<QString, QVariant> &taf, QObject *parent) {

    // Get station ID and flight category
    _id = metar.contains("station_id") ? metar.value("station_id").toString() : "UKN";
    _cat = metar.contains("flight_category") ? metar.value("flight_category").toString() : "UKN";
    
    // Generate METAR
    /*todo handle units through AviationUnits */
    /*todo handle exceptions */
    if (metar.contains("raw_text"))
        _metar.push_back("RAW   " + metar.value("raw_text").toString());
    if (metar.contains("observation_time"))
        _metar.push_back("TIME  " + this->decodeTime(metar.value("observation_time")));
    if (metar.contains("wind_dir_degrees") && metar.contains("wind_speed_kt"))
        _metar.push_back("WIND  From " + this->decodeWind(metar.value("wind_dir_degrees"), metar.value("wind_speed_kt")));
    if (metar.contains("visibility_statute_mi"))
        _metar.push_back("VIS   " + this->decodeVis(metar.value("visibility_statute_mi")));
    if (metar.contains("wx_string"))
    _metar.push_back("WX    " + this->decodeWx(metar.value("wx_string")));
    if (metar.contains("sky_condition"))
        _metar.push_back("CLDS  " + this->decodeClouds(metar.values("sky_condition")));
    if (metar.contains("temp_c"))
        _metar.push_back("TEMP  " + this->decodeTemp(metar.value("temp_c")));
    if (metar.contains("dewpoint_c"))
        _metar.push_back("DEWP  " + this->decodeTemp(metar.value("dewpoint_c")));
    if (metar.contains("altim_in_hg"))
        _metar.push_back("QNH   " + this->decodeQnh(metar.value("altim_in_hg")));

    // Generate TAF
    /*todo handle units through AviationUnits */
    /*todo handle exceptions */
    if (taf.contains("raw_text"))
        _taf.push_back("RAW   " + taf.value("raw_text").toString());
    if (taf.contains("issue_time"))
        _taf.push_back("TIME  " + this->decodeTime(taf.value("issue_time")));
    if (taf.contains("valid_time_from"))
        _taf.push_back("FROM  " + this->decodeTime(taf.value("valid_time_from")));
    if (taf.contains("valid_time_to"))
        _taf.push_back("TO    " + this->decodeTime(taf.value("valid_time_to")));
    if (taf.contains("forecast")) {
        for (int i = taf.values("forecast").size() - 1; i >= 0; --i) {
            QMultiMap<QString, QVariant> forecast = taf.values("forecast")[i].toMap();
            QString fcst;
            if (forecast.contains("fcst_time_from") && forecast.contains("fcst_time_to")) {
                if (!forecast.contains("change_indicator"))
                    fcst += "From " + this->decodeTime(forecast.value("fcst_time_from")) + " to " + this->decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else if (forecast.value("change_indicator").toString() == "TEMPO")
                    fcst += "Temporary from " + this->decodeTime(forecast.value("fcst_time_from")) + " to " + this->decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else if (forecast.value("change_indicator").toString() == "BECMG")
                    fcst += "Transition from " + this->decodeTime(forecast.value("fcst_time_from")) + " to " + this->decodeTime(forecast.value("fcst_time_to")) + "<br>";
                else
                    fcst += "From " + this->decodeTime(forecast.value("fcst_time_from")) + " to " + this->decodeTime(forecast.value("fcst_time_to")) + "<br>";
                
            }
            if (forecast.contains("probability"))
                fcst.push_back(forecast.value("probability").toString().remove("PROB") + "% probability<br>");
            if (forecast.contains("wind_dir_degrees") && forecast.contains("wind_speed_kt"))
                fcst.push_back("Wind from " + this->decodeWind(forecast.value("wind_dir_degrees"), forecast.value("wind_speed_kt")) + "<br>");
            if (forecast.contains("visibility_statute_mi"))
                fcst.push_back("Visibility " + this->decodeVis(forecast.value("visibility_statute_mi")) + "<br>");
            if (forecast.contains("wx_string"))
                fcst.push_back("Weather " + this->decodeWx(forecast.value("wx_string")) + "<br>");
            if (forecast.contains("sky_condition"))
                fcst.push_back("Clouds " + this->decodeClouds(forecast.values("sky_condition")) + "<br>");
            _taf.push_back("FCST  " + fcst);
        }
    }
}

QString WeatherReport::decodeTime(const QVariant &time) {
    QDateTime tim = QDateTime::fromString(time.toString().replace("T", " "), "yyyy-MM-dd hh:mm:ssZ");
    return tim.toString("ddd MMMM d yyyy hh:mm") + " UTC";
}

QString WeatherReport::decodeWind(const QVariant &windd, const QVariant &winds) {
    return windd.toString() + "° at " + winds.toString() + " kt";
}

QString WeatherReport::decodeVis(const QVariant &vis) {
    long v = std::lround(vis.toString().toDouble() * 1.61);
    return QString::number(v) + " km";
}

QString WeatherReport::decodeTemp(const QVariant &temp) {
    QString tmp = temp.toString();
    return tmp.left(tmp.lastIndexOf(".")) + " °C";
}

QString WeatherReport::decodeQnh(const QVariant &altim) {
    long qnh = std::lround(altim.toString().toDouble() * 33.86);
    return QString::number(qnh) + " hPa";
}

QString WeatherReport::decodeWx(const QVariant &wx) {
    QString w = wx.toString();
    w.replace("-", "light ");
    w.replace("+", "Heavy ");
    w.replace("SHRA", "showers of rain");
    w.replace("SHSN", "showers of snow");
    w.replace("RA", "rain");
    w.replace("RADZ", "rain-drizzle");
    w.replace("DZ", "drizzle");
    w.replace("TS", "thunderstorm");
    return w;
}

QString WeatherReport::decodeClouds(const QVariantList &clouds) {
    QString clds;
    for (int i = clouds.size() - 1; i >= 0; --i) {
        QString layer = clouds[i].toString();
        clds += layer.left(layer.lastIndexOf(",")) + " at " + layer.right(layer.lastIndexOf(",") + 1) + " ft";
        if (i > 0)
            clds += "<br>";
    }
    clds.replace("FEW", "Few");
    clds.replace("SCT", "Scattered");
    clds.replace("BKN", "Broken");
    clds.replace("OVC", "Overcast");
    return clds;
}
