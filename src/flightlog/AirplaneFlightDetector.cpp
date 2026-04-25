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
#include "flightlog/AirplaneFlightDetector.h"
#include "flightlog/FlightLog.h"
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
        auto closestAD = FlightLog::nearestAirfield(info.coordinate());
        if (!closestAD.isValid()) {
            return;
        }

        // If already well above the airfield, this is not a takeoff
        auto airfieldElevation = Units::Distance::fromM(closestAD.coordinate().altitude());
        if (altitudeAMSL.isFinite() && airfieldElevation.isFinite()
            && (altitudeAMSL - airfieldElevation) > Units::Distance::fromFT(maxTakeoffAltitudeAGLFT)) {
            return;
        }

        // Near airfield and speed above threshold → enter takeoff phase
        m_pendingDepartureICAO = closestAD.shortName();
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
        auto closestAD = FlightLog::nearestAirfield(info.coordinate());
        if (!closestAD.isValid()) {
            return;
        }

        // Check altitude above the airfield elevation
        auto airfieldElevation = Units::Distance::fromM(closestAD.coordinate().altitude());
        if (!airfieldElevation.isFinite()
            || (altitudeAMSL - airfieldElevation) > Units::Distance::fromFT(landingAltitudeAGLFT)) {
            return;
        }

        // Near airport and low altitude → enter landing phase
        m_landingPhaseEntryTime = info.timestamp();
        m_landingCount++;
        m_detectionState = LandingPhase;
        emit detectionStateChanged();
        break;
    }

    case LandingPhase: {
        // Confirmed landing: speed drops below threshold or timeout
        if ((groundSpeed.isFinite() && groundSpeed < Units::Speed::fromKMH(landingConfirmSpeedKMH))
            || (m_landingPhaseEntryTime.isValid() && m_landingPhaseEntryTime.secsTo(info.timestamp()) > 60)) {
            // Use the time we first went low as the landing time
            auto landingTime = m_landingPhaseEntryTime.isValid() ? m_landingPhaseEntryTime : info.timestamp();
            auto timeStr = landingTime.toUTC().time().toString(u"HH:mm"_s);
            QString arrivalICAO;
            QGeoCoordinate arrivalCoordinate;
            auto closestAD = FlightLog::nearestAirfield(info.coordinate());
            if (closestAD.isValid()) {
                arrivalICAO = closestAD.shortName();
                arrivalCoordinate = closestAD.coordinate();
            }
            auto landingCount = m_landingCount;

            // Reset state before emitting signal
            m_detectionState = Idle;
            m_pendingDepartureICAO.clear();
            m_pendingDepartureCoordinate = {};
            m_pendingDepartureElevation = {};
            m_pendingStartTime = {};
            m_landingPhaseEntryTime = {};
            m_landingCount = 0;
            emit detectionStateChanged();
            emit landingDetected(arrivalICAO, arrivalCoordinate, landingTime, landingCount, timeStr);
            break;
        }

        // Go-around / touch-and-go: altitude gain above airfield → end current leg, start new one
        if (altitudeAMSL.isFinite()) {
            auto closestAD2 = FlightLog::nearestAirfield(info.coordinate());
            if (closestAD2.isValid()) {
                auto elev = Units::Distance::fromM(closestAD2.coordinate().altitude());
                if (elev.isFinite()
                    && (altitudeAMSL - elev) > Units::Distance::fromFT(altitudeGainFT)) {
                    // Close the current leg by going through Idle first.
                    // This ensures onLandingDetected saves the track while
                    // m_flights[0] is still the current leg.
                    auto landingTime = m_landingPhaseEntryTime.isValid() ? m_landingPhaseEntryTime : info.timestamp();
                    auto timeStr = landingTime.toUTC().time().toString(u"HH:mm"_s);
                    auto landingCount = m_landingCount;

                    m_detectionState = Idle;
                    m_landingPhaseEntryTime = {};
                    m_landingCount = 0;
                    emit detectionStateChanged();
                    emit landingDetected(closestAD2.shortName(), closestAD2.coordinate(), landingTime, landingCount, timeStr);

                    // Start a new leg from the touch-and-go airport
                    Flight newLeg;
                    newLeg.setDepartureICAO(closestAD2.shortName());
                    newLeg.setDepartureCoordinate(closestAD2.coordinate());
                    newLeg.setStartTime(landingTime);
                    newLeg.setAircraftCallsign(GlobalObject::navigator()->aircraft().name());

                    m_pendingDepartureICAO = closestAD2.shortName();
                    m_pendingDepartureCoordinate = closestAD2.coordinate();
                    m_pendingDepartureElevation = elev;
                    m_pendingStartTime = landingTime;
                    m_detectionState = InFlight;
                    emit detectionStateChanged();
                    emit takeoffDetected(newLeg, timeStr);
                }
            }
        }
        break;
    }

    } // switch
}


void Flightlog::AirplaneFlightDetector::endFlight()
{
    if (m_detectionState != InFlight && m_detectionState != LandingPhase) {
        return;
    }

    auto now = QDateTime::currentDateTimeUtc();

    // Try to find the nearest airport based on current position
    QString arrivalICAO;
    QGeoCoordinate arrivalCoordinate;
    auto* positionProvider = GlobalObject::positionProvider();
    if (positionProvider != nullptr) {
        auto info = positionProvider->positionInfo();
        if (info.isValid()) {
            auto closestAD = FlightLog::nearestAirfield(info.coordinate());
            if (closestAD.isValid()) {
                arrivalICAO = closestAD.shortName();
                arrivalCoordinate = closestAD.coordinate();
            }
        }
    }
    auto landingCount = (m_detectionState == LandingPhase) ? m_landingCount : 1;

    auto timeStr = now.time().toString(u"HH:mm"_s);

    // Reset state before emitting signal
    m_detectionState = Idle;
    m_pendingDepartureICAO.clear();
    m_pendingDepartureCoordinate = {};
    m_pendingDepartureElevation = {};
    m_pendingStartTime = {};
    m_landingPhaseEntryTime = {};
    m_landingCount = 0;
    emit detectionStateChanged();
    emit landingDetected(arrivalICAO, arrivalCoordinate, now, landingCount, timeStr);
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
    m_landingPhaseEntryTime = {};
    m_landingCount = 0;
    emit detectionStateChanged();
}
