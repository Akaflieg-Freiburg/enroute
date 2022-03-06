/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
#include "GlobalObject.h"
#include "navigation/Navigator.h"
#include "Settings.h"


Navigation::Leg::Leg(const GeoMaps::Waypoint& start, const GeoMaps::Waypoint& end) :
    m_start(start), m_end(end)
{
}


auto Navigation::Leg::distance() const -> Units::Distance
{
    // Paranoid safety checks
    if (!valid()) {
        return {};
    }

    return Units::Distance::fromM( m_start.coordinate().distanceTo( m_end.coordinate() ));
}


auto Navigation::Leg::Fuel(const Weather::Wind& wind, const Navigation::Aircraft& aircraft) const -> Units::Volume
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle(wind, aircraft)) {
        return {};
    }

    return aircraft.fuelConsumption()*ETE(wind, aircraft);
}


auto Navigation::Leg::GS(const Weather::Wind& wind, const Navigation::Aircraft& aircraft) const -> Units::Speed
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


auto Navigation::Leg::TC() const -> Units::Angle
{
    // Paranoid safety checks
    if (!valid()) {
        return {};
    }
    if( Units::Distance::fromM(m_start.coordinate().distanceTo(m_end.coordinate())) < minLegLength ) {
        return {};
    }

    return Units::Angle::fromDEG( m_start.coordinate().azimuthTo(m_end.coordinate()) );
}


auto Navigation::Leg::WCA(const Weather::Wind& wind, const Navigation::Aircraft& aircraft) const -> Units::Angle
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle(wind, aircraft)) {
        return {};
    }

    Units::Speed TAS = aircraft.cruiseSpeed();
    Units::Speed WS  = wind.speed();
    Units::Angle WD  = wind.directionFrom();

    // Law of sine for wind triangle
    return Units::Angle::asin(-(TC()-WD).sin() *(WS/TAS));
}


auto Navigation::Leg::valid() const -> bool
{
    if (!m_start.coordinate().isValid()) {
        return false;
    }
    if (!m_end.coordinate().isValid()) {
        return false;
    }
    return true;
}


auto Navigation::Leg::description(const Weather::Wind& wind, const Navigation::Aircraft& aircraft) const -> QString
{
    if (!valid()) {
        return QString();
    }

    QString result;
    result += QString("%1").arg( aircraft.horizontalDistanceToString(distance()) );
    auto _time = ETE(wind, aircraft);
    if (_time.isFinite()) {
        result += QString(" • ETE %1 h").arg(_time.toHoursAndMinutes());
    }
    auto TCInDEG = TC().toDEG();
    if (qIsFinite(TCInDEG)) {
        result += QString(" • TC %1°").arg(qRound(TCInDEG));
    }
    double THInDEG = TH(wind, aircraft).toDEG();
    if (qIsFinite(THInDEG)) {
        result += QString(" • TH %1°").arg(qRound(THInDEG));
    }

    return result;
}


auto Navigation::Leg::hasDataForWindTriangle(const Weather::Wind& wind, const Navigation::Aircraft& aircraft) const -> bool
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
