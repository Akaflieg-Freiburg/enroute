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
            return;
        }

        // If already well above the airfield, this is not a takeoff
        auto airfieldElevation = Units::Distance::fromM(closestAD.coordinate().altitude());
        if (altitudeAMSL.isFinite() && airfieldElevation.isFinite()
            && (altitudeAMSL - airfieldElevation) > Units::Distance::fromFT(maxTakeoffAltitudeAGLFT)) {
            return;
        }

        // Near airfield and speed above threshold → enter takeoff phase.
        // The departure coordinate is the real GPS fix, not the airfield's
        // charted position.
        m_pendingDepartureICAO = closestAD.shortName();
        m_pendingDepartureCoordinate = info.coordinate();
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
                                 GlobalObject::navigator()->aircraft().name());
        }
        break;
    }

    case InFlight: {
        // Outlanding detection: sustained low ground speed close to the
        // terrain directly underneath (not tied to any airfield) is treated
        // as a landing on its own. This is what makes an outlanding away
        // from any charted airfield get detected automatically and
        // promptly, rather than relying on the pilot to press "End Flight"
        // or waiting for the many-hours-long safety valve below. The
        // altitude gate matters: ground speed alone can read near zero
        // while still fully airborne — e.g. a glider circling in a strong
        // headwind — so low speed is only trusted close to the ground.
        auto altAGL = info.trueAltitudeAGL();
        const bool nearGround = altAGL.isFinite() && altAGL < Units::Distance::fromFT(landingAltitudeAGLFT);
        const bool lowSpeed = groundSpeed.isFinite() && groundSpeed < aircraftMinimumSpeed();
        if (nearGround && lowSpeed) {
            if (!m_lowSpeedEntryTime.isValid()) {
                m_lowSpeedEntryTime = info.timestamp();
            } else if (m_lowSpeedEntryTime.secsTo(info.timestamp()) >= landingConfirmTimeoutS) {
                endFlight();
                return;
            }
        } else {
            m_lowSpeedEntryTime = {};
        }

        // Last-resort safety valve: if we have been InFlight for more than
        // maxFlightDurationH with no GPS speed, auto-end the flight so the
        // recorder doesn't grow unbounded. Only reachable when the check
        // above can't engage (e.g. no terrain elevation data for this
        // region). A real flight in progress will have a valid ground speed
        // above the minimum, so it won't be affected.
        if (m_pendingStartTime.isValid()
            && m_pendingStartTime.secsTo(info.timestamp()) > static_cast<qint64>(maxFlightDurationH * 3600)
            && (!groundSpeed.isFinite() || groundSpeed < aircraftMinimumSpeed())) {
            endFlight();
            return;
        }

        // Need valid AMSL altitude to detect landing near an airfield
        if (!altitudeAMSL.isFinite()) {
            return;
        }

        // Skip the expensive airfield lookup while well above ground
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
        m_lowSpeedEntryTime = {};
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
            // Arrival coordinate is the real GPS fix, not the airfield's
            // charted position; the ICAO code is only a label, when found.
            QString arrivalICAO;
            auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
            if (closestAD.isValid()) {
                arrivalICAO = closestAD.shortName();
            }
            auto arrivalCoordinate = info.coordinate();
            m_landingCount++;
            auto landingCount = m_landingCount;

            // Reset state before emitting signal
            m_detectionState = Idle;
            clearPendingState();
            emit detectionStateChanged();
            emit landingDetected(arrivalICAO, arrivalCoordinate, landingTime, landingCount);
            break;
        }

        // Aborted approach: climbed back above the landing threshold without
        // touching down — revert to InFlight, no landing recorded.
        if (altitudeAMSL.isFinite()) {
            auto closestAD2 = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
            if (closestAD2.isValid()) {
                auto elev = Units::Distance::fromM(closestAD2.coordinate().altitude());
                if (elev.isFinite()
                    && (altitudeAMSL - elev) > Units::Distance::fromFT(altitudeGainFT)) {
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

    // Capture the real GPS position, regardless of whether an airfield is
    // nearby — this is what makes an outlanding away from any charted
    // airfield still get a usable coordinate. The ICAO code is only a
    // label, filled in when an airfield happens to be close by.
    QString arrivalICAO;
    QGeoCoordinate arrivalCoordinate;
    auto* positionProvider = GlobalObject::positionProvider();
    if (positionProvider != nullptr) {
        auto info = positionProvider->positionInfo();
        if (info.isValid()) {
            arrivalCoordinate = info.coordinate();
            auto closestAD = FlightLog::nearestAirfield(info.coordinate(), airfieldProximityM);
            if (closestAD.isValid()) {
                arrivalICAO = closestAD.shortName();
            }
        }
    }
    // endFlight() is only ever invoked externally (manual "End Flight",
    // or the InFlight safety valve) to finalize a landing that the
    // automatic LandingPhase confirmation hasn't completed yet — so this
    // call always accounts for exactly one more landing than whatever
    // prior touch-and-goes already incremented m_landingCount to.
    auto landingCount = m_landingCount + 1;

    // Reset state before emitting signal
    m_detectionState = Idle;
    clearPendingState();
    emit detectionStateChanged();
    emit landingDetected(arrivalICAO, arrivalCoordinate, now, landingCount);
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
    m_lowSpeedEntryTime = {};
}


auto Flightlog::AirplaneFlightDetector::aircraftMinimumSpeed() const -> Units::Speed
{
    auto minSpeed = GlobalObject::navigator()->aircraft().minimumSpeed();
    if (minSpeed.isFinite()) {
        return minSpeed;
    }
    return Units::Speed::fromKMH(defaultTakeoffSpeedKMH);
}
