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

#include <QDir>
#include <QFile>

#include "flightlog/FlightRecorder.h"

using namespace Qt::Literals::StringLiterals;


Flightlog::FlightRecorder::FlightRecorder(QObject* parent)
    : QObject(parent)
{
}


auto Flightlog::FlightRecorder::makeTrackPoint(const Positioning::PositionInfo& info, Units::Distance pressureAltitude) -> TrackPoint
{
    TrackPoint pt;
    auto coord = info.coordinate();
    auto gpsAlt = info.trueAltitudeAMSL();
    if (gpsAlt.isFinite()) {
        coord.setAltitude(gpsAlt.toM());
    }
    pt.coordinate = coord;
    pt.timestamp = info.timestamp();
    if (pressureAltitude.isFinite()) {
        pt.pressureAltitude = pressureAltitude.toM();
    }
    return pt;
}


auto Flightlog::FlightRecorder::shouldRecord(const Positioning::PositionInfo& info) const -> bool
{
    if (!info.isValid()) {
        return false;
    }

    // Record if enough time has passed
    if (!m_lastTimestamp.isValid()
        || m_lastTimestamp.secsTo(info.timestamp()) >= minIntervalSecs) {
        return true;
    }

    // Record if we've moved far enough
    if (m_lastCoordinate.isValid()
        && m_lastCoordinate.distanceTo(info.coordinate()) >= minDistanceM) {
        return true;
    }

    return false;
}


void Flightlog::FlightRecorder::recordFiltered(const Positioning::PositionInfo& info, Units::Distance pressureAltitude)
{
    if (!shouldRecord(info)) {
        return;
    }

    m_track.append(makeTrackPoint(info, pressureAltitude));
    m_lastCoordinate = info.coordinate();
    m_lastTimestamp = info.timestamp();
    emit trackGeoPathChanged();
}


void Flightlog::FlightRecorder::processPositionUpdate(FlightDetector::DetectionState state,
                                                       const Positioning::PositionInfo& info,
                                                       Units::Distance pressureAltitude)
{
    // Handle state transitions
    if (state != m_previousState) {
        if (state == FlightDetector::TakeoffPhase) {
            // New detection cycle — clear everything
            m_lastCoordinate = {};
            m_lastTimestamp = {};
            m_track.clear();
        } else if (m_previousState == FlightDetector::LandingPhase) {
            // Landing or touch-and-go — reset for next recording.
            // Track saving is handled by FlightLog::onLandingDetected.
            m_lastCoordinate = {};
            m_lastTimestamp = {};
        }
        m_previousState = state;
    }

    // Record based on current state
    if (state == FlightDetector::TakeoffPhase
        || state == FlightDetector::InFlight
        || state == FlightDetector::LandingPhase) {
        recordFiltered(info, pressureAltitude);
    }
}


void Flightlog::FlightRecorder::clearTrack()
{
    m_track.clear();
    m_track.squeeze();
}


auto Flightlog::FlightRecorder::trackGeoPath() const -> QList<QGeoCoordinate>
{
    QList<QGeoCoordinate> path;
    path.reserve(m_track.size());
    for (const auto& pt : m_track) {
        path.append(pt.coordinate);
    }
    return path;
}


void Flightlog::FlightRecorder::saveTrack(Flight& flight)
{
    if (m_track.isEmpty()) {
        return;
    }

    // Ensure tracks directory exists
    QDir dir;
    dir.mkpath(m_trackDir);

    // Generate filename from start time
    auto dateTimeStr = flight.startTime().isValid()
        ? flight.startTime().toUTC().toString(u"yyyyMMdd_HHmm"_s)
        : QDateTime::currentDateTimeUtc().toString(u"yyyyMMdd_HHmm"_s);
    auto trackFileName = u"track_%1.igc"_s.arg(dateTimeStr);

    auto igcData = toIGC(flight, m_track);
    QFile file(m_trackDir + u"/"_s + trackFileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(igcData);
        file.close();
        flight.setTrackFile(trackFileName);
    }
}


auto Flightlog::FlightRecorder::loadTrackPath(const Flight& flight) const -> QList<QGeoCoordinate>
{
    if (flight.trackFile().isEmpty()) {
        return {};
    }

    QFile file(m_trackDir + u"/"_s + flight.trackFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    auto igcData = file.readAll();
    file.close();

    auto date = flight.startTime().isValid() ? flight.startTime().toUTC().date() : QDate();
    auto trackPoints = trackFromIGC(igcData, date);

    QList<QGeoCoordinate> path;
    path.reserve(trackPoints.size());
    for (const auto& pt : trackPoints) {
        path.append(pt.coordinate);
    }
    return path;
}


auto Flightlog::FlightRecorder::exportToIGC(const Flight& flight) -> QByteArray
{
    if (flight.trackFile().isEmpty()) {
        return {};
    }

    QFile file(m_trackDir + u"/"_s + flight.trackFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}


void Flightlog::FlightRecorder::removeTrack(Flight& flight)
{
    if (flight.trackFile().isEmpty()) {
        return;
    }

    QFile::remove(m_trackDir + u"/"_s + flight.trackFile());
    flight.setTrackFile({});
}


auto Flightlog::FlightRecorder::toIGC(const Flight& flight, const QList<TrackPoint>& track) -> QByteArray
{
    if (track.isEmpty()) {
        return {};
    }

    QByteArray igc;

    // A-record: manufacturer and flight recorder ID
    igc.append("AXXENROUTE\r\n");

    // H-records: header information
    if (flight.startTime().isValid()) {
        auto utc = flight.startTime().toUTC();
        igc.append(QStringLiteral("HFDTE%1%2%3\r\n")
            .arg(utc.date().day(), 2, 10, QChar(u'0'))
            .arg(utc.date().month(), 2, 10, QChar(u'0'))
            .arg(utc.date().year() % 100, 2, 10, QChar(u'0'))
            .toUtf8());
    }

    if (!flight.pilotName().isEmpty()) {
        igc.append(QStringLiteral("HFPLTPILOTINCHARGE:%1\r\n").arg(flight.pilotName()).toUtf8());
    }
    if (!flight.aircraftCallsign().isEmpty()) {
        igc.append(QStringLiteral("HFCIDCOMPETITIONID:%1\r\n").arg(flight.aircraftCallsign()).toUtf8());
    }
    igc.append("HFFTYFRTYPE:Enroute Flight Navigation\r\n");

    // B-records: track fixes
    for (const auto& pt : track) {
        if (!pt.isValid()) {
            continue;
        }

        auto utc = pt.timestamp.toUTC().time();
        auto lat = pt.coordinate.latitude();
        auto lon = pt.coordinate.longitude();

        // Latitude: DDMMmmmN/S
        auto latChar = lat >= 0 ? 'N' : 'S';
        lat = std::abs(lat);
        int latDeg = static_cast<int>(lat);
        int latMin = static_cast<int>((lat - latDeg) * 60000);

        // Longitude: DDDMMmmmE/W
        auto lonChar = lon >= 0 ? 'E' : 'W';
        lon = std::abs(lon);
        int lonDeg = static_cast<int>(lon);
        int lonMin = static_cast<int>((lon - lonDeg) * 60000);

        // Pressure altitude and GPS altitude from coordinate
        int gpsAlt = !qIsNaN(pt.coordinate.altitude()) ? static_cast<int>(std::round(pt.coordinate.altitude())) : 0;
        int pressAlt = std::isfinite(pt.pressureAltitude) ? static_cast<int>(std::round(pt.pressureAltitude)) : 0;

        // B-record: BHHMMSSDDMMmmmNDDDMMmmmEAPPPPPGGGGG
        igc.append(QStringLiteral("B%1%2%3%4%5%6%7%8%9A%10%11\r\n")
            .arg(utc.hour(), 2, 10, QChar(u'0'))
            .arg(utc.minute(), 2, 10, QChar(u'0'))
            .arg(utc.second(), 2, 10, QChar(u'0'))
            .arg(latDeg, 2, 10, QChar(u'0'))
            .arg(latMin, 5, 10, QChar(u'0'))
            .arg(QLatin1Char(latChar))
            .arg(lonDeg, 3, 10, QChar(u'0'))
            .arg(lonMin, 5, 10, QChar(u'0'))
            .arg(QLatin1Char(lonChar))
            .arg(pressAlt, 5, 10, QChar(u'0'))
            .arg(gpsAlt, 5, 10, QChar(u'0'))
            .toUtf8());
    }

    return igc;
}


auto Flightlog::FlightRecorder::trackFromIGC(const QByteArray& igcData, const QDate& date) -> QList<TrackPoint>
{
    QList<TrackPoint> track;

    // Parse H-record date if no date provided
    QDate flightDate = date;

    const auto lines = igcData.split('\n');
    for (const auto& rawLine : lines) {
        auto line = rawLine.trimmed();

        // Parse HFDTE date record: HFDTEddmmyy
        if (!flightDate.isValid() && line.startsWith("HFDTE") && line.size() >= 11) {
            int dayStart = 5;
            // Skip optional "DATE:" or ":"
            if (line.size() >= 15 && line.mid(5, 5) == "DATE:") {
                dayStart = 10;
            }
            if (line.size() >= dayStart + 6) {
                int dd = line.mid(dayStart, 2).toInt();
                int mm = line.mid(dayStart + 2, 2).toInt();
                int yy = line.mid(dayStart + 4, 2).toInt();
                flightDate = QDate(2000 + yy, mm, dd);
            }
        }

        // Parse B-record: BHHMMSSDDMMmmmNDDDMMmmmEAPPPPPGGGGG
        // Minimum length: 1 + 6 + 8 + 9 + 1 + 5 + 5 = 35
        if (line.size() < 35 || line[0] != 'B') {
            continue;
        }

        int hh = line.mid(1, 2).toInt();
        int mm = line.mid(3, 2).toInt();
        int ss = line.mid(5, 2).toInt();

        // Latitude: DDMMmmmN/S
        int latDeg = line.mid(7, 2).toInt();
        int latMin = line.mid(9, 5).toInt(); // in thousandths of minutes
        char latHem = line[14];
        double lat = latDeg + latMin / 60000.0;
        if (latHem == 'S') {
            lat = -lat;
        }

        // Longitude: DDDMMmmmE/W
        int lonDeg = line.mid(15, 3).toInt();
        int lonMin = line.mid(18, 5).toInt();
        char lonHem = line[23];
        double lon = lonDeg + lonMin / 60000.0;
        if (lonHem == 'W') {
            lon = -lon;
        }

        // Validity flag at position 24
        // Pressure altitude at 25-29, GPS altitude at 30-34
        int pressAlt = line.mid(25, 5).toInt();
        int gpsAlt = line.mid(30, 5).toInt();

        TrackPoint pt;
        if (gpsAlt != 0) {
            pt.coordinate = QGeoCoordinate(lat, lon, gpsAlt);
        } else {
            pt.coordinate = QGeoCoordinate(lat, lon);
        }
        if (pressAlt != 0) {
            pt.pressureAltitude = pressAlt;
        }
        if (flightDate.isValid()) {
            pt.timestamp = QDateTime(flightDate, QTime(hh, mm, ss), QTimeZone::UTC);
        }

        if (pt.coordinate.isValid()) {
            track.append(pt);
        }
    }

    return track;
}
