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

#pragma once

#include "flightlog/FlightDetector.h"

using namespace Qt::Literals::StringLiterals;

namespace Flightlog {

/*! \brief Flight detection for powered aircraft
 *
 *  This detector implements a state machine suitable for powered aircraft
 *  (airplanes, motor gliders, etc.). It uses ground speed, altitude AGL,
 *  and proximity to ICAO airfields to detect takeoff and landing.
 *
 *  Detection criteria:
 *  - **Takeoff**: Ground speed exceeds the aircraft's configured minimum speed
 *    (default 50 km/h) within 5 km of an airfield, confirmed by altitude gain
 *    of at least 200 ft above airfield elevation within 60 seconds.
 *  - **Landing (near a charted airfield)**: Altitude below 100 ft above
 *    airfield elevation within 5 km of an airfield, confirmed by speed
 *    dropping below the aircraft's configured minimum speed.
 *  - **Landing (anywhere, e.g. an outlanding)**: Ground speed below the
 *    aircraft's configured minimum speed, sustained for 60 seconds, while
 *    below 100 ft above the terrain directly underneath (from elevation
 *    data, not tied to any airfield). This is what lets an outlanding away
 *    from any charted airfield still be detected automatically and
 *    promptly. The altitude gate matters: ground speed alone can read near
 *    zero while still fully airborne — e.g. a glider circling in a strong
 *    headwind — so low speed is only trusted close to the ground.
 *  - **Go-around**: While in LandingPhase, altitude gain > 200 ft above
 *    airfield elevation ends the current flight leg and starts a new one
 *    from the touch-and-go airport.
 *  - **Abort**: Speed drops below half the takeoff threshold or 60 seconds
 *    elapse in TakeoffPhase without altitude confirmation.
 */
class AirplaneFlightDetector : public FlightDetector
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit AirplaneFlightDetector(QObject* parent = nullptr);

    /*! \brief Standard destructor */
    ~AirplaneFlightDetector() override = default;

    [[nodiscard]] auto detectionState() const -> DetectionState override { return m_detectionState; }

    void processPositionUpdate(const Positioning::PositionInfo& info) override;

    void endFlight() override;

    void resetDetection() override;

private:
    Q_DISABLE_COPY_MOVE(AirplaneFlightDetector)

    DetectionState m_detectionState {Idle};

    // Pending data accumulated during TakeoffPhase / InFlight / LandingPhase
    QString m_pendingDepartureICAO;
    QGeoCoordinate m_pendingDepartureCoordinate;
    QDateTime m_pendingStartTime;
    Units::Distance m_pendingDepartureElevation;

    // Pending data accumulated during LandingPhase
    QDateTime m_landingPhaseEntryTime;
    int m_landingCount {0};

    // When sustained low ground speed near the ground (terrain AGL) was
    // first observed during InFlight, for outlanding detection away from
    // any airfield. Invalid whenever that condition isn't currently met.
    QDateTime m_lowSpeedEntryTime;

    // Detection thresholds
    static constexpr double defaultTakeoffSpeedKMH = 50.0;     ///< Fallback takeoff speed when aircraft has no minimum speed configured
    static constexpr double airfieldProximityM = 5000.0;        ///< Maximum distance to an airfield for detection
    static constexpr double altitudeGainFT = 200.0;             ///< Minimum altitude above airfield elevation to confirm takeoff
    static constexpr double landingAltitudeAGLFT = 100.0;       ///< Maximum altitude above airfield elevation (or terrain, for the airfield-independent outlanding check) to detect landing
    static constexpr double maxTakeoffAltitudeAGLFT = 500.0;    ///< Maximum altitude above airfield to consider a takeoff
    static constexpr double takeoffAbortSpeedFactor = 0.5;      ///< Speed drop factor to abort takeoff detection
    static constexpr double maxFlightDurationH = 18.0;          ///< Last-resort auto-end after this many hours InFlight, for when even the outlanding check can't confirm a landing (e.g. no terrain data for the region)
    static constexpr qint64 takeoffConfirmTimeoutS = 60;        ///< Abort takeoff detection if no altitude gain after this many seconds
    static constexpr qint64 landingConfirmTimeoutS = 60;        ///< Confirm landing (near an airfield, or the airfield-independent outlanding check) if speed stays low for this many seconds

    // Helpers
    [[nodiscard]] auto aircraftMinimumSpeed() const -> Units::Speed;
    void clearPendingState();   ///< Reset all pending members to defaults; does not change state or emit signals
};

} // namespace Flightlog
