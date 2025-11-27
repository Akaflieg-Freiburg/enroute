/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include "Leg.h"

#include <utility>


namespace {

double deg2rad(double deg)
{
    return deg * M_PI / 180.0;
}

double haversine(double lat1, double lon1, double lat2, double lon2)
{
    const double R = 6371000.0; // Earth radius (meters)
    const double dLat = lat2 - lat1;
    const double dLon = lon2 - lon1;
    const double a = sin(dLat/2)*sin(dLat/2) + cos(lat1)*cos(lat2)*sin(dLon/2)*sin(dLon/2);
    return 2 * R * atan2(sqrt(a), sqrt(1-a));
}

double bearing(double lat1, double lon1, double lat2, double lon2)
{
    const double y = sin(lon2-lon1) * cos(lat2);
    const double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2-lon1);
    return atan2(y, x);
}

Units::Distance distancePointToSegment(const QGeoCoordinate& segmentStart, const QGeoCoordinate& segmentEnd, const QGeoCoordinate& P)
{
    const double R = 6371000.0;

    const double lat1 = deg2rad(segmentStart.latitude());
    const double lon1 = deg2rad(segmentStart.longitude());
    const double lat2 = deg2rad(segmentEnd.latitude());
    const double lon2 = deg2rad(segmentEnd.longitude());
    const double latP = deg2rad(P.latitude());
    const double lonP = deg2rad(P.longitude());

    const double d13 = haversine(lat1, lon1, latP, lonP) / R;
    const double d12 = haversine(lat1, lon1, lat2, lon2) / R;
    const double brg13 = bearing(lat1, lon1, latP, lonP);
    const double brg12 = bearing(lat1, lon1, lat2, lon2);

    const double dxt = asin(sin(d13) * sin(brg13 - brg12)); // radians

    // Check if projection lies outside the segment
    const double dat = acos( cos(d13) / cos(dxt) );  // along-track distance
    if (dat < 0)
        return Units::Distance::fromM(haversine(latP, lonP, lat1, lon1)); // closest to A
    if (dat > d12)
        return Units::Distance::fromM(haversine(latP, lonP, lat2, lon2)); // closest to B

    return Units::Distance::fromM(fabs(dxt) * R); // perpendicular distance
}

} // namespace



//
// Constructors and destructors
//

Navigation::Leg::Leg(GeoMaps::Waypoint start, GeoMaps::Waypoint end) :
    m_start(std::move(start)), m_end(std::move(end))
{
}


//
// Getter Methods
//

auto Navigation::Leg::distance() const -> Units::Distance
{
    // Paranoid safety checks
    if (!isValid()) {
        return {};
    }

    return Units::Distance::fromM( m_start.coordinate().distanceTo( m_end.coordinate() ));
}


auto Navigation::Leg::isValid() const -> bool
{
    if (!m_start.coordinate().isValid()) {
        return false;
    }
    if (!m_end.coordinate().isValid()) {
        return false;
    }
    return true;
}


auto Navigation::Leg::TC() const -> Units::Angle
{
    // Paranoid safety checks
    if (!isValid()) {
        return {};
    }
    if( Units::Distance::fromM(m_start.coordinate().distanceTo(m_end.coordinate())) < minLegLength ) {
        return {};
    }

    return Units::Angle::fromDEG( m_start.coordinate().azimuthTo(m_end.coordinate()) );
}


//
// Methods
//

auto Navigation::Leg::Fuel(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Volume
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle(wind, aircraft)) {
        return {};
    }

    return aircraft.fuelConsumption()*ETE(wind, aircraft);
}


auto Navigation::Leg::GS(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Speed
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle(wind, aircraft)) {
        return {};
    }

    auto TASInKN = aircraft.cruiseSpeed().toKN();
    auto WSInKN  = wind.speed().toKN();
    auto WD      = wind.directionFrom();

    // Law of cosine for wind triangle
    auto GSInKT = qSqrt( TASInKN*TASInKN + WSInKN*WSInKN - 2.0*TASInKN*WSInKN*(WD-TH(wind, aircraft)).cos() );

    return Units::Speed::fromKN(GSInKT);
}


auto Navigation::Leg::isFollowing(const Positioning::PositionInfo& positionInfo) const -> bool
{
    if (!isNear(positionInfo)) {
        return false;
    }

    auto TT = positionInfo.trueTrack();
    if (!TT.isFinite()) {
        return false;
    }

    auto delta = TT -  Units::Angle::fromDEG(positionInfo.coordinate().azimuthTo(m_end.coordinate()));
    auto deltaDeg = delta.toDEG();
    if ((deltaDeg < 300.0) && (deltaDeg > 60.0)) {
        return false;
    }

    delta = TT -  Units::Angle::fromDEG(positionInfo.coordinate().azimuthTo(m_start.coordinate()));
    deltaDeg = delta.toDEG();
    return (deltaDeg >= 120.0) && (deltaDeg <= 240.0);
}


bool Navigation::Leg::isNear(const Positioning::PositionInfo& positionInfo) const
{
    if (!isValid() || !positionInfo.isValid() || !m_start.isValid() || !m_end.isValid())
    {
        return false;
    }

    auto dp2s = distancePointToSegment(m_start.coordinate(), m_end.coordinate(), positionInfo.coordinate());
    if (!dp2s.isFinite())
    {
        return false;
    }

    return dp2s <= nearThreshold;
}


auto Navigation::Leg::WCA(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Angle
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle(wind, aircraft)) {
        return {};
    }

    Units::Speed const TAS = aircraft.cruiseSpeed();
    Units::Speed const WS = wind.speed();
    Units::Angle const WD = wind.directionFrom();

    // Law of sine for wind triangle
    return Units::Angle::asin(-(TC()-WD).sin() *(WS/TAS));
}


auto Navigation::Leg::description(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> QString
{
    if (!isValid()) {
        return {};
    }

    QString result;
    result += QStringLiteral("%1").arg( aircraft.horizontalDistanceToString(distance()) );
    auto _time = ETE(wind, aircraft);
    if (_time.isFinite()) {
        result += QStringLiteral(" • ETE %1 h").arg(_time.toHoursAndMinutes());
    }
    auto TCInDEG = TC().toDEG();
    if (qIsFinite(TCInDEG)) {
        result += QStringLiteral(" • TC %1°").arg(qRound(TCInDEG));
    }
    double const THInDEG = TH(wind, aircraft).toDEG();
    if (qIsFinite(THInDEG)) {
        result += QStringLiteral(" • TH %1°").arg(qRound(THInDEG));
    }

    return result;
}


auto Navigation::Leg::hasDataForWindTriangle(Weather::Wind wind, const Navigation::Aircraft& aircraft) -> bool
{

    if ( !aircraft.cruiseSpeed().isFinite() ) {
        return false;
    }

    if ( !wind.speed().isFinite() ) {
        return false;
    }
    if ( !wind.directionFrom().isFinite() ) {
        return false;
    }
    if (wind.speed() > 0.75*aircraft.cruiseSpeed()) {
        return false;
    }

    return true;
}
