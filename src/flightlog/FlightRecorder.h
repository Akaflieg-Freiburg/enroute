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

#include <QStandardPaths>

#include "flightlog/Flight.h"
#include "flightlog/FlightDetector.h"
#include "flightlog/TrackPoint.h"
#include "positioning/PositionInfo.h"
#include "units/Distance.h"

using namespace Qt::Literals::StringLiterals;

namespace Flightlog {

/*! \brief GPS track recorder and IGC file manager
 *
 *  This class handles recording GPS track points during a flight,
 *  writing/reading IGC files, and exporting tracks via the platform
 *  share dialog. It applies distance and time filters to keep the
 *  number of recorded points reasonable.
 *
 *  Recording is driven by calling processPositionUpdate() with the
 *  current detection state on every position update. Track points are
 *  recorded during TakeoffPhase, InFlight, and LandingPhase states.
 *
 *  - **TakeoffPhase → abort (Idle)**: Track is discarded on next cycle.
 *  - **LandingPhase → Idle**: Recording ends. Track saving is handled
 *    externally by FlightLog::onLandingDetected.
 *  - **LandingPhase → InFlight**: Touch-and-go — same as above, the
 *    detector transitions through Idle first.
 */
class FlightRecorder : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FlightRecorder(QObject* parent = nullptr);

    /*! \brief Standard destructor */
    ~FlightRecorder() override = default;

    /*! \brief Process a position update with the current detection state
     *
     *  Call this on every position update when auto-detection is enabled.
     *  The recorder decides whether and where to record based on the
     *  detection state and internal state transitions.
     *
     *  @param state Current detection state from the FlightDetector
     *  @param info The current position info
     *  @param pressureAltitude Pressure altitude, or an invalid Distance if unknown
     */
    void processPositionUpdate(FlightDetector::DetectionState state,
                               const Positioning::PositionInfo& info,
                               Units::Distance pressureAltitude = {});

    /*! \brief Save the recorded track to an IGC file
     *
     *  Writes the recorder's in-memory track as an IGC file in the tracks
     *  directory, using metadata from the flight. Sets the trackFile
     *  property on the flight.
     *
     *  @param flight The flight whose metadata to use for the IGC headers
     */
    bool saveTrack(Flight& flight);

    /*! \brief Clear the in-memory track data
     *
     *  Frees the recorded track points from RAM. Call after saveTrack()
     *  and after caching the geo path for display.
     */
    void clearTrack();

    /*! \brief Load a track's coordinates from an IGC file
     *
     *  Reads the IGC file referenced by the flight's trackFile
     *  property and returns the coordinates for map display.
     *
     *  @param flight The flight whose track to load
     *  @returns List of coordinates from the track, or empty if not available
     */
    [[nodiscard]] auto loadTrackPath(const Flight& flight) const -> QList<QGeoCoordinate>;

    /*! \brief Export a flight's track via the platform share dialog
     *
     *  Reads the IGC file and returns its raw content.
     *
     *  @param flight The flight whose track to export
     *  @returns IGC file content, or empty if no track file or file not readable
     */
    QByteArray exportToIGC(const Flight& flight);

    /*! \brief Delete the track file from disk
     *
     *  Removes the IGC file referenced by the flight's trackFile
     *  property and clears the trackFile.
     *  Does nothing if no trackFile is set.
     *
     *  @param flight The flight whose track to delete
     */
    void removeTrack(Flight& flight);

    /*! \brief Get the current track as a list of coordinates
     *
     *  Returns the coordinates of all recorded track points.
     *  Suitable for binding to a MapPolyline in QML.
     *
     *  @returns List of coordinates from the current track
     */
    [[nodiscard]] auto trackGeoPath() const -> QList<QGeoCoordinate>;

    /*! \brief Generate IGC file content from flight metadata and track points
     *
     *  Creates a complete IGC file with A-record, H-records (date,
     *  pilot, aircraft, recorder type), and B-records for each
     *  track point.
     *
     *  @param flight The flight whose metadata to use for headers
     *  @param track The track points to write as B-records
     *  @returns IGC file content, or empty if no track
     */
    static auto toIGC(const Flight& flight, const QList<TrackPoint>& track) -> QByteArray;

    /*! \brief Parse track points from IGC file content
     *
     *  Reads B-records and extracts position, altitude, and timestamp
     *  for each fix. Parses H-record date if no date is provided.
     *
     *  @param igcData The IGC file content
     *  @param date The flight date (optional, parsed from H-record if empty)
     *  @returns List of parsed track points
     */
    static auto trackFromIGC(const QByteArray& igcData, const QDate& date = {}) -> QList<TrackPoint>;

signals:
    /*! \brief Emitted when a new track point has been recorded
     *
     *  Use this to update the live track display on the map.
     */
    void trackGeoPathChanged();

private:
    Q_DISABLE_COPY_MOVE(FlightRecorder)

    // Build a TrackPoint from position info and optional pressure altitude
    static auto makeTrackPoint(const Positioning::PositionInfo& info, Units::Distance pressureAltitude) -> TrackPoint;

    // Check if a point should be recorded based on distance/time thresholds
    [[nodiscard]] auto shouldRecord(const Positioning::PositionInfo& info) const -> bool;

    // Record a point if thresholds are met
    void recordFiltered(const Positioning::PositionInfo& info, Units::Distance pressureAltitude);

    // Last recorded point for distance/time filtering
    QGeoCoordinate m_lastCoordinate;
    QDateTime m_lastTimestamp;

    // Previous detection state for transition detection
    FlightDetector::DetectionState m_previousState {FlightDetector::Idle};

    // Accumulated track points
    QList<TrackPoint> m_track;

    // Recording thresholds
    static constexpr auto minDistance = Units::Distance::fromM(300.0);
    static constexpr auto minTimeInterval = Units::Timespan::fromS(10);
    static constexpr int maxTrackPoints = 100'000; ///< Hard cap on in-memory track length

    // Tracks directory
    const QString m_trackDir {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/tracks"_s};
};

} // namespace Flightlog
