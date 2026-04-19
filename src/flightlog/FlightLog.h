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

#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "flightlog/Flight.h"
#include "flightlog/FlightDetector.h"
#include "geomaps/Waypoint.h"

using namespace Qt::Literals::StringLiterals;

namespace Flightlog {

/*! \brief Flight log manager with pluggable automatic flight detection
 *
 *  This class manages a persistent list of flight log entries and provides
 *  CRUD operations for flights. Automatic takeoff/landing detection is
 *  delegated to a FlightDetector subclass, which can be swapped at runtime
 *  to support different types of flying (e.g. powered aircraft, paragliders).
 *
 *  The detection state is forwarded from the active FlightDetector and
 *  exposed to QML via the detectionState property.
 */

class FlightLog : public GlobalObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:

    //
    // Constructors and Destructors
    //

    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FlightLog(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit FlightLog() = delete;

    /*! \brief Standard destructor */
    ~FlightLog() override = default;

    // factory function for QML singleton
    static FlightLog* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::flightLog();
    }


    //
    // PROPERTIES
    //

    /*! \brief List of all recorded flights, newest first */
    Q_PROPERTY(QList<Flightlog::Flight> flights READ flights NOTIFY flightsChanged)
    [[nodiscard]] auto flights() const -> QList<Flight> { return m_flights; }

    /*! \brief Current state of the automatic detection state machine */
    Q_PROPERTY(Flightlog::FlightDetector::DetectionState detectionState READ detectionState NOTIFY detectionStateChanged)
    [[nodiscard]] auto detectionState() const -> FlightDetector::DetectionState;

    /*! \brief Number of recorded flights */
    Q_PROPERTY(int count READ count NOTIFY flightsChanged)
    [[nodiscard]] auto count() const -> int { return static_cast<int>(m_flights.size()); }


    //
    // Methods
    //

    /*! \brief Add a new flight to the log
     *
     *  The flight is prepended (newest first). Coordinates are resolved
     *  from the ICAO codes if possible.
     *
     *  @param flight The flight to add
     */
    Q_INVOKABLE void addFlight(const Flightlog::Flight& flight);

    /*! \brief Remove a flight from the log
     *
     *  @param index The index of the flight to remove
     */
    Q_INVOKABLE void removeFlight(int index);

    /*! \brief Update an existing flight in the log
     *
     *  Coordinates are re-resolved from the ICAO codes. If resolution
     *  fails, old coordinates are preserved.
     *
     *  @param index The index of the flight to update
     *  @param flight The updated flight data
     */
    Q_INVOKABLE void updateFlight(int index, const Flightlog::Flight& flight);

    /*! \brief Create a Flight value from individual field strings
     *
     *  This factory method is intended for QML use, where constructing
     *  a Q_GADGET value type with all properties is cumbersome.
     *
     *  @param departureICAO ICAO code of departure airport
     *  @param arrivalICAO ICAO code of arrival airport
     *  @param date Date string in yyyy-MM-dd format
     *  @param offBlockTimeStr Off-block time string in HH:mm format, or empty
     *  @param startTimeStr Start time string in HH:mm format
     *  @param landingTimeStr Landing time string in HH:mm format
     *  @param onBlockTimeStr On-block time string in HH:mm format, or empty
     *  @param pilotName Pilot name, or empty
     *  @param aircraftCallsign Aircraft callsign (e.g. D-KEBE), or empty
     *  @param comments Free-text comments, or empty
     *  @returns A new Flight with the given values
     */
    Q_INVOKABLE static Flightlog::Flight createFlight(
        const QString& departureICAO,
        const QString& arrivalICAO,
        const QString& date,
        const QString& offBlockTimeStr,
        const QString& startTimeStr,
        const QString& landingTimeStr,
        const QString& onBlockTimeStr,
        const QString& pilotName,
        const QString& aircraftCallsign,
        const QString& comments);

    /*! \brief Manually end the current in-flight recording
     *
     *  Delegates to the active FlightDetector. If the detector is in
     *  InFlight state, it completes the flight entry with the current
     *  UTC time. Does nothing if not in InFlight state.
     */
    Q_INVOKABLE void endFlight();

    /*! \brief Find the last arrival ICAO for a given aircraft
     *
     *  Searches the flight log for the most recent flight with the
     *  given aircraft callsign and returns its arrival ICAO code.
     *  Returns an empty string if no matching flight is found.
     *
     *  @param aircraftCallsign The callsign to search for
     *  @returns Arrival ICAO of the most recent matching flight
     */
    Q_INVOKABLE QString lastArrivalICAO(const QString& aircraftCallsign) const;

    /*! \brief Find the nearest airfield within 5 km
     *
     *  Returns the closest airfield (type "AD") to the given position,
     *  provided it is within 5 km. Returns an invalid Waypoint if none found.
     *  If no position is given (or an invalid one), the last valid coordinate
     *  from PositionProvider is used.
     *
     *  @param position The geographic position to search near (default: current GPS position)
     *  @returns The nearest airfield, or an invalid Waypoint
     */
    Q_INVOKABLE static GeoMaps::Waypoint nearestAirfield(const QGeoCoordinate& position = {});

signals:
    /*! \brief Notifier signal */
    void flightsChanged();

    /*! \brief Notifier signal */
    void detectionStateChanged();

    /*! \brief Emitted when a takeoff is detected
     *
     *  @param time The takeoff time as a human-readable UTC string (HH:mm)
     */
    void takeoffDetected(const QString& time);

    /*! \brief Emitted when a landing is detected
     *
     *  @param time The landing time as a human-readable UTC string (HH:mm)
     */
    void landingDetected(const QString& time);

private slots:
    // Process position updates — delegates to the active FlightDetector
    void onPositionUpdated();

    // Handle detection state change — forwards signal
    void onDetectionStateChanged();

    // Handle auto-detection setting change — manages Android foreground service
    void onAutoFlightDetectionChanged();

    // Handle takeoff detected by the FlightDetector
    void onTakeoffDetected(const Flightlog::Flight& flight, const QString& timeStr);

    // Handle landing detected by the FlightDetector
    void onLandingDetected(const QString& arrivalICAO,
                           const QGeoCoordinate& arrivalCoordinate,
                           const QDateTime& landingTime,
                           const QString& timeStr);

private:
    Q_DISABLE_COPY_MOVE(FlightLog)

    // Install signal connections for the given detector
    void connectDetector(FlightDetector* detector);

    // Persist flights to JSON file
    void save();

    // Load flights from JSON file
    void load();

    // Resolve ICAO codes to coordinates using the GeoMapProvider
    void resolveCoordinates(Flight& flight);

    // Sort m_flights by startTime, most recent first
    void sortFlights();

    // Helper to parse a date+time string to QDateTime
    static auto parseDateTime(const QString& date, const QString& timeStr) -> QDateTime;

    QList<Flight> m_flights;

    // The active flight detector (owned by this object)
    FlightDetector* m_detector {nullptr};

#ifdef Q_OS_ANDROID
    // Tracks whether the Android foreground service is running
    bool m_foregroundServiceRunning {false};
#endif

    // Persistence
    const QString m_fileName {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/flightlog.json"_s};
};

} // namespace Flightlog
