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

#include "GeoMapProvider.h"
#include "Meteorologist.h"


Meteorologist::WeatherStation::WeatherStation(QObject *parent)
    : QObject(parent)
{
}


Meteorologist::WeatherStation::WeatherStation(const QString &id, GeoMapProvider *geoMapProvider, QObject *parent)
    : QObject(parent),
      _ICAOCode(id),
      _geoMapProvider(geoMapProvider)
{
    _extendedName = _ICAOCode;
    _twoLineTitle = _ICAOCode;

    // Wire up with GeoMapProvider, in order to learn about future changes in waypoints
    connect(_geoMapProvider, &GeoMapProvider::geoJSONChanged, this, &Meteorologist::WeatherStation::readDataFromWaypoint);
    readDataFromWaypoint();
}


void Meteorologist::WeatherStation::readDataFromWaypoint()
{
    // Paranoid safety checks
    if (_geoMapProvider.isNull())
        return;
    // Immediately quit if we already have the necessary data
    if (hasWaypointData)
        return;

    auto waypoint = _geoMapProvider->findByID(_ICAOCode);
    if (waypoint == nullptr)
        return;
    hasWaypointData = true;

    // Update data
    auto cacheCoordiante = _coordinate;
    _coordinate = waypoint->coordinate();
    if (_coordinate != cacheCoordiante)
        emit coordinateChanged();

    auto cacheExtendedName = _extendedName;
    _extendedName = waypoint->extendedName();
    if (_extendedName != cacheExtendedName)
        emit extendedNameChanged();

    auto cacheIcon = _icon;
    _icon = waypoint->icon();
    if (_icon != cacheIcon)
        emit iconChanged();

    auto cacheTwoLineTitle = _twoLineTitle;
    _twoLineTitle = waypoint->twoLineTitle();
    if (_twoLineTitle != cacheTwoLineTitle)
        emit twoLineTitleChanged();

    disconnect(_geoMapProvider, nullptr, this, nullptr);
}


void Meteorologist::WeatherStation::setMETAR(Weather::METAR *metar)
{
    // Ignore invalid and expired METARs. Also ignore METARs whose ICAO code does not match with this weather station
    if (metar)
        if (!metar->isValid() || metar->isExpired() || (metar->ICAOCode() != _ICAOCode)) {
            metar->deleteLater();
            return;
        }

    // If METAR did not change, then do nothing
    if (metar == _metar)
        return;

    // Cache values
    auto cacheHasMETAR = hasMETAR();

    // Take ownership. This will guarantee that the METAR gets deleted along with this weather station.
    if (metar)
        metar->setParent(this);

    // Clear and delete old METAR, if one exists.
    if (!_metar.isNull()) {
        disconnect(_metar, nullptr, this, nullptr);
        _metar->deleteLater();
    }

    // Overwrite metar pointer
    _metar = metar;

    if (_metar) {
        // Connect new METAR, update the coordinate if necessary.
        connect(_metar, &QObject::destroyed, this, &Meteorologist::WeatherStation::metarChanged);

        // Update coordinate
        if (!_coordinate.isValid())
            _coordinate = _metar->coordinate();
    }

    // Let the world know that the metar changed
    if (cacheHasMETAR != hasMETAR())
        emit hasMETARChanged();
    emit metarChanged();
}


void Meteorologist::WeatherStation::setTAF(Weather::TAF *taf)
{
    // Ignore invalid and expired TAFs. Also ignore TAFs whose ICAO code does not match with this weather station
    if (taf)
        if (!taf->isValid() || taf->isExpired() || (taf->ICAOCode() != _ICAOCode)) {
            taf->deleteLater();
            return;
        }

    // If TAF did not change, then do nothing
    if (taf == _taf)
        return;

    // Cache values
    auto cacheHasTAF = hasTAF();

    // Take ownership. This will guarantee that the METAR gets deleted along with this weather station.
    if (taf)
        taf->setParent(this);

    // Clear and delete old TAF, if one exists.
    if (!_taf.isNull()) {
        disconnect(_taf, nullptr, this, nullptr);
        _taf->deleteLater();
    }

    // Overwrite TAF pointer
    _taf = taf;

    if (_taf) {
        // Connect new TAF, update the coordinate if necessary.
        connect(_taf, &QObject::destroyed, this, &Meteorologist::WeatherStation::tafChanged);

        // Update coordinate
        if (!_coordinate.isValid())
            _coordinate = _taf->coordinate();
    }

    // Let the world know that the taf changed
    if (cacheHasTAF != hasTAF())
        emit hasTAFChanged();
    emit tafChanged();
}


QString Meteorologist::WeatherStation::wayTo(QGeoCoordinate fromCoordinate, bool useMetricUnits) const
{
    // Paranoid safety checks
    if (!fromCoordinate.isValid())
        return QString();
    auto _coordinate = coordinate();
    if (!_coordinate.isValid())
        return QString();

    auto dist = AviationUnits::Distance::fromM(fromCoordinate.distanceTo(_coordinate));
    auto QUJ = qRound(fromCoordinate.azimuthTo(_coordinate));

    if (useMetricUnits)
        return QString("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    return QString("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
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
