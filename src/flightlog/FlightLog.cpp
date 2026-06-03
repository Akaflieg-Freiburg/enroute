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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/Waypoint.h"
#include "flightlog/AirplaneFlightDetector.h"
#include "flightlog/FlightLog.h"
#include "positioning/PositionProvider.h"

using namespace Qt::Literals::StringLiterals;


//
// Constructors and destructors
//

Flightlog::FlightLog::FlightLog(QObject *parent) : GlobalObject(parent)
{
    load();

    // Create the default detector (airplane mode)
    m_detector = new AirplaneFlightDetector(this);
    connectDetector(m_detector);

    // Forward live track updates to the map when no saved track is selected
    connect(&m_recorder, &FlightRecorder::trackGeoPathChanged, this, [this]() {
        if (m_displayedTrackFile.isEmpty()) {
            emit displayedTrackPathChanged();
        }
    });
}


void Flightlog::FlightLog::deferredInitialization()
{
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged,
            this, &Flightlog::FlightLog::onPositionUpdated);

    connect(GlobalObject::globalSettings(), &GlobalSettings::autoFlightDetectionChanged,
            this, &Flightlog::FlightLog::onAutoFlightDetectionChanged);

    // If auto-detection was already enabled (persisted setting), start the service now
    if (GlobalObject::globalSettings()->autoFlightDetection()) {
        onAutoFlightDetectionChanged();
    }
}


//
// Properties
//

auto Flightlog::FlightLog::displayedTrackIndex() const -> int
{
    if (m_displayedTrackFile.isEmpty()) {
        return -1;
    }
    for (int i = 0; i < m_flights.size(); ++i) {
        if (m_flights[i].trackFile() == m_displayedTrackFile) {
            return i;
        }
    }
    return -1;
}


auto Flightlog::FlightLog::detectionState() const -> FlightDetector::DetectionState
{
    if (m_detector == nullptr) {
        return FlightDetector::Idle;
    }
    return m_detector->detectionState();
}


//
// CRUD operations
//

void Flightlog::FlightLog::addFlight(const Flightlog::Flight& flight)
{
    auto f = flight;
    resolveCoordinates(f);
    m_flights.prepend(f);
    sortFlights();
    save();
    emit flightsChanged();
}

void Flightlog::FlightLog::setTrackRecording(bool enabled)
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        settings->setTrackRecording(enabled);
    }
    emit trackRecordingChanged();
}

bool Flightlog::FlightLog::trackRecording() const
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        return settings->trackRecording();
    }
    return true; // default
}

void Flightlog::FlightLog::setShowCurrentFlightTrace(bool enabled)
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        settings->setShowCurrentFlightTrace(enabled);
    }
    emit showCurrentFlightTraceChanged();
    emit displayedTrackPathChanged();
}

bool Flightlog::FlightLog::showCurrentFlightTrace() const
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        return settings->showCurrentFlightTrace();
    }
    return true; // default
}


void Flightlog::FlightLog::removeFlight(int index)
{
    if (index < 0 || index >= m_flights.size()) {
        return;
    }

    // If this flight's track is currently displayed, hide it first
    if (!m_displayedTrackFile.isEmpty()
        && m_flights[index].trackFile() == m_displayedTrackFile) {
        hideTrack();
    }

    // If the in-progress flight is being removed while detection is active,
    // reset the detector so it doesn't later complete a landing onto wrong flight
    if (index == m_currentFlightIndex && m_detector != nullptr
        && m_detector->detectionState() != FlightDetector::Idle) {
        m_currentFlightIndex = -1;
        m_detector->resetDetection();
    } else if (index < m_currentFlightIndex) {
        // If removing a flight before the current one, adjust the index
        --m_currentFlightIndex;
    } else if (index == m_currentFlightIndex) {
        // Removing the current flight, reset tracking
        m_currentFlightIndex = -1;
    }

    m_flights.removeAt(index);
    save();
    emit flightsChanged();
}


void Flightlog::FlightLog::updateFlight(int index, const Flightlog::Flight& flight)
{
    if (index < 0 || index >= m_flights.size()) {
        return;
    }

    auto f = flight;
    resolveCoordinates(f);

    // Preserve old coordinates if new resolution failed but ICAO codes match
    const auto& old = m_flights[index];
    if (!f.departureCoordinate().isValid() && old.departureCoordinate().isValid()
        && f.departureICAO() == old.departureICAO()) {
        f.setDepartureCoordinate(old.departureCoordinate());
    }
    if (!f.arrivalCoordinate().isValid() && old.arrivalCoordinate().isValid()
        && f.arrivalICAO() == old.arrivalICAO()) {
        f.setArrivalCoordinate(old.arrivalCoordinate());
    }

    m_flights[index] = f;
    sortFlights();
    save();
    emit flightsChanged();
}


auto Flightlog::FlightLog::createFlight(
    const QString& departureICAO,
    const QString& arrivalICAO,
    const QString& date,
    const QString& offBlockTimeStr,
    const QString& startTimeStr,
    const QString& landingTimeStr,
    const QString& onBlockTimeStr,
    const QString& pilotName,
    const QString& aircraftCallsign,
    const QString& comments) -> Flightlog::Flight
{
    Flight f;
    f.setDepartureICAO(departureICAO);
    f.setArrivalICAO(arrivalICAO);
    f.setOffBlockTime(parseDateTime(date, offBlockTimeStr));
    f.setStartTime(parseDateTime(date, startTimeStr));
    f.setLandingTime(parseDateTime(date, landingTimeStr));
    f.setOnBlockTime(parseDateTime(date, onBlockTimeStr));
    f.setPilotName(pilotName);
    f.setAircraftCallsign(aircraftCallsign);
    f.setComments(comments);
    f.setLandingCount(1);
    return f;
}


void Flightlog::FlightLog::endFlight()
{
    if (m_detector != nullptr) {
        m_detector->endFlight();
    }
}


auto Flightlog::FlightLog::lastArrivalICAO(const QString& aircraftCallsign) const -> QString
{
    for (const auto& flight : m_flights) {
        if (flight.aircraftCallsign() == aircraftCallsign && !flight.arrivalICAO().isEmpty()) {
            return flight.arrivalICAO();
        }
    }
    return {};
}


auto Flightlog::FlightLog::nearestAirfield(const QGeoCoordinate& position) -> GeoMaps::Waypoint
{
    auto coord = position;
    if (!coord.isValid()) {
        auto* positionProvider = GlobalObject::positionProvider();
        if (positionProvider == nullptr) {
            return {};
        }
        coord = positionProvider->lastValidCoordinate();
        if (!coord.isValid()) {
            return {};
        }
    }

    auto* geoMapProvider = GlobalObject::geoMapProvider();
    if (geoMapProvider == nullptr) {
        return {};
    }

    auto nearby = geoMapProvider->nearbyWaypoints(coord, u"AD"_s);
    if (nearby.isEmpty()) {
        return {};
    }

    auto closest = nearby.first();
    if (coord.distanceTo(closest.coordinate()) > 5000.0) {
        return {};
    }

    return closest;
}


void Flightlog::FlightLog::exportToIGC(int index)
{
    if (index < 0 || index >= m_flights.size()) {
        return;
    }
    m_recorder.exportToIGC(m_flights.at(index));
}


void Flightlog::FlightLog::removeTrack(int index)
{
    if (index < 0 || index >= m_flights.size()) {
        return;
    }

    // If this track is currently displayed, hide it first
    if (!m_displayedTrackFile.isEmpty()
        && m_flights[index].trackFile() == m_displayedTrackFile) {
        hideTrack();
    }

    m_recorder.removeTrack(m_flights[index]);

    save();
    emit flightsChanged();
}


auto Flightlog::FlightLog::displayedTrackPath() const -> QList<QGeoCoordinate>
{
    // If a saved track is selected, return its cached path
    if (!m_displayedTrackFile.isEmpty()) {
        return m_displayedTrackPath;
    }

    // Otherwise return the live recording track when enabled
    if (!showCurrentFlightTrace()) {
        return {};
    }

    // Live recording track (empty if not recording)
    return m_recorder.trackGeoPath();
}


void Flightlog::FlightLog::showTrack(int index)
{
    if (index < 0 || index >= m_flights.size()) {
        return;
    }
    if (!m_flights.at(index).hasTrack()) {
        return;
    }

    // Load track coordinates directly from IGC file
    m_displayedTrackPath = m_recorder.loadTrackPath(m_flights[index]);
    m_displayedTrackFile = m_flights[index].trackFile();
    emit displayedTrackPathChanged();
}


void Flightlog::FlightLog::hideTrack()
{
    m_displayedTrackFile.clear();
    m_displayedTrackPath.clear();
    emit displayedTrackPathChanged();
}


//
// Coordinate resolution
//

void Flightlog::FlightLog::resolveCoordinates(Flight& flight)
{
    auto* geoMapProvider = GlobalObject::geoMapProvider();
    if (geoMapProvider == nullptr) {
        return;
    }

    if (!flight.departureICAO().isEmpty()) {
        auto wp = geoMapProvider->findByID(flight.departureICAO());
        if (wp.coordinate().isValid()) {
            flight.setDepartureCoordinate(wp.coordinate());
        }
    }

    if (!flight.arrivalICAO().isEmpty()) {
        auto wp = geoMapProvider->findByID(flight.arrivalICAO());
        if (wp.coordinate().isValid()) {
            flight.setArrivalCoordinate(wp.coordinate());
        }
    }
}


void Flightlog::FlightLog::sortFlights()
{
    // Remember the current flight before sorting (if any)
    Flight currentFlight;
    const bool hasCurrentFlight = (m_currentFlightIndex >= 0 && m_currentFlightIndex < m_flights.size());
    if (hasCurrentFlight) {
        currentFlight = m_flights[m_currentFlightIndex];
    }

    // Sort by start time, most recent first
    std::sort(m_flights.begin(), m_flights.end(),
              [](const Flight& a, const Flight& b) {
                  return a.startTime() > b.startTime();
              });

    // Find the current flight's new position after sorting
    if (hasCurrentFlight) {
        for (int i = 0; i < m_flights.size(); ++i) {
            // Match by both startTime and trackFile to uniquely identify the flight
            if (m_flights[i].startTime() == currentFlight.startTime() &&
                m_flights[i].trackFile() == currentFlight.trackFile()) {
                m_currentFlightIndex = i;
                return;
            }
        }
        // If we couldn't find it (shouldn't happen), reset the index
        m_currentFlightIndex = -1;
    }
}


//
// Persistence
//

void Flightlog::FlightLog::save()
{
    QJsonArray array;
    for (const auto& flight : m_flights) {
        array.append(flight.toJSON());
    }

    QJsonObject root;
    root[u"content"_s] = u"flightLog"_s;
    root[u"version"_s] = 1;
    root[u"exportDate"_s] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    root[u"flights"_s] = array;

    const QJsonDocument doc(root);
    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
    }
}


void Flightlog::FlightLog::load()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }

    const auto root = doc.object();
    if (root.value(u"content"_s).toString() != u"flightLog"_s) {
        return;
    }

    // version field is available for future migration logic
    // const auto version = root.value(u"version"_s).toInt();

    m_flights.clear();
    const auto array = root.value(u"flights"_s).toArray();
    for (const auto& val : array) {
        if (val.isObject()) {
            m_flights.append(Flight::fromJSON(val.toObject()));
        }
    }
    sortFlights();
}


auto Flightlog::FlightLog::parseDateTime(const QString& date, const QString& timeStr) -> QDateTime
{
    if (date.isEmpty() || timeStr.isEmpty()) {
        return {};
    }
    return QDateTime::fromString(date + u"T"_s + timeStr + u":00Z"_s, Qt::ISODate);
}


//
// Detector signal handlers
//

void Flightlog::FlightLog::connectDetector(FlightDetector* detector)
{
    if (detector == nullptr) {
        return;
    }

    connect(detector, &FlightDetector::detectionStateChanged,
            this, &FlightLog::onDetectionStateChanged);
    connect(detector, &FlightDetector::takeoffDetected,
            this, &FlightLog::onTakeoffDetected);
    connect(detector, &FlightDetector::landingDetected,
            this, &FlightLog::onLandingDetected);
}


void Flightlog::FlightLog::onDetectionStateChanged()
{
    if (m_detector->detectionState() == FlightDetector::InFlight) {
        m_displayedTrackFile.clear();
        m_displayedTrackPath.clear();
        emit displayedTrackPathChanged();
    }
    emit detectionStateChanged();
}


void Flightlog::FlightLog::onTakeoffDetected(const Flightlog::Flight& flight, const QString& timeStr)
{
    // Add the preliminary flight entry so it appears in the list immediately
    addFlight(flight);

    // Track that a flight is now in progress (always at index 0 due to prepending in addFlight)
    m_currentFlightIndex = 0;

#ifdef Q_OS_ANDROID
    // Post a notification with sound so the pilot knows takeoff was detected,
    // even if the app is in the background.
    auto title = tr("Takeoff Detected");
    auto message = tr("Departed %1 at %2 UTC").arg(
        flight.departureICAO().isEmpty() ? tr("unknown") : flight.departureICAO(),
        timeStr);
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject::callStaticMethod<void>(
        "de/akaflieg_freiburg/enroute/FlightLogService",
        "notifyEvent",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
        context.object(),
        QJniObject::fromString(title).object<jstring>(),
        QJniObject::fromString(message).object<jstring>());
#endif

    emit takeoffDetected(timeStr);
}


void Flightlog::FlightLog::onLandingDetected(const QString& arrivalICAO,
                                                const QGeoCoordinate& arrivalCoordinate,
                                                const QDateTime& landingTime,
                                                int landingCount,
                                                const QString& timeStr)
{
    // Complete the in-progress flight using m_currentFlightIndex
    if (m_currentFlightIndex >= 0 && m_currentFlightIndex < m_flights.size()) {
        auto& flight = m_flights[m_currentFlightIndex];
        if (!arrivalICAO.isEmpty()) {
            flight.setArrivalICAO(arrivalICAO);
        }
        if (arrivalCoordinate.isValid()) {
            flight.setArrivalCoordinate(arrivalCoordinate);
        }
        flight.setLandingTime(landingTime);
        flight.setLandingCount(landingCount);

        // Save the track from the recorder to an IGC file
        if (trackRecording()) {
            m_recorder.saveTrack(flight);

            // Cache the geo path for map display, then free recorder RAM
            m_displayedTrackPath = m_recorder.trackGeoPath();
            m_displayedTrackFile = m_displayedTrackPath.isEmpty() ? QString{} : flight.trackFile();
            m_recorder.clearTrack();
        }

        save();
        emit flightsChanged();
        emit displayedTrackPathChanged();
    }

    // Flight recording is complete
    m_currentFlightIndex = -1;

    emit landingDetected(timeStr);

#ifdef Q_OS_ANDROID
    // Post a notification with sound so the pilot knows landing was detected.
    auto title = tr("Landing Detected");
    auto message = tr("Landed %1 at %2 UTC").arg(
        arrivalICAO.isEmpty() ? tr("unknown") : arrivalICAO,
        timeStr);
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject::callStaticMethod<void>(
        "de/akaflieg_freiburg/enroute/FlightLogService",
        "notifyEvent",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
        context.object(),
        QJniObject::fromString(title).object<jstring>(),
        QJniObject::fromString(message).object<jstring>());
#endif
}


//
// Position update delegation
//

void Flightlog::FlightLog::onPositionUpdated()
{
    // Only run auto-detection if the setting is enabled
    if (!GlobalObject::globalSettings()->autoFlightDetection()) {
        // If detection was in progress when disabled, reset state
        if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
            m_detector->resetDetection();
        }
        return;
    }

    if (m_detector == nullptr) {
        return;
    }

    auto info = GlobalObject::positionProvider()->positionInfo();
    m_detector->processPositionUpdate(info);

    // Forward to recorder — it manages preflight/inflight/postlanding internally
    if (trackRecording()) {
        auto pressAlt = GlobalObject::positionProvider()->pressureAltitude();
        m_recorder.processPositionUpdate(m_detector->detectionState(), info, pressAlt);
    }
}


void Flightlog::FlightLog::onAutoFlightDetectionChanged()
{
#ifdef Q_OS_ANDROID
    bool enabled = GlobalObject::globalSettings()->autoFlightDetection();

    if (enabled && !m_foregroundServiceRunning) {
        QJniObject context = QNativeInterface::QAndroidApplication::context();
        QJniObject::callStaticMethod<void>(
            "de/akaflieg_freiburg/enroute/FlightLogService",
            "start",
            "(Landroid/content/Context;)V",
            context.object());
        m_foregroundServiceRunning = true;
    } else if (!enabled && m_foregroundServiceRunning) {
        QJniObject context = QNativeInterface::QAndroidApplication::context();
        QJniObject::callStaticMethod<void>(
            "de/akaflieg_freiburg/enroute/FlightLogService",
            "stop",
            "(Landroid/content/Context;)V",
            context.object());
        m_foregroundServiceRunning = false;
    }
#endif
}
