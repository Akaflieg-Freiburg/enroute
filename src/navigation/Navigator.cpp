/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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
#include "GlobalSettings.h"
#include "Librarian.h"
#include "dataManagement/DataManager.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"


//
// Constructors and destructors
//

Navigation::Navigator::~Navigator()
{
    // Break all bindings before destruction proceeds
    m_hasAviationMapForCurrentLocation.takeBinding();
}


namespace {
const auto activeAircraftKey = u"Aircraft/activeName"_s;
} // namespace


Navigation::Navigator::Navigator(QObject *parent) : GlobalObject(parent)
{
    // Restore wind
    QSettings const settings;
    m_wind.setSpeed(Units::Speed::fromKN(settings.value(QStringLiteral("Wind/windSpeedInKT"), qQNaN()).toDouble()));
    m_wind.setDirectionFrom( Units::Angle::fromDEG(settings.value(QStringLiteral("Wind/windDirectionInDEG"), qQNaN()).toDouble()) );

    // Restore aircraft. An entry that can no longer be read leaves no aircraft active.
    migrateLegacyAircraft();
    auto const name = QSettings().value(activeAircraftKey).toString();
    Navigation::Aircraft newAircraft;
    if (!name.isEmpty() && newAircraft.loadFromJSON(Librarian::fullPath(Librarian::Aircraft, name)).isEmpty())
    {
        m_aircraftName = name;
        m_aircraft = newAircraft;
    }
}


void Navigation::Navigator::deferredInitialization()
{
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateAltitudeLimit);
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateFlightStatus);

    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::updateRemainingRouteInfo);
    connect(this, &Navigation::Navigator::aircraftChanged, this, [this](){ updateRemainingRouteInfo(); });

    // Follow changes to the library entry of the active aircraft
    auto* librarian = GlobalObject::librarian();
    connect(librarian, &Librarian::entrySaved, this, [this](Librarian::Library library, const QString& baseName) {
        if ((library == Librarian::Aircraft) && !m_aircraftName.isEmpty() && (baseName == m_aircraftName))
        {
            (void)selectAircraft(baseName);
        }
    });
    connect(librarian, &Librarian::entryRemoved, this, [this](Librarian::Library library, const QString& baseName) {
        if ((library == Librarian::Aircraft) && !m_aircraftName.isEmpty() && (baseName == m_aircraftName))
        {
            (void)selectAircraft({});
        }
    });
    connect(librarian, &Librarian::entryRenamed, this, [this](Librarian::Library library, const QString& oldName, const QString& newName) {
        if ((library == Librarian::Aircraft) && !m_aircraftName.isEmpty() && (oldName == m_aircraftName))
        {
            m_aircraftName = newName;
            QSettings().setValue(activeAircraftKey, newName);
            emit aircraftChanged();
        }
    });
    connect(this, &Navigation::Navigator::windChanged, this, [this](){ updateRemainingRouteInfo(); });
    connect(flightRoute(), &Navigation::FlightRoute::waypointsChanged, this, [this](){ updateRemainingRouteInfo(); });

    connect(GlobalObject::dataManager()->aviationMaps(), &DataManagement::Downloadable_MultiFile::downloadablesChanged, this,
            [this]() { m_aviationMapsGeneration = m_aviationMapsGeneration.value() + 1; });
    m_hasAviationMapForCurrentLocation.setBinding([this]() {return computeHasAviationMapForCurrentLocation();});
}


//
// Getter Methods
//

Navigation::FlightRoute* Navigation::Navigator::flightRoute()
{
    if (m_flightRoute.isNull())
    {
        m_flightRoute = new FlightRoute(this);
        m_flightRoute->load(m_flightRouteFileName);
        connect(m_flightRoute, &Navigation::FlightRoute::waypointsChanged, this, [this]() {if (m_flightRoute != nullptr) {(void)m_flightRoute->save(m_flightRouteFileName);}});
        QQmlEngine::setObjectOwnership(m_flightRoute, QQmlEngine::CppOwnership);
    }
    return m_flightRoute;
}


//
// Setter Methods
//

QString Navigation::Navigator::selectAircraft(const QString& baseName)
{
    Navigation::Aircraft newAircraft;
    if (!baseName.isEmpty())
    {
        auto errorMsg = newAircraft.loadFromJSON(Librarian::fullPath(Librarian::Aircraft, baseName));
        if (!errorMsg.isEmpty())
        {
            return errorMsg;
        }
    }

    if ((baseName == m_aircraftName) && (newAircraft == m_aircraft.value()))
    {
        return {};
    }
    QSettings().setValue(activeAircraftKey, baseName);
    m_aircraftName = baseName;
    m_aircraft = newAircraft;
    emit aircraftChanged();
    return {};
}


void Navigation::Navigator::migrateLegacyAircraft()
{
    auto const legacyFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/aircraft.json"_s;
    QFile file(legacyFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    Navigation::Aircraft acft;
    auto const errorMsg = acft.loadFromJSON(file.readAll());
    file.close();

    // An unreadable or never-edited aircraft is not worth a library entry
    if (errorMsg.isEmpty() && (acft != Navigation::Aircraft()))
    {
        auto name = Librarian::find(acft);
        if (name.isEmpty())
        {
            name = Librarian::uniqueName(Librarian::Aircraft, acft.name().isEmpty() ? tr("My Aircraft") : acft.name());
            auto const saveError = acft.save(Librarian::fullPath(Librarian::Aircraft, name));
            if (!saveError.isEmpty())
            {
                // Keep the legacy file, so that the next start tries again
                qWarning() << "Navigator: cannot migrate aircraft:" << saveError;
                return;
            }
        }
        QSettings().setValue(activeAircraftKey, name);
    }
    QFile::remove(legacyFileName);
}


void Navigation::Navigator::setFlightStatus(FlightStatus newFlightStatus)
{
    if (m_flightStatus == newFlightStatus)
    {
        return;
    }

    m_flightStatus = newFlightStatus;
    emit flightStatusChanged();
}


void Navigation::Navigator::setWind(Weather::Wind newWind)
{
    if (newWind == m_wind)
    {
        return;
    }

    // Save wind
    QSettings settings;
    settings.setValue(QStringLiteral("Wind/windSpeedInKT"), newWind.speed().toKN());
    settings.setValue(QStringLiteral("Wind/windDirectionInDEG"), newWind.directionFrom().toDEG());

    // Set new wind
    m_wind = newWind;
    emit windChanged();
}


//
// Slots
//

void Navigation::Navigator::updateAltitudeLimit()
{  
    auto info = GlobalObject::positionProvider()->positionInfo();
    if (!info.isValid())
    {
        return;
    }

    auto altLimit = GlobalObject::globalSettings()->airspaceAltitudeLimit();
    auto trueAltitude = info.trueAltitudeAMSL();
    if (altLimit.isFinite() &&
        trueAltitude.isFinite() &&
        (trueAltitude + Units::Distance::fromFT(1000) > altLimit))
    {
        // Round trueAltitude+1000ft up to nearest 500ft and set that as a new limit
        auto newAltLimit = Units::Distance::fromFT(500.0*qCeil(trueAltitude.toFeet()/500.0+2.0));
        GlobalObject::globalSettings()->setAirspaceAltitudeLimit(newAltLimit);
        emit airspaceAltitudeLimitAdjusted();
    }
}


void Navigation::Navigator::updateFlightStatus()
{
    auto info = GlobalObject::positionProvider()->positionInfo();
    if (!info.isValid())
    {
        setFlightStatus(Unknown);
        return;
    }

    // Get ground speed and aircraft minimum speed
    auto GS = info.groundSpeed();
    auto aircraftMinSpeed = m_aircraft.value().minimumSpeed();
    if (!GS.isFinite() || !aircraftMinSpeed.isFinite())
    {
        setFlightStatus(Unknown);
        return;
    }

    // Go to ground mode if ground speed is less then aircraftMinSpeed-flightSpeedHysteresis
    if (m_flightStatus == Flight)
    {
        // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
        if ( GS < aircraftMinSpeed-flightSpeedHysteresis)
        {
            setFlightStatus(Ground);
        }
        return;
    }

    // Go to flight mode if ground speed is more than aircraftMinSpeed
    if ( GS > aircraftMinSpeed )
    {
        setFlightStatus(Flight);
    }
}


void Navigation::Navigator::updateRemainingRouteInfo()
{
    auto info = GlobalObject::positionProvider()->positionInfo();

    // If there are no waypoints, then there is no remaining route info
    auto geoPath = flightRoute()->geoPath();
    if (geoPath.isEmpty())
    {
        RemainingRouteInfo rrInfo;
        rrInfo.status = RemainingRouteInfo::NoRoute;
        m_remainingRouteInfo = rrInfo;
        return;
    }

    // If there is no valid positionInfo, then there is no remaining route info
    if (!info.isValid())
    {
        RemainingRouteInfo rrInfo;
        rrInfo.status = RemainingRouteInfo::PositionUnknown;
        m_remainingRouteInfo = rrInfo;
        return;
    }

    // If we are closer than 3 nm from endpoint, then we do not give a remaining route info
    const auto &finalCoordinate = geoPath[geoPath.size() - 1];
    if (Units::Distance::fromM(finalCoordinate.distanceTo(info.coordinate())) < Leg::nearThreshold)
    {
        RemainingRouteInfo rrInfo;
        rrInfo.status = RemainingRouteInfo::NearDestination;
        m_remainingRouteInfo = rrInfo;
        return;
    }

    //
    // Figure out what the current leg is
    //
    auto legs = flightRoute()->legs();

    // If the flight route contains one waypoint only, then create an artificial leg from the current position
    // to the one waypoint of the route.
    if (flightRoute()->size() == 1)
    {
        auto start = Positioning::PositionProvider::lastValidCoordinate();
        auto end = flightRoute()->waypoints()[0];
        legs += Leg(start, end);
    }

    // Check legs that we are following, and take the last one
    qsizetype currentLeg = -1;
    for(auto i=legs.size()-1; i>=0; i--)
    {
        if (legs[i].isFollowing(info))
        {
            currentLeg = i;
            break;
        }
    }

    // If no current leg found, check legs that we are near to, and take the last one.
    if (currentLeg < 0)
    {
        for(auto i=legs.size()-1; i>=0; i--)
        {
            if (legs[i].isNear(info))
            {
                currentLeg = i;
                break;
            }
        }
    }

    // If still no current leg found, then abort
    if (currentLeg < 0)
    {
        RemainingRouteInfo rrInfo;
        rrInfo.status = RemainingRouteInfo::OffRoute;
        m_remainingRouteInfo = rrInfo;
        return;
    }

    //
    //
    //
    RemainingRouteInfo rri;
    rri.status = RemainingRouteInfo::OnRoute;
    Leg const legToNextWP(info.coordinate(), legs[currentLeg].endPoint());
    auto dist = legToNextWP.distance();
    auto ETE = legToNextWP.ETE(m_wind, m_aircraft);

    // If bearing is less than +/- 30°, then use current ground speed to
    // estimate ETE for leg to next waypoint
    if (legToNextWP.TC().isFinite() &&
            info.trueTrack().isFinite() &&
            info.groundSpeed().isFinite() &&
            (info.groundSpeed() > Units::Speed::fromKN(10.0)) )
    {
        auto bearing = legToNextWP.TC()-info.trueTrack();
        auto bearingDEG = bearing.toDEG();

        if ((bearingDEG < 30) || (bearingDEG > 360-30))
        {
            ETE = dist/(bearing.cos()*info.groundSpeed());
        }
    }

    rri.nextWP = legToNextWP.endPoint();
    rri.nextWP_DIST = dist;
    rri.nextWP_ETE  = ETE;
    if (ETE.isFinite())
    {
        rri.nextWP_ETA = QDateTime::currentDateTimeUtc().addSecs( qRound64(rri.nextWP_ETE.toS()) );
    }
    rri.nextWP_TC = legToNextWP.TC();

    if (currentLeg < legs.size()-1)
    {
        for(auto i=currentLeg+1; i<legs.size(); i++)
        {
            dist += legs[i].distance();
            ETE += legs[i].ETE(m_wind, m_aircraft);
        }

        rri.finalWP = legs.last().endPoint();
        rri.finalWP_DIST = dist;
        rri.finalWP_ETE  = ETE;
        if (ETE.isFinite())
        {
            rri.finalWP_ETA = QDateTime::currentDateTimeUtc().addSecs( qRound64(rri.finalWP_ETE.toS()) ).toUTC();
        }
    }

    QStringList complaints;
    if (!m_aircraft.value().cruiseSpeed().isFinite())
    {
        complaints += tr("Cruise speed not specified.");
    }
    if (!m_wind.speed().isFinite())
    {
        complaints += tr("Wind speed not specified.");
    }
    if (!m_wind.directionFrom().isFinite())
    {
        complaints += tr("Wind direction not specified.");
    }
    if (!complaints.isEmpty())
    {
        rri.note = tr("Computation incomplete. %1").arg(complaints.join(QStringLiteral(" ")));
    }

    m_remainingRouteInfo = rri;
}


bool Navigation::Navigator::computeHasAviationMapForCurrentLocation()
{
    // Register a dependency on the map list generation, see Navigator.h
    (void)m_aviationMapsGeneration.value();

    auto coordinate = GlobalObject::positionProvider()->approximateLastValidCoordinate();
    auto aviationMaps = GlobalObject::dataManager()->aviationMaps()->downloadables();
    for(auto* map : std::as_const(aviationMaps))
    {
        if (map->hasFile() && map->boundingBox().contains(coordinate))
        {
            return true;
        }
    }
    return false;
}
