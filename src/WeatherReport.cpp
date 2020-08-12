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

WeatherReport::WeatherReport(const QString &id,
                             const QMultiMap<QString, QVariant> &metar,
                             const QMultiMap<QString, QVariant> &taf,
                             QObject *parent) : _id(id) {

    // Get flight category
    _cat = metar.contains("flight_category") ? metar.value("flight_category").toString() : "UKN";
    
    // Generate METAR
    if (metar.empty())
        _metar.push_back("NONE");
    else {
        if (metar.contains("raw_text"))
            _metar.push_back("RAW " + metar.value("raw_text").toString());
        if (metar.contains("observation_time"))
            _metar.push_back("TIME" + this->decodeTime(metar.value("observation_time")));
        if (metar.contains("wind_dir_degrees") && metar.contains("wind_speed_kt")) {
            if (metar.contains("wind_gust_kt"))
                _metar.push_back("WIND" + this->decodeWind(metar.value("wind_dir_degrees"), metar.value("wind_speed_kt"), metar.value("wind_gust_kt")));
            else
                _metar.push_back("WIND" + this->decodeWind(metar.value("wind_dir_degrees"), metar.value("wind_speed_kt")));
        }
        if (metar.contains("visibility_statute_mi"))
            _metar.push_back("VIS " + this->decodeVis(metar.value("visibility_statute_mi")));
        if (metar.contains("wx_string"))
            _metar.push_back("WX  " + this->decodeWx(metar.value("wx_string")));
        if (metar.contains("sky_condition"))
            _metar.push_back("CLDS" + this->decodeClouds(metar.values("sky_condition")));
        if (metar.contains("temp_c"))
            _metar.push_back("TEMP" + this->decodeTemp(metar.value("temp_c")));
        if (metar.contains("dewpoint_c"))
            _metar.push_back("DEWP" + this->decodeTemp(metar.value("dewpoint_c")));
        if (metar.contains("altim_in_hg"))
            _metar.push_back("QNH " + this->decodeQnh(metar.value("altim_in_hg")));
    }

    // Generate TAF
    if (taf.empty())
        _taf.push_back("NONE");
    else {
        if (taf.contains("raw_text"))
            _taf.push_back("RAW " + taf.value("raw_text").toString());
        if (taf.contains("issue_time"))
            _taf.push_back("TIME" + this->decodeTime(taf.value("issue_time")));
        if (taf.contains("valid_time_from"))
            _taf.push_back("FROM" + this->decodeTime(taf.value("valid_time_from")));
        if (taf.contains("valid_time_to"))
            _taf.push_back("TO  " + this->decodeTime(taf.value("valid_time_to")));
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
                    fcst.push_back(forecast.value("probability").toString() + "% probability<br>");
                if (forecast.contains("wind_dir_degrees") && forecast.contains("wind_speed_kt"))
                    fcst.push_back("Wind " + this->decodeWind(forecast.value("wind_dir_degrees"), forecast.value("wind_speed_kt")) + "<br>");
                if (forecast.contains("visibility_statute_mi"))
                    fcst.push_back("Visibility " + this->decodeVis(forecast.value("visibility_statute_mi")) + "<br>");
                if (forecast.contains("wx_string"))
                    fcst.push_back(this->decodeWx(forecast.value("wx_string")) + "<br>");
                if (forecast.contains("sky_condition"))
                    fcst.push_back(this->decodeClouds(forecast.values("sky_condition")));
                _taf.push_back("FCST" + fcst);
            }
        }
    }
}

QString WeatherReport::decodeTime(const QVariant &time) {
    QDateTime tim = QDateTime::fromString(time.toString().replace("T", " "), "yyyy-MM-dd hh:mm:ssZ");
    return tim.toString("ddd MMMM d yyyy hh:mm") + " UTC";
}

QString WeatherReport::decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg) {
    QString w;
    if (windd.toString() == "0")
        if (winds.toString() == "0")
            return "calm";
        else
            w += "variable";
    else
        w += "from " + windd.toString() + "°";
    w += " at " + winds.toString() + " kt";
    if (windg.toString() != "0")
        w+= ", gusty " + windg.toString() + " kt";
    return w;
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
    // clear
    w.replace("NSW", "No significant weather");
    // intensity
    w.replace("-", "light ");
    w.replace("+", "heavy ");
    // qualifier
    w.replace("BC", "patches of");
    w.replace("BL", "blowing");
    w.replace("FZ", "freezing");
    w.replace("MI", "shallow");
    w.replace("PR", "partial");
    w.replace("RE", "recent");
    w.replace("SH", "showers of");
    // precipitation
    w.replace("DZ", "drizzle");
    w.replace("IC", "ice crystal");
    w.replace("GR", "hail");
    w.replace("GS", "snow pellets");
    w.replace("PL", "ice pellets");
    w.replace("RA", "rain");
    w.replace("SN", "snow");
    w.replace("SG", "snow grains");
    // obscuration
    w.replace("BR", "mist");
    w.replace("DU", "dust");
    w.replace("FG", "fog");
    w.replace("FU", "smoke");
    w.replace("HZ", "haze");
    w.replace("PY", "spray");
    w.replace("SA", "sand");
    w.replace("VA", "volcanic ashe");
    // other
    w.replace("DS", "duststorm");
    w.replace("FC", "tornado");
    w.replace("TS", "thunderstorm");
    w.replace("SQ", "squalls");
    w.replace("SS", "sandstorm");
    return w;
}

QString WeatherReport::decodeClouds(const QVariantList &clouds) {
    QString clds;
    for (int i = clouds.size() - 1; i >= 0; --i) {
        QList<QString> layer = clouds[i].toString().split(",");
        clds += layer[0];
        if (layer.size() >= 2) {
            if (layer.size() == 3)
                clds += " " + layer[2];
            else
                clds += " clouds";
            clds += " at " + layer[1] + " ft AGL";
        }
        if (i > 0)
            clds += "<br>";
    }
    clds.replace("NSC", "No significant clouds");
    clds.replace("SKC", "Sky clear");
    clds.replace("CLR", "Clear");
    clds.replace("CAVOK", "Ceiling and visibility OK");
    clds.replace("FEW", "Few");
    clds.replace("SCT", "Scattered");
    clds.replace("BKN", "Broken");
    clds.replace("OVC", "Overcast");
    clds.replace("OVX", "Obscured");
    clds.replace("OVCX", "Obscured");
    clds.replace("CB", "Cumulonimbus");
    clds.replace("TCU", "Towering cumulus");
    clds.replace("CU", "Cumulus");
    return clds;
}
