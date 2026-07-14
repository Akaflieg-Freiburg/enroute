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

#include <QDateTime>
#include <QGeoCoordinate>
#include <QJsonObject>
#include <QQmlEngine>

#include "units/Distance.h"

namespace Flightlog {

/*! \brief A single flight log entry
 *
 *  This class represents a single flight with departure/arrival airports,
 *  times, and optional metadata. It can compute the direct distance between
 *  departure and arrival, and the flight duration.
 */

class Flight
{
    Q_GADGET
    QML_VALUE_TYPE(flight)

    /*! \brief ICAO code of the departure airport */
    Q_PROPERTY(QString departureICAO READ departureICAO WRITE setDepartureICAO)

    /*! \brief ICAO code of the arrival airport */
    Q_PROPERTY(QString arrivalICAO READ arrivalICAO WRITE setArrivalICAO)

    /*! \brief Off-block time in UTC (optional) */
    Q_PROPERTY(QDateTime offBlockTime READ offBlockTime WRITE setOffBlockTime)

    /*! \brief Start (takeoff) time in UTC */
    Q_PROPERTY(QDateTime startTime READ startTime WRITE setStartTime)

    /*! \brief Landing time in UTC */
    Q_PROPERTY(QDateTime landingTime READ landingTime WRITE setLandingTime)

    /*! \brief On-block time in UTC (optional) */
    Q_PROPERTY(QDateTime onBlockTime READ onBlockTime WRITE setOnBlockTime)

    /*! \brief Pilot name (optional) */
    Q_PROPERTY(QString pilotName READ pilotName WRITE setPilotName)

    /*! \brief Aircraft callsign (e.g. D-KEBE, optional) */
    Q_PROPERTY(QString aircraftCallsign READ aircraftCallsign WRITE setAircraftCallsign)

    /*! \brief Free-text comments (optional) */
    Q_PROPERTY(QString comments READ comments WRITE setComments)

    /*! \brief Number of landings (touch-and-goes + final landing) */
    Q_PROPERTY(int landingCount READ landingCount WRITE setLandingCount)

    /*! \brief Whether this flight has a recorded track */
    Q_PROPERTY(bool hasTrack READ hasTrack)

    /*! \brief Filename of the track file (relative to tracks directory) */
    Q_PROPERTY(QString trackFile READ trackFile)

public:
    /*! \brief Default constructor */
    Flight() = default;

    //
    // Getters
    //

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property departureICAO
     */
    [[nodiscard]] auto departureICAO() const -> QString { return m_departureICAO; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property arrivalICAO
     */
    [[nodiscard]] auto arrivalICAO() const -> QString { return m_arrivalICAO; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property offBlockTime
     */
    [[nodiscard]] auto offBlockTime() const -> QDateTime { return m_offBlockTime; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property startTime
     */
    [[nodiscard]] auto startTime() const -> QDateTime { return m_startTime; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property landingTime
     */
    [[nodiscard]] auto landingTime() const -> QDateTime { return m_landingTime; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property onBlockTime
     */
    [[nodiscard]] auto onBlockTime() const -> QDateTime { return m_onBlockTime; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property pilotName
     */
    [[nodiscard]] auto pilotName() const -> QString { return m_pilotName; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property aircraftCallsign
     */
    [[nodiscard]] auto aircraftCallsign() const -> QString { return m_aircraftCallsign; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property comments
     */
    [[nodiscard]] auto comments() const -> QString { return m_comments; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property landingCount
     */
    [[nodiscard]] auto landingCount() const -> int { return m_landingCount; }

    /*! \brief Coordinate of the departure airport
     *
     *  @returns The departure airport coordinate, or an invalid coordinate if unknown
     */
    [[nodiscard]] auto departureCoordinate() const -> QGeoCoordinate { return m_departureCoordinate; }

    /*! \brief Coordinate of the arrival airport
     *
     *  @returns The arrival airport coordinate, or an invalid coordinate if unknown
     */
    [[nodiscard]] auto arrivalCoordinate() const -> QGeoCoordinate { return m_arrivalCoordinate; }

    /*! \brief Check if this flight has a recorded GPS track
     *
     *  @returns True if a track file is associated with this flight
     */
    [[nodiscard]] auto hasTrack() const -> bool { return !m_trackFile.isEmpty(); }

    /*! \brief Get the track file name
     *
     *  @returns The track file name (relative to tracks directory), or empty
     */
    [[nodiscard]] auto trackFile() const -> QString { return m_trackFile; }

    //
    // Setters
    //

    /*! \brief Setter function for property of the same name
     *
     *  @param icao Property departureICAO
     */
    void setDepartureICAO(const QString& icao) { m_departureICAO = icao; }

    /*! \brief Setter function for property of the same name
     *
     *  @param icao Property arrivalICAO
     */
    void setArrivalICAO(const QString& icao) { m_arrivalICAO = icao; }

    /*! \brief Setter function for property of the same name
     *
     *  @param dt Property offBlockTime
     */
    void setOffBlockTime(const QDateTime& dt) { m_offBlockTime = dt; }

    /*! \brief Setter function for property of the same name
     *
     *  @param dt Property startTime
     */
    void setStartTime(const QDateTime& dt) { m_startTime = dt; }

    /*! \brief Setter function for property of the same name
     *
     *  @param dt Property landingTime
     */
    void setLandingTime(const QDateTime& dt) { m_landingTime = dt; }

    /*! \brief Setter function for property of the same name
     *
     *  @param dt Property onBlockTime
     */
    void setOnBlockTime(const QDateTime& dt) { m_onBlockTime = dt; }

    /*! \brief Setter function for property of the same name
     *
     *  @param name Property pilotName
     */
    void setPilotName(const QString& name) { m_pilotName = name; }

    /*! \brief Setter function for property of the same name
     *
     *  @param callsign Property aircraftCallsign
     */
    void setAircraftCallsign(const QString& callsign) { m_aircraftCallsign = callsign; }

    /*! \brief Setter function for property of the same name
     *
     *  @param text Property comments
     */
    void setComments(const QString& text) { m_comments = text; }

    /*! \brief Setter function for property of the same name
     *
     *  @param count Property landingCount
     */
    void setLandingCount(int count) { m_landingCount = count; }

    /*! \brief Set departure coordinate
     *
     *  @param coord The departure airport coordinate
     */
    void setDepartureCoordinate(const QGeoCoordinate& coord) { m_departureCoordinate = coord; }

    /*! \brief Set arrival coordinate
     *
     *  @param coord The arrival airport coordinate
     */
    void setArrivalCoordinate(const QGeoCoordinate& coord) { m_arrivalCoordinate = coord; }

    /*! \brief Set the track file name
     *
     *  @param fileName The track file name (relative to tracks directory)
     */
    void setTrackFile(const QString& fileName) { m_trackFile = fileName; }

    //
    // Calculated properties
    //

    /*! \brief Direct distance between departure and arrival airports
     *
     *  @returns The distance, or an invalid distance if coordinates are unknown
     */
    [[nodiscard]] Q_INVOKABLE Units::Distance distance() const;

    /*! \brief Flight duration as a human-readable string (H:MM)
     *
     *  @returns The flight time string, or an empty string if times are invalid
     */
    [[nodiscard]] Q_INVOKABLE QString flightTime() const;
    /*! \brief Block time as a formatted string
     *
     *  Returns the block time (off-block to on-block) formatted as h:mm.
     *  Returns an empty string if either time is invalid or if block time is negative.
     *
     *  @returns Block time string, e.g. "1:23", or empty string
     */
    [[nodiscard]] Q_INVOKABLE QString blockTime() const;
    /*! \brief Flight duration in seconds
     *
     *  @returns The duration in seconds, or -1 if times are invalid
     */
    [[nodiscard]] Q_INVOKABLE qint64 flightTimeSeconds() const;

    //
    // Serialization
    //

    /*! \brief Serialize to JSON
     *
     *  @returns A JSON object representing this flight
     */
    [[nodiscard]] auto toJSON() const -> QJsonObject;

    /*! \brief Deserialize from JSON
     *
     *  @param json The JSON object to read from
     *  @returns A Flight constructed from the JSON data
     */
    static auto fromJSON(const QJsonObject& json) -> Flight;

    /*! \brief Comparison operator */
    Q_INVOKABLE bool operator==(const Flightlog::Flight& other) const;

private:
    QString m_departureICAO;
    QString m_arrivalICAO;
    QDateTime m_offBlockTime;
    QDateTime m_startTime;
    QDateTime m_landingTime;
    QDateTime m_onBlockTime;
    QString m_pilotName;
    QString m_aircraftCallsign;
    QString m_comments;
    int m_landingCount {0};
    QGeoCoordinate m_departureCoordinate;
    QGeoCoordinate m_arrivalCoordinate;
    QString m_trackFile;
};

} // namespace Flightlog
