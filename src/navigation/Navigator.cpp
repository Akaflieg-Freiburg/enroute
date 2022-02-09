/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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


Navigation::Navigator::Navigator(QObject *parent) : QObject(parent)
{
    m_aircraftFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aircraft.json";

    QTimer::singleShot(0, this, &Navigation::Navigator::deferredInitialization);
}


Navigation::Navigator::~Navigator()
{
    saveAircraft();
}


auto Navigation::Navigator::aircraft() -> Navigation::Aircraft*
{
    if (m_aircraft.isNull()) {
        m_aircraft = new Navigation::Aircraft(this);
        QQmlEngine::setObjectOwnership(m_aircraft, QQmlEngine::CppOwnership);

        QFile file(m_aircraftFileName);
        if (file.open(QIODevice::ReadOnly)) {
            m_aircraft->loadFromJSON(file.readAll());
        } else {
            QSettings settings;
            auto cruiseSpeed = Units::Speed::fromKN(settings.value("Aircraft/cruiseSpeedInKTS", 0.0).toDouble());
            auto descentSpeed = Units::Speed::fromKN(settings.value("Aircraft/descentSpeedInKTS", 0.0).toDouble());
            auto fuelConsumption = Units::VolumeFlow::fromLPH(settings.value("Aircraft/fuelConsumptionInLPH", 0.0).toDouble());
            m_aircraft->setCruiseSpeed(cruiseSpeed);
            m_aircraft->setDescentSpeed(descentSpeed);
            m_aircraft->setFuelConsumption(fuelConsumption);
        }

        connect(m_aircraft, &Navigation::Aircraft::cruiseSpeedChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::descentSpeedChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::fuelConsumptionChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::fuelConsumptionUnitChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::horizontalDistanceUnitChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::minimumSpeedChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::nameChanged, this, &Navigation::Navigator::saveAircraft);
        connect(m_aircraft, &Navigation::Aircraft::verticalDistanceUnitChanged, this, &Navigation::Navigator::saveAircraft);
    }
    return m_aircraft;
}


auto Navigation::Navigator::clock() -> Navigation::Clock*
{
    if (m_clock.isNull()) {
        m_clock = new Navigation::Clock(this);
    }
    return m_clock;
}


auto Navigation::Navigator::describeWay(const QGeoCoordinate &from, const QGeoCoordinate &to) -> QString
{
    // Paranoid safety checks
    if (!from.isValid()) {
        return {};
    }
    if (!to.isValid()) {
        return {};
    }

    auto dist = Units::Distance::fromM(from.distanceTo(to));
    auto QUJ = qRound(from.azimuthTo(to));

    switch(GlobalObject::navigator()->aircraft()->horizontalDistanceUnit()) {
    case Navigation::Aircraft::Kilometer:
        return QStringLiteral("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    case Navigation::Aircraft::StatuteMile:
        return QStringLiteral("DIST %1 mil • QUJ %2°").arg(dist.toMIL(), 0, 'f', 1).arg(QUJ);
    case Navigation::Aircraft::NauticalMile:
        return QStringLiteral("DIST %1 nm • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
    }

    // This should never be reached.
    return {};
}


void Navigation::Navigator::deferredInitialization() const
{
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::onPositionUpdated);
}


auto Navigation::Navigator::flightRoute() -> FlightRoute*
{
    if (m_flightRoute.isNull()) {
        m_flightRoute = new FlightRoute(this);
    }
    return m_flightRoute;
}


void Navigation::Navigator::onPositionUpdated(const Positioning::PositionInfo& info)
{  
    // Paranoid safety checks
    if (m_aircraft.isNull()) {
        setFlightStatus(Unknown);
        return;
    }

    if (!info.isValid()) {
        setFlightStatus(Unknown);
        return;
    }

    // Get ground speed
    auto GS = info.groundSpeed();
    if (!GS.isFinite()) {
        setFlightStatus(Unknown);
        return;
    }

    // Get aircraft minimum speed
    auto aircraftMinSpeed = m_aircraft->minimumSpeed();
    if (!aircraftMinSpeed.isFinite()) {
        setFlightStatus(Unknown);
        return;
    }

    // Go to ground mode if ground speed is less then aircraftMinSpeed-flightSpeedHysteresis
    if (m_isInFlight) {
        // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
        if ( GS < aircraftMinSpeed-flightSpeedHysteresis) {
            setIsInFlight(Ground);
        }
        return;
    }

    // Go to flight mode if ground speed is more than aircraftMinSpeed
    if ( GS > aircraftMinSpeed ) {
        setIsInFlight(Flight);
    }
}


void Navigation::Navigator::saveAircraft() const
{
    QFile file(m_aircraftFileName);
    file.open(QIODevice::WriteOnly);
    file.write(m_aircraft->toJSON());
}


void Navigation::Navigator::setIsInFlight(bool newIsInFlight)
{
    if (m_isInFlight == newIsInFlight) {
        return;
    }
    m_isInFlight = newIsInFlight;
    emit isInFlightChanged();
}


void Navigation::Navigator::setFlightStatus(FlightStatus newFlightStatus)
{
    if (m_flightStatus == newFlightStatus) {
        return;
    }
    m_flightStatus = newFlightStatus;
    emit flightStatusChanged();
}


auto Navigation::Navigator::wind() -> Weather::Wind*
{
    if (m_wind.isNull()) {
        m_wind = new Weather::Wind(this);
        QQmlEngine::setObjectOwnership(m_wind, QQmlEngine::CppOwnership);
    }
    return m_wind;
}
