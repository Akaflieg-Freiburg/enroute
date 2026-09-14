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

#include <QObject>
#include <QQmlEngine>

#include "positioning/PositionInfo.h"

namespace Flightlog {

/*! \brief Abstract base class for automatic flight detection strategies
 *
 *  This class defines the interface for automatic takeoff and landing
 *  detection. Concrete subclasses implement detection logic appropriate
 *  for different types of flying (e.g. powered aircraft, paragliders).
 *
 *  The detection state machine has four states:
 *  - **Idle**: On the ground, monitoring for takeoff.
 *  - **TakeoffPhase**: Takeoff indicators detected, waiting for confirmation.
 *  - **InFlight**: Confirmed airborne, monitoring for landing.
 *  - **LandingPhase**: Landing indicators detected, waiting for confirmation.
 *
 *  Subclasses decide how to transition between these states based on
 *  position, speed, altitude, and other sensor data.
 */
class FlightDetector : public QObject
{
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_OBJECT

public:
    /*! \brief Detection state for automatic takeoff/landing detection
     *
     *  This enum is shared by all detector implementations.
     */
    enum DetectionState : quint8
    {
        Idle,          /*!< On ground, monitoring for takeoff */
        TakeoffPhase,  /*!< Takeoff indicators detected, waiting for confirmation */
        InFlight,      /*!< Confirmed airborne, monitoring for landing */
        LandingPhase   /*!< Landing indicators detected, waiting for confirmation */
    };
    Q_ENUM(DetectionState)

    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FlightDetector(QObject* parent = nullptr) : QObject(parent) {}

    /*! \brief Standard destructor */
    ~FlightDetector() override = default;

    /*! \brief Current detection state
     *
     *  @returns The current detection state
     */
    [[nodiscard]] virtual auto detectionState() const -> DetectionState = 0;

    /*! \brief Process a position update
     *
     *  Called on every position update when auto-detection is enabled.
     *  Subclasses implement the state machine transitions here.
     *
     *  @param info The current position information
     */
    virtual void processPositionUpdate(const Positioning::PositionInfo& info) = 0;

    /*! \brief Manually end the current in-flight recording
     *
     *  If the detector is in the InFlight or LandingPhase state, this creates
     *  a completed flight with the current UTC time as landing time and
     *  resolves the nearest airport if available. The accumulated landing
     *  count from LandingPhase is preserved. Resets the detection state
     *  to Idle.
     *
     *  Does nothing if the detection state is Idle or TakeoffPhase.
     */
    virtual void endFlight() = 0;

    /*! \brief Reset the detection state to Idle
     *
     *  Clears all pending detection data and returns to the Idle state.
     *  Used when auto-detection is disabled while detection is in progress.
     */
    virtual void resetDetection() = 0;

signals:
    /*! \brief Emitted when the detection state changes */
    void detectionStateChanged();

    /*! \brief Emitted when a takeoff is confirmed
     *
     *  @param departureICAO ICAO code of the departure airport (may be empty)
     *  @param departureCoordinate Coordinate of the departure airport
     *  @param startTime The takeoff time
     *  @param aircraftCallsign Callsign of the aircraft
     */
    void takeoffDetected(const QString& departureICAO,
                         const QGeoCoordinate& departureCoordinate,
                         const QDateTime& startTime,
                         const QString& aircraftCallsign);

    /*! \brief Emitted when a landing is confirmed
     *
     *  Emitted on LandingPhase → Idle transition, after speed drops below
     *  the confirmation threshold.
     *
     *  @param arrivalICAO ICAO code of the arrival airport (may be empty)
     *  @param arrivalCoordinate Coordinate of the arrival airport
     *  @param landingTime The landing time
     *  @param landingCount Number of landings (1 for normal, >1 for touch-and-go)
     */
    void landingDetected(const QString& arrivalICAO,
                         const QGeoCoordinate& arrivalCoordinate,
                         const QDateTime& landingTime,
                         int landingCount);

private:
    Q_DISABLE_COPY_MOVE(FlightDetector)
};

} // namespace Flightlog
