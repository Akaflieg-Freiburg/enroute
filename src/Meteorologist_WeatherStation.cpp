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

#include <cmath>

#include "Meteorologist.h"


Meteorologist::WeatherStation::WeatherStation(QObject *parent) : QObject(parent)
{
}


Meteorologist::WeatherStation::WeatherStation(const QString &id, QObject *parent) : QObject(parent), _ICAOCode(id)
{
}


QGeoCoordinate Meteorologist::WeatherStation::coordinate() const
{
    if (!_metar.isNull())
        return _metar->coordinate();
    if (!_taf.isNull())
        return _taf->coordinate();
    return QGeoCoordinate();
}


void Meteorologist::WeatherStation::setMETAR(Meteorologist::METAR *metar)
{
    // If metar did not change, then do nothing
    if (metar == _metar)
        return;

    // Disconnect old metar
    if (!_metar.isNull())
        disconnect(_metar, &QObject::destroyed, this, &Meteorologist::WeatherStation::metarChanged);

    // Overwrite metar pointer and connect new metar
    _metar = metar;
    if (!_metar.isNull())
        connect(_metar, &QObject::destroyed, this, &Meteorologist::WeatherStation::metarChanged);

    // Let the world know that the metar changed
    emit metarChanged();
}


void Meteorologist::WeatherStation::setTAF(Meteorologist::TAF *taf)
{
    // If metar did not change, then do nothing
    if (taf == _taf)
        return;

    // Disconnect old taf
    if (!_taf.isNull())
        disconnect(_taf, &QObject::destroyed, this, &Meteorologist::WeatherStation::tafChanged);

    // Overwrite metar pointer and connect new taf
    _taf = taf;
    if (!_taf.isNull())
        connect(_taf, &QObject::destroyed, this, &Meteorologist::WeatherStation::tafChanged);

    // Let the world know that the taf changed
    emit tafChanged();
}

// ================================
#warning old functionality, needs to go out

QString Meteorologist::WeatherStation::decodeTime(const QVariant &time) {
    QDateTime tim = QDateTime::fromString(time.toString().replace("T", " "), "yyyy-MM-dd hh:mm:ssZ");
    return tim.toString("ddd MMMM d yyyy hh:mm") + " UTC";
}


QString Meteorologist::WeatherStation::decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg) {
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


QString Meteorologist::WeatherStation::decodeVis(const QVariant &vis) {
    long v = std::lround(vis.toString().toDouble() * 1.61);
    return QString::number(v) + " km";
}


QString Meteorologist::WeatherStation::decodeTemp(const QVariant &temp) {
    QString tmp = temp.toString();
    return tmp.left(tmp.lastIndexOf(".")) + " °C";
}


QString Meteorologist::WeatherStation::decodeQnh(const QVariant &altim) {
    long qnh = std::lround(altim.toString().toDouble() * 33.86);
    return QString::number(qnh) + " hPa";
}


QString Meteorologist::WeatherStation::decodeWx(const QVariant &wx) {
    QString w = wx.toString();
    // clear
    w.replace("NSW", "No significant weather");
    // intensity
    w.replace("-", "light ");
    w.replace("+", "heavy ");
    w.replace("VC", "in the vicinity ");
    w.replace("RE", "recent ");
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
    w.replace("VA", "volcanic ash");
    // other
    w.replace("DS", "duststorm");
    w.replace("FC", "tornado");
    w.replace("TS", "thunderstorm");
    w.replace("SQ", "squalls");
    w.replace("SS", "sandstorm");
    return w;
}


QString Meteorologist::WeatherStation::decodeClouds(const QVariantList &clouds) {
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
