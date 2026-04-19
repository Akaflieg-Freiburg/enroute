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
 *  - **Takeoff**: Ground speed > 50 km/h within 5 km of an airfield,
 *    confirmed by altitude gain of at least 200 ft above airfield elevation
 *    within 60 seconds.
 *  - **Landing**: Altitude below 100 ft above airfield elevation within
 *    5 km of an airfield.
 *  - **Abort**: Speed drops below 25 km/h or 60 seconds elapse in
 *    TakeoffPhase without altitude confirmation.
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

    void processPositionUpdate(Positioning::PositionInfo info) override;

    void endFlight() override;

    void resetDetection() override;

private:
    Q_DISABLE_COPY_MOVE(AirplaneFlightDetector)

    DetectionState m_detectionState {Idle};

    // Pending data accumulated during TakeoffPhase / InFlight
    QString m_pendingDepartureICAO;
    QGeoCoordinate m_pendingDepartureCoordinate;
    QDateTime m_pendingStartTime;
    Units::Distance m_pendingDepartureElevation;

    // Detection thresholds
    static constexpr double takeoffSpeedKMH = 50.0;          ///< Minimum ground speed to trigger takeoff detection
    static constexpr double altitudeGainFT = 200.0;          ///< Minimum altitude above airfield elevation to confirm takeoff
    static constexpr double landingAltitudeAGLFT = 100.0;    ///< Maximum altitude above airfield elevation to detect landing
    static constexpr double airfieldProximityM = 5000.0;     ///< Maximum distance to an airfield for detection
    static constexpr double maxTakeoffAltitudeAGLFT = 500.0; ///< Maximum altitude above airfield to consider a takeoff
    static constexpr double takeoffAbortSpeedFactor = 0.5;   ///< Speed drop factor to abort takeoff detection
};

} // namespace Flightlog
