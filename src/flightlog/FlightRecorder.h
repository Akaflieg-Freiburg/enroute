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
#include <QTimer>

#include "flightlog/Flight.h"
#include "flightlog/FlightDetector.h"
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
 *  current detection state on every position update. The recorder
 *  manages its own internal state machine:
 *  - **TakeoffPhase**: Points are buffered internally.
 *  - **InFlight**: Points are accumulated (preflight buffer is prepended
 *    on transition from TakeoffPhase).
 *  - **Landing detected → post-landing**: Recording continues for 20s
 *    to capture the rollout, then recordingFinished() is emitted.
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

    /*! \brief Take the accumulated track points
     *
     *  Returns the recorded track and clears the internal buffer.
     *  Call this after recordingFinished() to get the complete track.
     *
     *  @returns The recorded track points
     */
    auto takeTrack() -> QList<TrackPoint>;

    /*! \brief Save in-memory track points to an IGC file
     *
     *  Writes the flight's track as an IGC file in the tracks
     *  directory. Sets the trackFile property on the flight and
     *  clears the in-memory track data.
     *
     *  @param flight The flight whose track to save
     */
    void saveTrack(Flight& flight);

    /*! \brief Load track points from an IGC file into a flight
     *
     *  Reads the IGC file referenced by the flight's trackFile
     *  property and populates the in-memory track data.
     *  Does nothing if no trackFile is set or track is already loaded.
     *
     *  @param flight The flight to load the track into
     */
    void loadTrack(Flight& flight) const;

    /*! \brief Export a flight's track via the platform share dialog
     *
     *  Reads the IGC file and shares it using the platform's native
     *  share mechanism (Android intent, desktop file dialog, etc.).
     *
     *  @param flight The flight whose track to export
     */
    void exportToIGC(const Flight& flight);

    /*! \brief Delete the track file from disk and clear track data
     *
     *  Removes the IGC file referenced by the flight's trackFile
     *  property and clears the trackFile and in-memory track data.
     *  Does nothing if no trackFile is set.
     *
     *  @param flight The flight whose track to delete
     */
    void removeTrack(Flight& flight);

    /*! \brief Generate IGC file content from a flight's track
     *
     *  Creates a complete IGC file with A-record, H-records (date,
     *  pilot, aircraft, recorder type), and B-records for each
     *  track point.
     *
     *  @param flight The flight to generate IGC content for
     *  @returns IGC file content, or empty if no track
     */
    static auto toIGC(const Flight& flight) -> QByteArray;

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
    /*! \brief Emitted when the post-landing recording period has ended
     *
     *  The FlightLog should call takeTrack() to retrieve the complete
     *  track, set it on the flight, save to IGC, and persist.
     */
    void recordingFinished();

private:
    Q_DISABLE_COPY_MOVE(FlightRecorder)

    // Build a TrackPoint from position info and optional pressure altitude
    static auto makeTrackPoint(const Positioning::PositionInfo& info, Units::Distance pressureAltitude) -> TrackPoint;

    // Check if a point should be recorded based on distance/time thresholds
    [[nodiscard]] auto shouldRecord(const Positioning::PositionInfo& info) const -> bool;

    // Record a point to the given list if thresholds are met
    void recordFiltered(const Positioning::PositionInfo& info, Units::Distance pressureAltitude, QList<TrackPoint>& target);

    // Last recorded point for distance/time filtering
    QGeoCoordinate m_lastCoordinate;
    QDateTime m_lastTimestamp;

    // Previous detection state for transition detection
    FlightDetector::DetectionState m_previousState {FlightDetector::Idle};

    // Pre-flight buffer for TakeoffPhase points
    QList<TrackPoint> m_preflightBuffer;

    // Accumulated track points (InFlight + post-landing)
    QList<TrackPoint> m_track;

    // Post-landing timer (20 seconds after landing detection)
    QTimer m_postLandingTimer;

    // Recording thresholds
    static constexpr double minDistanceM = 300.0;
    static constexpr int minIntervalSecs = 10;
    static constexpr int postLandingDurationMs = 20000;

    // Tracks directory
    const QString m_trackDir {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/tracks"_s};
};

} // namespace Flightlog
