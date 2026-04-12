/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include "GlobalObject.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/Waypoint.h"
#include "flightlog/AirplaneFlightDetector.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"
#include "units/Speed.h"

using namespace Qt::Literals::StringLiterals;


Flightlog::AirplaneFlightDetector::AirplaneFlightDetector(QObject* parent)
    : FlightDetector(parent)
{
}


void Flightlog::AirplaneFlightDetector::processPositionUpdate(Positioning::PositionInfo info)
{
    if (!info.isValid()) {
        return;
    }

    auto* geoMapProvider = GlobalObject::geoMapProvider();
    if (geoMapProvider == nullptr) {
        return;
    }

    auto groundSpeed = info.groundSpeed();
    auto altitudeAMSL = info.trueAltitudeAMSL();

    switch (m_detectionState) {

    case Idle: {
        // Need valid ground speed to detect takeoff
        if (!groundSpeed.isFinite()) {
            return;
        }

        // Speed must exceed takeoff threshold
        if (groundSpeed < Units::Speed::fromKMH(takeoffSpeedKMH)) {
            return;
        }

        // Check if near an airfield
        auto nearby = geoMapProvider->nearbyWaypoints(info.coordinate(), u"AD"_s);
        if (nearby.isEmpty()) {
            return;
        }

        auto closestAD = nearby.first();
        auto distToAD = info.coordinate().distanceTo(closestAD.coordinate());
        if (distToAD > airfieldProximityM) {
            return;
        }

        // Near airfield and speed above threshold → enter takeoff phase
        m_pendingDepartureICAO = closestAD.ICAOCode();
        m_pendingDepartureCoordinate = closestAD.coordinate();
        m_pendingDepartureElevation = Units::Distance::fromM(closestAD.coordinate().altitude());
        m_pendingStartTime = info.timestamp();
        m_detectionState = TakeoffPhase;
        emit detectionStateChanged();
        break;
    }

    case TakeoffPhase: {
        // Abort takeoff detection if speed drops significantly
        // or if more than 1 minute has elapsed without altitude confirmation
        if ((groundSpeed.isFinite() && groundSpeed < Units::Speed::fromKMH(takeoffSpeedKMH * takeoffAbortSpeedFactor))
            || (m_pendingStartTime.isValid() && m_pendingStartTime.secsTo(info.timestamp()) > 60)) {
            resetDetection();
            return;
        }

        // Check if altitude has gained enough above the airfield to confirm takeoff
        if (altitudeAMSL.isFinite() && m_pendingDepartureElevation.isFinite()
            && (altitudeAMSL - m_pendingDepartureElevation) > Units::Distance::fromFT(altitudeGainFT)) {
            // Build the preliminary flight entry
            Flight prelimFlight;
            prelimFlight.setDepartureICAO(m_pendingDepartureICAO);
            prelimFlight.setDepartureCoordinate(m_pendingDepartureCoordinate);
            prelimFlight.setStartTime(m_pendingStartTime);
            prelimFlight.setAircraftCallsign(GlobalObject::navigator()->aircraft().name());

            m_detectionState = InFlight;
            emit detectionStateChanged();
            emit takeoffDetected(prelimFlight, m_pendingStartTime.toUTC().time().toString(u"HH:mm"_s));
        }
        break;
    }

    case InFlight: {
        // Need valid AMSL altitude to detect landing
        if (!altitudeAMSL.isFinite()) {
            return;
        }

        // Check if near an airport
        auto nearby = geoMapProvider->nearbyWaypoints(info.coordinate(), u"AD"_s);
        if (nearby.isEmpty()) {
            return;
        }

        auto closestAD = nearby.first();
        auto distToAD = info.coordinate().distanceTo(closestAD.coordinate());
        if (distToAD > airfieldProximityM) {
            return;
        }

        // Check altitude above the airfield elevation
        auto airfieldElevation = Units::Distance::fromM(closestAD.coordinate().altitude());
        if (!airfieldElevation.isFinite()
            || (altitudeAMSL - airfieldElevation) > Units::Distance::fromFT(landingAltitudeAGLFT)) {
            return;
        }

        // Near airport and low altitude → landing detected
        auto landingTime = info.timestamp();
        auto timeStr = landingTime.toUTC().time().toString(u"HH:mm"_s);

        // Reset state before emitting signal
        m_detectionState = Idle;
        m_pendingDepartureICAO.clear();
        m_pendingDepartureCoordinate = {};
        m_pendingDepartureElevation = {};
        m_pendingStartTime = {};
        emit detectionStateChanged();
        emit landingDetected(closestAD.ICAOCode(), closestAD.coordinate(), landingTime, timeStr);
        break;
    }

    } // switch
}


void Flightlog::AirplaneFlightDetector::endFlight()
{
    if (m_detectionState != InFlight) {
        return;
    }

    auto now = QDateTime::currentDateTimeUtc();

    // Try to find the nearest airport based on current position
    QString arrivalICAO;
    QGeoCoordinate arrivalCoordinate;

    auto* positionProvider = GlobalObject::positionProvider();
    auto* geoMapProvider = GlobalObject::geoMapProvider();
    if ((positionProvider != nullptr) && (geoMapProvider != nullptr)) {
        auto info = positionProvider->positionInfo();
        if (info.isValid()) {
            auto nearby = geoMapProvider->nearbyWaypoints(info.coordinate(), u"AD"_s);
            if (!nearby.isEmpty()) {
                auto closestAD = nearby.first();
                auto distToAD = info.coordinate().distanceTo(closestAD.coordinate());
                if (distToAD < airfieldProximityM) {
                    arrivalICAO = closestAD.ICAOCode();
                    arrivalCoordinate = closestAD.coordinate();
                }
            }
        }
    }

    auto timeStr = now.time().toString(u"HH:mm"_s);

    // Reset state before emitting signal
    m_detectionState = Idle;
    m_pendingDepartureICAO.clear();
    m_pendingDepartureCoordinate = {};
    m_pendingDepartureElevation = {};
    m_pendingStartTime = {};
    emit detectionStateChanged();
    emit landingDetected(arrivalICAO, arrivalCoordinate, now, timeStr);
}


void Flightlog::AirplaneFlightDetector::resetDetection()
{
    if (m_detectionState == Idle) {
        return;
    }

    m_detectionState = Idle;
    m_pendingDepartureICAO.clear();
    m_pendingDepartureCoordinate = {};
    m_pendingDepartureElevation = {};
    m_pendingStartTime = {};
    emit detectionStateChanged();
}
