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


void Flightlog::AirplaneFlightDetector::processPositionUpdate(const Positioning::PositionInfo& info)
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
        if (groundSpeed < aircraftMinimumSpeed()) {
            return;
        }

        // Check if near an airfield
        auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
        if (!closestAD.isValid()) {
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
        if ((groundSpeed.isFinite() && groundSpeed < takeoffAbortSpeedFactor * aircraftMinimumSpeed())
            || (m_pendingStartTime.isValid() && m_pendingStartTime.secsTo(info.timestamp()) > takeoffConfirmTimeoutS)) {
            resetDetection();
            return;
        }

        // Check if altitude has gained enough above the airfield to confirm takeoff
        if (altitudeAMSL.isFinite() && m_pendingDepartureElevation.isFinite()
            && (altitudeAMSL - m_pendingDepartureElevation) > Units::Distance::fromFT(altitudeGainFT)) {
            m_detectionState = InFlight;
            emit detectionStateChanged();
            emit takeoffDetected(m_pendingDepartureICAO,
                                 m_pendingDepartureCoordinate,
                                 m_pendingStartTime,
                                 GlobalObject::navigator()->aircraft().name(),
                                 m_pendingStartTime.toUTC().time().toString(u"HH:mm"_s));
        }
        break;
    }

    case InFlight: {
        // Safety valve: if we have been InFlight for more than maxFlightDurationH
        // with no GPS speed (e.g. landed at an unmapped field), auto-end the flight
        // so the recorder doesn't grow unbounded.  A real flight in progress will
        // have a valid ground speed above the minimum, so it won't be affected.
        if (m_pendingStartTime.isValid()
            && m_pendingStartTime.secsTo(info.timestamp()) > static_cast<qint64>(maxFlightDurationH * 3600)
            && (!groundSpeed.isFinite() || groundSpeed < aircraftMinimumSpeed())) {
            endFlight();
            return;
        }

        // Need valid AMSL altitude to detect landing
        if (!altitudeAMSL.isFinite()) {
            return;
        }

        // Skip the expensive airfield lookup while well above ground
        auto altAGL = info.trueAltitudeAGL();
        if (altAGL.isFinite() && altAGL > Units::Distance::fromFT(landingAltitudeAGLFT * 3.0)) {
            return;
        }

        // Check if near an airport
        auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
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
        m_detectionState = LandingPhase;
        emit detectionStateChanged();
        break;
    }

    case LandingPhase: {
        // Confirmed landing: speed drops below threshold or timeout
        if ((groundSpeed.isFinite() && groundSpeed < aircraftMinimumSpeed())
            || (m_landingPhaseEntryTime.isValid() && m_landingPhaseEntryTime.secsTo(info.timestamp()) > landingConfirmTimeoutS)) {
            // Use the time we first went low as the landing time
            auto landingTime = m_landingPhaseEntryTime.isValid() ? m_landingPhaseEntryTime : info.timestamp();
            auto timeStr = landingTime.toUTC().time().toString(u"HH:mm"_s);
            QString arrivalICAO;
            QGeoCoordinate arrivalCoordinate;
            auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
            if (closestAD.isValid()) {
                arrivalICAO = closestAD.shortName();
                arrivalCoordinate = closestAD.coordinate();
            }
            m_landingCount++;
            auto landingCount = m_landingCount;

            // Reset state before emitting signal
            m_detectionState = Idle;
            clearPendingState();
            emit detectionStateChanged();
            emit landingDetected(arrivalICAO, arrivalCoordinate, landingTime, landingCount, timeStr);
            break;
        }

        // Aborted approach: climbed back above the landing threshold without
        // touching down — revert to InFlight, no landing recorded.
        if (altitudeAMSL.isFinite()) {
            auto closestAD2 = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
            if (closestAD2.isValid()) {
                auto elev = Units::Distance::fromM(closestAD2.coordinate().altitude());
                if (elev.isFinite()
                    && (altitudeAMSL - elev) > Units::Distance::fromFT(landingAltitudeAGLFT)) {
                    // Count the low pass as a landing. GPS data cannot reliably
                    // distinguish a touch-and-go from a balked landing / go-around,
                    // so any approach that descended below the landing threshold near
                    // an airfield is counted — intentional conservative design.
                    m_landingCount++;
                    m_landingPhaseEntryTime = {};
                    m_detectionState = InFlight;
                    emit detectionStateChanged();
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
            auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
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
    clearPendingState();
    emit detectionStateChanged();
    emit landingDetected(arrivalICAO, arrivalCoordinate, now, landingCount, timeStr);
}


void Flightlog::AirplaneFlightDetector::resetDetection()
{
    if (m_detectionState == Idle) {
        return;
    }

    m_detectionState = Idle;
    clearPendingState();
    emit detectionStateChanged();
}


void Flightlog::AirplaneFlightDetector::clearPendingState()
{
    m_pendingDepartureICAO.clear();
    m_pendingDepartureCoordinate = {};
    m_pendingDepartureElevation = {};
    m_pendingStartTime = {};
    m_landingPhaseEntryTime = {};
    m_landingCount = 0;
}


auto Flightlog::AirplaneFlightDetector::aircraftMinimumSpeed() const -> Units::Speed
{
    auto minSpeed = GlobalObject::navigator()->aircraft().minimumSpeed();
    if (minSpeed.isFinite()) {
        return minSpeed;
    }
    return Units::Speed::fromKMH(defaultTakeoffSpeedKMH);
}
