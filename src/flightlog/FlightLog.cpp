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

#include <QDateTime>
#include <QFile>
#include <QGeoPath>
#include <QSet>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

#ifdef Q_OS_IOS
#include "ios/ObjCAdapter.h"
#endif

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/Waypoint.h"
#include "flightlog/AirplaneFlightDetector.h"
#include "flightlog/FlightLog.h"
#include "flightlog/FlightLogExportForeFlight.h"
#include "flightlog/FlightLogExportJSON.h"
#include "flightlog/FlightLogStorage.h"
#include "flightlog/FlightRecorder.h"
#include "positioning/PositionProvider.h"

using namespace Qt::Literals::StringLiterals;

#ifdef Q_OS_ANDROID
namespace {

// Notification IDs — must match FlightLogService.NOTIFICATION_ID_* Java constants.
constexpr jint NOTIFICATION_ID_EVENT  = 1002;
constexpr jint NOTIFICATION_ID_NO_GPS = 1004;

void postAndroidNotification(jint id, const QString& title, const QString& message)
{
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject::callStaticMethod<void>(
        "de/akaflieg_freiburg/enroute/FlightLogService",
        "postNotification",
        "(Landroid/content/Context;ILjava/lang/String;Ljava/lang/String;)V",
        context.object(),
        id,
        QJniObject::fromString(title).object<jstring>(),
        QJniObject::fromString(message).object<jstring>());
}

void cancelAndroidNotification(jint id)
{
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject::callStaticMethod<void>(
        "de/akaflieg_freiburg/enroute/FlightLogService",
        "cancelNotification",
        "(Landroid/content/Context;I)V",
        context.object(),
        id);
}

} // namespace
#endif


//
// Constructors and destructors
//

Flightlog::FlightLog::FlightLog(QObject *parent)
    : GlobalObject(parent)
    , m_recorder(std::make_unique<FlightRecorder>(this))
    , m_storage(std::make_unique<FlightLogStorage>(this))
{
    connect(m_storage.get(), &FlightLogStorage::saveError, this, &FlightLog::saveError);

    load();

    // Create the default detector (airplane mode)
    m_detector = new AirplaneFlightDetector(this);
    connectDetector(m_detector);

    // Forward live track updates to the map when no saved track is selected
    connect(m_recorder.get(), &FlightRecorder::trackGeoPathChanged, this, [this]() {
        if (m_displayedTrackFile.isEmpty()) {
            emit displayedTrackPathChanged();
        }
    });
}


Flightlog::FlightLog::~FlightLog() = default;


void Flightlog::FlightLog::deferredInitialization()
{
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged,
            this, &Flightlog::FlightLog::onPositionUpdated);

    connect(GlobalObject::globalSettings(), &GlobalSettings::autoFlightDetectionChanged,
            this, &Flightlog::FlightLog::onAutoFlightDetectionChanged);

    // Forward GlobalSettings NOTIFY signals so FlightLog property bindings
    // stay consistent regardless of who writes the underlying setting.
    connect(GlobalObject::globalSettings(), &GlobalSettings::trackRecordingChanged,
            this, &Flightlog::FlightLog::trackRecordingChanged);
    connect(GlobalObject::globalSettings(), &GlobalSettings::showCurrentFlightTraceChanged,
            this, [this]() {
                emit showCurrentFlightTraceChanged();
                emit displayedTrackPathChanged();
            });

#ifdef Q_OS_ANDROID
    // After a 30-second grace period, post a notification if auto-detection is
    // on but still no position data (e.g. GPS disabled, no traffic receiver).
    m_noGPSTimer.setInterval(30000);
    m_noGPSTimer.setSingleShot(true);
    connect(&m_noGPSTimer, &QTimer::timeout, this, [this]() {
        if (!GlobalObject::globalSettings()->autoFlightDetection()) {
            return;
        }
        if (GlobalObject::positionProvider()->receivingPositionInfo()) {
            return;
        }
        postAndroidNotification(NOTIFICATION_ID_NO_GPS,
            tr("No Position Data"),
            tr("Automatic flight detection is active but no GPS or traffic receiver data is being received. Enable Location Service."));
    });

    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::receivingPositionInfoChanged,
            this, &Flightlog::FlightLog::onReceivingPositionInfoChanged);
#endif

    // If auto-detection was already enabled (persisted setting), start the service now
    if (GlobalObject::globalSettings()->autoFlightDetection()) {
        onAutoFlightDetectionChanged();
    }
}


//
// Properties
//

auto Flightlog::FlightLog::displayedTrackUuid() const -> QString
{
    if (m_displayedTrackFile.isEmpty()) {
        return {};
    }
    const auto& flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.trackFile() == m_displayedTrackFile;
    });
    if (it == flights.end()) {
        return {};
    }
    return it->uuid().toString(QUuid::WithoutBraces);
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
    auto flights = m_flights.value();
    flights.prepend(f);
    sortFlights(flights);
    m_flights.setValue(std::move(flights));
    m_storage->upsert(f);
}

void Flightlog::FlightLog::setTrackRecording(bool enabled)
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        settings->setTrackRecording(enabled);
    }
    // Signal forwarded via GlobalSettings::trackRecordingChanged connection
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
    // Signal forwarded via GlobalSettings::showCurrentFlightTraceChanged connection
}

bool Flightlog::FlightLog::showCurrentFlightTrace() const
{
    auto* settings = GlobalObject::globalSettings();
    if (settings != nullptr) {
        return settings->showCurrentFlightTrace();
    }
    return true; // default
}


void Flightlog::FlightLog::removeFlight(const QString& uuid)
{
    const auto id = QUuid::fromString(uuid);
    if (id.isNull()) {
        return;
    }
    auto flights = m_flights.value();
    const auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.uuid() == id;
    });
    if (it == flights.end()) {
        return;
    }

    auto& flight = *it;

    // If this flight's track is currently displayed, hide it first
    if (!m_displayedTrackFile.isEmpty()
        && flight.trackFile() == m_displayedTrackFile) {
        hideTrack();
    }

    // If the in-progress flight is being removed, reset tracking
    if (flight.uuid() == m_currentFlightUuid) {
        m_currentFlightUuid = {};
        if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
            m_detector->resetDetection();
        }
        m_recorder->clearTrack();
    }

    m_recorder->removeTrack(flight);
    flights.erase(it);
    m_flights.setValue(std::move(flights));
    m_storage->remove(id);
}


void Flightlog::FlightLog::removeFlights(const QStringList& uuids)
{
    auto flights = m_flights.value();
    QList<QUuid> removedIds;
    for (const QString& uuid : uuids) {
        const auto id = QUuid::fromString(uuid);
        if (id.isNull()) {
            continue;
        }
        auto it = std::ranges::find_if(flights, [&](const Flight& f) {
            return f.uuid() == id;
        });
        if (it == flights.end()) {
            continue;
        }
        if (!m_displayedTrackFile.isEmpty() && it->trackFile() == m_displayedTrackFile) {
            hideTrack();
        }
        if (it->uuid() == m_currentFlightUuid) {
            m_currentFlightUuid = {};
            if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
                m_detector->resetDetection();
            }
            m_recorder->clearTrack();
        }
        m_recorder->removeTrack(*it);
        flights.erase(it);
        removedIds.append(id);
    }
    if (!removedIds.isEmpty()) {
        m_flights.setValue(std::move(flights));
        m_storage->removeMany(removedIds);
    }
}


void Flightlog::FlightLog::clearFlights()
{
    auto flights = m_flights.value();
    if (flights.isEmpty()) {
        return;
    }
    hideTrack();
    if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
        m_detector->resetDetection();
    }
    if (!m_currentFlightUuid.isNull()) {
        m_currentFlightUuid = {};
        m_recorder->clearTrack();
    }
    for (auto& flight : flights) {
        m_recorder->removeTrack(flight);
    }
    m_flights.setValue({});
    m_storage->removeAll();
}



void Flightlog::FlightLog::updateFlight(const QString& uuid, const Flightlog::Flight& flight)
{
    auto targetUuid = QUuid::fromString(uuid);
    auto flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == flights.end()) {
        return;
    }

    // Start from the existing entry so that read-only fields (trackFile,
    // coordinates) are preserved by default.
    auto f = *it;
    const auto& old = *it;

    // Only clear a coordinate when its ICAO code actually changes, so
    // resolveCoordinates() below re-derives it from the new code. If the
    // code is unchanged, the coordinate is left exactly as it was — this is
    // what protects a real GPS-derived coordinate (from automatic detection
    // or the "End Flight" button) from being replaced by an ICAO-based
    // approximation just because some other field was edited.
    if (flight.departureICAO() != old.departureICAO()) {
        f.setDepartureCoordinate({});
    }
    if (flight.arrivalICAO() != old.arrivalICAO()) {
        f.setArrivalCoordinate({});
    }

    f.setDepartureICAO(flight.departureICAO());
    f.setArrivalICAO(flight.arrivalICAO());
    f.setOffBlockTime(flight.offBlockTime());
    f.setStartTime(flight.startTime());
    f.setLandingTime(flight.landingTime());
    f.setOnBlockTime(flight.onBlockTime());
    f.setPilotName(flight.pilotName());
    f.setAircraftCallsign(flight.aircraftCallsign());
    f.setComments(flight.comments());
    f.setLandingCount(flight.landingCount());

    resolveCoordinates(f);

    *it = f;
    sortFlights(flights);
    m_flights.setValue(std::move(flights));
    m_storage->upsert(f);
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
    for (const auto& flight : m_flights.value()) {
        if (flight.aircraftCallsign().compare(aircraftCallsign, Qt::CaseInsensitive) == 0 && !flight.arrivalICAO().isEmpty()) {
            return flight.arrivalICAO();
        }
    }
    return {};
}


auto Flightlog::FlightLog::nearestAirfield(const QGeoCoordinate& position, double proximityM) -> GeoMaps::Waypoint
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
    if (coord.distanceTo(closest.coordinate()) > proximityM) {
        return {};
    }

    return closest;
}


auto Flightlog::FlightLog::exportToIGC(const QString& uuid) const -> QByteArray
{
    auto targetUuid = QUuid::fromString(uuid);
    const auto& flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == flights.end()) {
        return {};
    }
    return m_recorder->exportToIGC(*it);
}


auto Flightlog::FlightLog::flightsForUuids(const QStringList& uuids) const -> QList<Flight>
{
    if (uuids.isEmpty()) {
        return m_flights.value();
    }
    const auto& allFlights = m_flights.value();
    QList<Flight> result;
    for (const QString& uuid : uuids) {
        const auto id = QUuid::fromString(uuid);
        const auto it = std::ranges::find_if(allFlights, [&](const Flight& f) { return f.uuid() == id; });
        if (it != allFlights.end()) {
            result.append(*it);
        }
    }
    return result;
}


auto Flightlog::FlightLog::exportToForeFlight(const QStringList& uuids) const -> QByteArray
{
    return FlightLogExportForeFlight::toCSV(flightsForUuids(uuids));
}


auto Flightlog::FlightLog::exportToJSON(const QStringList& uuids) const -> QByteArray
{
    return FlightLogExportJSON::toJSON(flightsForUuids(uuids));
}


auto Flightlog::FlightLog::importFromJSON(const QString& fileName) -> QString
{
    QString myFileName = fileName;
    if (myFileName.startsWith(u"file://"_s)) {
        myFileName = myFileName.mid(7);
    }

    QFile file(myFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return tr("Cannot open file: %1").arg(file.errorString());
    }
    const auto raw = file.readAll();
    file.close();

    const auto imported = FlightLogExportJSON::fromJSON(raw);
    if (imported.isEmpty()) {
        return tr("The file does not contain a valid flight log.");
    }

    auto flights = m_flights.value();
    QSet<QUuid> existingUuids;
    existingUuids.reserve(flights.size());
    for (const auto& f : std::as_const(flights)) {
        existingUuids.insert(f.uuid());
    }

    QList<Flight> newFlights;
    for (const auto& importedFlight : imported) {
        if (existingUuids.contains(importedFlight.uuid())) {
            continue;
        }
        auto flight = importedFlight;
        resolveCoordinates(flight);
        flights.append(flight);
        existingUuids.insert(flight.uuid());
        newFlights.append(flight);
    }

    if (newFlights.isEmpty()) {
        return {};
    }

    if (!m_storage->upsertMany(newFlights)) {
        // upsertMany() has already emitted saveError() with the details;
        // returning a non-empty string here suppresses the "N flight(s)
        // imported" success toast so the UI does not report success for an
        // import that was not actually persisted.
        return tr("Failed to save the imported flights to storage. Nothing was imported.");
    }

    sortFlights(flights);
    m_flights.setValue(std::move(flights));
    return {};
}


void Flightlog::FlightLog::removeTrack(const QString& uuid)
{
    auto targetUuid = QUuid::fromString(uuid);
    auto flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == flights.end()) {
        return;
    }

    if (it->trackFile().isEmpty()) {
        return;
    }

    // If this track is currently displayed, hide it first
    if (!m_displayedTrackFile.isEmpty()
        && it->trackFile() == m_displayedTrackFile) {
        hideTrack();
    }

    m_recorder->removeTrack(*it);
    auto updatedFlight = *it;
    m_flights.setValue(std::move(flights));
    m_storage->upsert(updatedFlight);
}


auto Flightlog::FlightLog::displayedTrackPath() const -> QGeoPath
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
    return m_recorder->trackGeoPath();
}


void Flightlog::FlightLog::showTrack(const QString& uuid)
{
    auto targetUuid = QUuid::fromString(uuid);
    const auto& flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == flights.end()) {
        return;
    }
    if (!it->hasTrack()) {
        return;
    }

    // Load into a local first — only commit state if successful
    auto path = m_recorder->loadTrackPath(*it);
    if (path.isEmpty()) {
        return;
    }
    m_displayedTrackPath = QGeoPath(path);
    m_displayedTrackFile = it->trackFile();
    emit displayedTrackPathChanged();
}


void Flightlog::FlightLog::hideTrack()
{
    m_displayedTrackFile.clear();
    m_displayedTrackPath = {};
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

    // Only fill in a coordinate that isn't already set. A coordinate can
    // already be present here because it came from a real GPS fix (automatic
    // flight detection, or the "End Flight" button) rather than an ICAO
    // lookup — that is strictly more trustworthy than an airport's charted
    // position and must not be overwritten.
    if (!flight.departureCoordinate().isValid() && !flight.departureICAO().isEmpty()) {
        auto wp = geoMapProvider->findByID(flight.departureICAO());
        if (wp.coordinate().isValid()) {
            flight.setDepartureCoordinate(wp.coordinate());
        }
    }

    if (!flight.arrivalCoordinate().isValid() && !flight.arrivalICAO().isEmpty()) {
        auto wp = geoMapProvider->findByID(flight.arrivalICAO());
        if (wp.coordinate().isValid()) {
            flight.setArrivalCoordinate(wp.coordinate());
        }
    }
}


void Flightlog::FlightLog::sortFlights(QList<Flight>& flights)
{
    std::sort(flights.begin(), flights.end(),
              [](const Flight& a, const Flight& b) {
                  return a.startTime() > b.startTime();
              });
}


//
// Persistence
//

void Flightlog::FlightLog::load()
{
    auto newFlights = m_storage->loadAll();
    sortFlights(newFlights);
    m_flights.setValue(std::move(newFlights));
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
    if (m_detector == nullptr) {
        return;
    }
    if (m_detector->detectionState() == FlightDetector::InFlight) {
        m_displayedTrackFile.clear();
        m_displayedTrackPath = {};
        emit displayedTrackPathChanged();
    }
    emit detectionStateChanged();
}


void Flightlog::FlightLog::onTakeoffDetected(const QString& departureICAO,
                                              const QGeoCoordinate& departureCoordinate,
                                              const QDateTime& startTime,
                                              const QString& aircraftCallsign)
{
    auto timeStr = startTime.toUTC().time().toString(u"HH:mm"_s);
    Flight flight;
    flight.setDepartureICAO(departureICAO);
    flight.setDepartureCoordinate(departureCoordinate);
    flight.setStartTime(startTime);
    flight.setAircraftCallsign(aircraftCallsign);

    // Add the preliminary flight entry so it appears in the list immediately
    addFlight(flight);

    // Track which flight is in progress by its stable UUID
    m_currentFlightUuid = flight.uuid();

#ifdef Q_OS_ANDROID
    // Post a notification with sound so the pilot knows takeoff was detected,
    // even if the app is in the background.
    auto title = tr("Takeoff Detected");
    auto message = tr("Departed %1 at %2 UTC").arg(
        departureICAO.isEmpty() ? tr("unknown") : departureICAO,
        timeStr);
    postAndroidNotification(NOTIFICATION_ID_EVENT, title, message);
#endif

#ifdef Q_OS_IOS
    ObjCAdapter::postNotification(
        tr("Takeoff Detected"),
        tr("Departed %1 at %2 UTC").arg(
            departureICAO.isEmpty() ? tr("unknown") : departureICAO,
            timeStr));
#endif

    emit takeoffDetected(timeStr);
}


void Flightlog::FlightLog::onLandingDetected(const QString& arrivalICAO,
                                                const QGeoCoordinate& arrivalCoordinate,
                                                const QDateTime& landingTime,
                                                int landingCount)
{
    auto timeStr = landingTime.toUTC().time().toString(u"HH:mm"_s);
    // Complete the in-progress flight by UUID lookup
    auto flights = m_flights.value();
    auto it = std::ranges::find_if(flights, [this](const Flight& f) {
        return f.uuid() == m_currentFlightUuid;
    });
    if (it != flights.end()) {
        auto& flight = *it;
        if (!arrivalICAO.isEmpty()) {
            flight.setArrivalICAO(arrivalICAO);
        }
        if (arrivalCoordinate.isValid()) {
            flight.setArrivalCoordinate(arrivalCoordinate);
        }
        flight.setLandingTime(landingTime);
        flight.setLandingCount(landingCount);

        // Save the track from the recorder to an IGC file, then always
        // clear it — even when recording was off — so no stale points
        // linger in RAM or remain visible on the map.
        if (trackRecording()) {
            if (m_recorder->saveTrack(flight)) {
                // Cache the geo path for map display, then free recorder RAM
                m_displayedTrackPath = m_recorder->trackGeoPath();
                m_displayedTrackFile = m_displayedTrackPath.path().isEmpty() ? QString{} : flight.trackFile();
            } else {
                emit saveError(tr("Failed to save GPS track for flight from %1.").arg(flight.departureICAO()));
            }
        }
        m_recorder->clearTrack();
        auto updatedFlight = flight;
        m_flights.setValue(std::move(flights));
        m_storage->upsert(updatedFlight);
        emit displayedTrackPathChanged();
    }

    // Flight recording is complete
    m_currentFlightUuid = {};

    emit landingDetected(timeStr);

#ifdef Q_OS_ANDROID
    // Post a notification with sound so the pilot knows landing was detected.
    auto title = tr("Landing Detected");
    auto message = tr("Landed %1 at %2 UTC").arg(
        arrivalICAO.isEmpty() ? tr("unknown") : arrivalICAO,
        timeStr);
    postAndroidNotification(NOTIFICATION_ID_EVENT, title, message);
#endif

#ifdef Q_OS_IOS
    ObjCAdapter::postNotification(
        tr("Landing Detected"),
        tr("Landed %1 at %2 UTC").arg(
            arrivalICAO.isEmpty() ? tr("unknown") : arrivalICAO,
            timeStr));
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

    // Update the recorder first so the current position is committed to the
    // open leg before the detector can emit landingDetected and clear it.
    // This ordering is correct regardless of whether the landingDetected
    // connection is direct or queued.
    if (trackRecording()) {
        auto pressAlt = GlobalObject::positionProvider()->pressureAltitude();
        m_recorder->processPositionUpdate(m_detector->detectionState(), info, pressAlt);
    }

    m_detector->processPositionUpdate(info);
}


void Flightlog::FlightLog::onAutoFlightDetectionChanged()
{
#ifdef Q_OS_IOS
    // Request local notification permission when the user first enables
    // automatic flight detection. The system shows the dialog at most once;
    // subsequent calls are no-ops if permission was already granted or denied.
    if (GlobalObject::globalSettings()->autoFlightDetection()) {
        ObjCAdapter::requestNotificationPermission();
    }
    GlobalObject::positionProvider()->setBackgroundUpdates(
        u"flightlog"_s,
        GlobalObject::globalSettings()->autoFlightDetection());
#endif

    // Ensure the satellite GPS source is running whenever auto-detection is
    // enabled. QML calls startUpdates() on Component.onCompleted, but if the
    // process was killed and restarted by the Android foreground service
    // (START_STICKY) the QML engine may not re-execute that handler.
    // Calling startUpdates() here guarantees GPS flows into the detector
    // regardless of whether the UI is visible.
    if (GlobalObject::globalSettings()->autoFlightDetection()) {
        GlobalObject::positionProvider()->startUpdates();
    } else {
        // Detection was just disabled. Reset the detector's own state machine
        // so it doesn't stay stuck in InFlight/LandingPhase — otherwise a
        // later endFlight() call would still pass the detector's guard and
        // emit landingDetected for a flight m_currentFlightUuid (cleared
        // below) can no longer identify.
        if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
            m_detector->resetDetection();
        }

        // If a flight was being recorded, discard the in-memory GPS track so
        // it doesn't appear frozen on the map or bleed into the next flight.
        // The flight entry itself (with its start time) is kept in the log —
        // the pilot can edit it manually.
        if (!m_currentFlightUuid.isNull()) {
            m_recorder->clearTrack();
            m_currentFlightUuid = {};
            if (m_displayedTrackFile.isEmpty()) {
                // Was showing the live trace — tell the map it's gone
                emit displayedTrackPathChanged();
            }
        }
    }

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

    // Manage the "no GPS" warning notification.
    if (enabled && !GlobalObject::positionProvider()->receivingPositionInfo()) {
        // Start grace-period timer — notification fires only if GPS doesn't
        // arrive within 30 seconds (avoids false alarm at startup).
        m_noGPSTimer.start();
    } else {
        // Detection disabled or GPS already available: stop any pending timer
        // and cancel an existing notification.
        m_noGPSTimer.stop();
        cancelAndroidNotification(NOTIFICATION_ID_NO_GPS);
    }
#endif
}


#ifdef Q_OS_ANDROID
void Flightlog::FlightLog::onReceivingPositionInfoChanged(bool receiving)
{
    if (receiving) {
        // Position arrived — stop the timer and clear any existing warning.
        m_noGPSTimer.stop();
        cancelAndroidNotification(NOTIFICATION_ID_NO_GPS);
    } else {
        // Position lost — start the grace-period timer if auto-detection is on.
        if (GlobalObject::globalSettings()->autoFlightDetection()) {
            m_noGPSTimer.start();
        }
    }
}
#endif
