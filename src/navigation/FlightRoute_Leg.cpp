/***************************************************************************
 *   Copyright (C) 2019,2021 by Stefan Kebekus                             *
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

#include <QtGlobal>

#include "FlightRoute_Leg.h"
#include "GlobalObject.h"
#include "navigation/Navigator.h"
#include "Settings.h"


Navigation::FlightRoute::Leg::Leg(const GeoMaps::Waypoint& start, const GeoMaps::Waypoint& end, Aircraft *aircraft, Weather::Wind *wind, QObject* parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    _start = start;
    _end   = end;

    connect(_aircraft, &Aircraft::cruiseSpeedChanged, this, &FlightRoute::Leg::valChanged);
    connect(_aircraft, &Aircraft::fuelConsumptionChanged, this, &FlightRoute::Leg::valChanged);
    connect(_wind, &Weather::Wind::valChanged, this, &FlightRoute::Leg::valChanged);
}


auto Navigation::FlightRoute::Leg::distance() const -> Units::Distance
{
    // Paranoid safety checks
    if (!isValid()) {
        return {};
    }

    return Units::Distance::fromM( _start.coordinate().distanceTo( _end.coordinate() ));
}


auto Navigation::FlightRoute::Leg::Fuel() const -> Units::Volume
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle()) {
        return {};
    }

    return _aircraft->fuelConsumption()*Time();
}


auto Navigation::FlightRoute::Leg::GS() const -> Units::Speed
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle()) {
        return {};
    }

    auto TASInKN = _aircraft->cruiseSpeed().toKN();
    auto WSInKN  = _wind->speed().toKN();
    auto WD      = _wind->directionFrom();

    // Law of cosine for wind triangle
    auto GSInKT = qSqrt( TASInKN*TASInKN + WSInKN*WSInKN - 2.0*TASInKN*WSInKN*(WD-TH()).cos() );

    return Units::Speed::fromKN(GSInKT);
}


auto Navigation::FlightRoute::Leg::TC() const -> Units::Angle
{
    // Paranoid safety checks
    if (!isValid()) {
        return {};
    }
    if( _start.coordinate().distanceTo( _end.coordinate() ) < minLegLength ) {
        return {};
    }

    return Units::Angle::fromDEG( _start.coordinate().azimuthTo(_end.coordinate()) );
}


auto Navigation::FlightRoute::Leg::WCA() const -> Units::Angle
{
    // This also checks for _aircraft and _wind to be non-nullptr
    if (!hasDataForWindTriangle()) {
        return {};
    }

    Units::Speed TAS = _aircraft->cruiseSpeed();
    Units::Speed WS  = _wind->speed();
    Units::Angle WD  = _wind->directionFrom();

    // Law of sine for wind triangle
    return Units::Angle::asin(-(TC()-WD).sin() *(WS/TAS));
}


auto Navigation::FlightRoute::Leg::isValid() const -> bool
{
    if (!_start.coordinate().isValid()) {
        return false;
    }
    if (!_end.coordinate().isValid()) {
        return false;
    }
    return true;
}


auto Navigation::FlightRoute::Leg::description() const -> QString
{
    if (!isValid()) {
        return QString();
    }

    QString result;
    result += QString("%1").arg( GlobalObject::navigator()->aircraft()->horizontalDistanceToString(distance()) );
    auto _time = Time();
    if (_time.isFinite()) {
        result += QString(" • %1 h").arg(_time.toHoursAndMinutes());
    }
    auto TCInDEG = TC().toDEG();
    if (qIsFinite(TCInDEG)) {
        result += QString(" • TC %1°").arg(qRound(TCInDEG));
    }
    double THInDEG = TH().toDEG();
    if (qIsFinite(THInDEG)) {
        result += QString(" • TH %1°").arg(qRound(THInDEG));
    }

    return result;
}


auto Navigation::FlightRoute::Leg::hasDataForWindTriangle() const -> bool
{
    if ( _aircraft.isNull() ) {
        return false;
    }
    if ( !_aircraft->cruiseSpeed().isFinite() ) {
        return false;
    }

    if (_wind.isNull()) {
        return false;
    }
    if ( !_wind->speed().isFinite() ) {
        return false;
    }
    if ( !_wind->directionFrom().isFinite() ) {
        return false;
    }
    if (_wind->speed().toKN() > 0.75*_aircraft->cruiseSpeed().toKN() ) {
        return false;
    }

    return true;
}
