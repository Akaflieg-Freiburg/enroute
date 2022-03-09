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

#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "Settings.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"


//
// Constructors and destructors
//

Navigation::Navigator::Navigator(QObject *parent) : GlobalObject(parent)
{
    m_aircraftFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aircraft.json";

    // Restore wind
    QSettings settings;
    m_wind.setSpeed(Units::Speed::fromKN(settings.value("Wind/windSpeedInKT", qQNaN()).toDouble()));
    m_wind.setDirectionFrom( Units::Angle::fromDEG(settings.value("Wind/windDirectionInDEG", qQNaN()).toDouble()) );

    // Restore aircraft
    QFile file(m_aircraftFileName);
    if (file.open(QIODevice::ReadOnly)) {
        m_aircraft.loadFromJSON(file.readAll());
    } else {
        auto cruiseSpeed = Units::Speed::fromKN(settings.value("Aircraft/cruiseSpeedInKTS", 0.0).toDouble());
        auto descentSpeed = Units::Speed::fromKN(settings.value("Aircraft/descentSpeedInKTS", 0.0).toDouble());
        auto fuelConsumption = Units::VolumeFlow::fromLPH(settings.value("Aircraft/fuelConsumptionInLPH", 0.0).toDouble());
        m_aircraft.setCruiseSpeed(cruiseSpeed);
        m_aircraft.setDescentSpeed(descentSpeed);
        m_aircraft.setFuelConsumption(fuelConsumption);
    }
}


void Navigation::Navigator::deferredInitialization()
{
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateAltitudeLimit);
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateFlightStatus);
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateNextWaypoint);
}


//
// Getter Methods
//

auto Navigation::Navigator::clock() -> Navigation::Clock*
{
    if (m_clock.isNull()) {
        m_clock = new Navigation::Clock(this);
        QQmlEngine::setObjectOwnership(m_clock, QQmlEngine::CppOwnership);
    }
    return m_clock;
}


auto Navigation::Navigator::flightRoute() -> FlightRoute*
{
    if (m_flightRoute.isNull()) {
        m_flightRoute = new FlightRoute(this);
        QQmlEngine::setObjectOwnership(m_flightRoute, QQmlEngine::CppOwnership);
    }
    return m_flightRoute;
}


//
// Setter Methods
//

void Navigation::Navigator::setAircraft(const Navigation::Aircraft& newAircraft)
{
    if (newAircraft == m_aircraft) {
        return;
    }

    // Save aircraft
    QFile file(m_aircraftFileName);
    file.open(QIODevice::WriteOnly);
    file.write(newAircraft.toJSON());

    // Set new wind
    m_aircraft = newAircraft;
    emit aircraftChanged();
}


void Navigation::Navigator::setFlightStatus(FlightStatus newFlightStatus)
{
    if (m_flightStatus == newFlightStatus) {
        return;
    }

    m_flightStatus = newFlightStatus;
    emit flightStatusChanged();
}


void Navigation::Navigator::setWind(const Weather::Wind& newWind)
{
    if (newWind == m_wind) {
        return;
    }

    // Save wind
    QSettings settings;
    settings.setValue("Wind/windSpeedInKT", newWind.speed().toKN());
    settings.setValue("Wind/windDirectionInDEG", newWind.directionFrom().toDEG());

    // Set new wind
    m_wind = newWind;
    emit windChanged();

}


//
// Slots
//

void Navigation::Navigator::updateAltitudeLimit(const Positioning::PositionInfo& info)
{  
    if (!info.isValid()) {
        return;
    }

    auto altLimit = GlobalObject::settings()->airspaceAltitudeLimit();
    auto trueAltitude = info.trueAltitude();
    if (altLimit.isFinite() &&
            trueAltitude.isFinite() &&
            (trueAltitude + Units::Distance::fromFT(1000) > altLimit)) {

        // Round trueAltitude+1000ft up to nearest 500ft and set that as a new limit
        auto newAltLimit = Units::Distance::fromFT(500.0*qCeil(trueAltitude.toFeet()/500.0+2.0));
        GlobalObject::settings()->setAirspaceAltitudeLimit(newAltLimit);
        emit airspaceAltitudeLimitAdjusted();
    }
}


void Navigation::Navigator::updateFlightStatus(const Positioning::PositionInfo& info)
{
    if (!info.isValid()) {
        setFlightStatus(Unknown);
        return;
    }

    // Get ground speed and aircraft minimum speed
    auto GS = info.groundSpeed();
    auto aircraftMinSpeed = m_aircraft.minimumSpeed();
    if (!GS.isFinite() || !aircraftMinSpeed.isFinite()) {
        setFlightStatus(Unknown);
        return;
    }

    // Go to ground mode if ground speed is less then aircraftMinSpeed-flightSpeedHysteresis
    if (m_flightStatus == Flight) {
        // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
        if ( GS < aircraftMinSpeed-flightSpeedHysteresis) {
            setFlightStatus(Ground);
        }
        return;
    }

    // Go to flight mode if ground speed is more than aircraftMinSpeed
    if ( GS > aircraftMinSpeed ) {
        setFlightStatus(Flight);
    }
}


void Navigation::Navigator::updateNextWaypoint(const Positioning::PositionInfo& info)
{
    qWarning() << "updateNextWaypoint";

    auto legs = m_flightRoute->legs();
    if (legs.isEmpty()) {
        return;
    }

    for(int i=legs.size()-1; i>=0; i--) {
        if (legs[i].isFollowing(info)) {
            qWarning() << "Following leg" << legs[i].startPoint().ICAOCode() << legs[i].endPoint().ICAOCode();
            return;
        }
    }

    for(int i=legs.size()-1; i>=0; i--) {
        if (legs[i].isNear(info)) {
            qWarning() << "Near leg" << legs[i].startPoint().ICAOCode() << legs[i].endPoint().ICAOCode();
            return;
        }
    }
}
