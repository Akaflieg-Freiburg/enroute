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
#include <QDebug>
#include <QFile>
#include <QGeoPath>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

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
    auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.trackFile() == m_displayedTrackFile;
    });
    if (it == m_flights.end()) {
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
    const auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.uuid() == id;
    });
    if (it == m_flights.end()) {
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
    }

    m_recorder.removeTrack(flight);
    m_flights.erase(it);
    save();
    emit flightsChanged();
}


void Flightlog::FlightLog::removeFlights(const QStringList& uuids)
{
    bool changed = false;
    for (const QString& uuid : uuids) {
        const auto id = QUuid::fromString(uuid);
        if (id.isNull()) {
            continue;
        }
        const auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
            return f.uuid() == id;
        });
        if (it == m_flights.end()) {
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
        }
        m_recorder.removeTrack(*it);
        m_flights.erase(it);
        changed = true;
    }
    if (changed) {
        save();
        emit flightsChanged();
    }
}


void Flightlog::FlightLog::clearFlights()
{
    if (m_flights.isEmpty()) {
        return;
    }
    hideTrack();
    if (m_detector != nullptr && m_detector->detectionState() != FlightDetector::Idle) {
        m_detector->resetDetection();
    }
    m_currentFlightUuid = {};
    for (auto& flight : m_flights) {
        m_recorder.removeTrack(flight);
    }
    m_flights.clear();
    save();
    emit flightsChanged();
}



void Flightlog::FlightLog::updateFlight(const QString& uuid, const Flightlog::Flight& flight)
{
    auto targetUuid = QUuid::fromString(uuid);
    auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == m_flights.end()) {
        return;
    }
    auto index = static_cast<int>(std::ranges::distance(m_flights.begin(), it));

    // Start from the existing entry so that read-only fields (trackFile,
    // landingCount, coordinates) are preserved by default.
    auto f = *it;
    f.setDepartureICAO(flight.departureICAO());
    f.setArrivalICAO(flight.arrivalICAO());
    f.setOffBlockTime(flight.offBlockTime());
    f.setStartTime(flight.startTime());
    f.setLandingTime(flight.landingTime());
    f.setOnBlockTime(flight.onBlockTime());
    f.setPilotName(flight.pilotName());
    f.setAircraftCallsign(flight.aircraftCallsign());
    f.setComments(flight.comments());

    // Re-resolve coordinates; fall back to existing ones if resolution fails
    // but the ICAO code is unchanged.
    const auto& old = *it;
    resolveCoordinates(f);
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


auto Flightlog::FlightLog::exportToIGC(const QString& uuid) -> QByteArray
{
    auto targetUuid = QUuid::fromString(uuid);
    auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == m_flights.end()) {
        return {};
    }
    return m_recorder.exportToIGC(*it);
}


auto Flightlog::FlightLog::flightsForUuids(const QStringList& uuids) const -> QList<Flight>
{
    if (uuids.isEmpty()) {
        return m_flights;
    }
    QList<Flight> result;
    for (const QString& uuid : uuids) {
        const auto id = QUuid::fromString(uuid);
        const auto it = std::ranges::find_if(m_flights, [&](const Flight& f) { return f.uuid() == id; });
        if (it != m_flights.end()) {
            result.append(*it);
        }
    }
    return result;
}


auto Flightlog::FlightLog::exportToForeFlight(const QStringList& uuids) -> QByteArray
{
    const auto toExport = flightsForUuids(uuids);
    if (toExport.isEmpty()) {
        return {};
    }

    // Wrap a CSV field in double-quotes if it contains commas, quotes, or line breaks
    auto csvField = [](const QString& s) -> QString {
        if (s.contains(u',') || s.contains(u'"') || s.contains(u'\n') || s.contains(u'\r')) {
            return u'"' + QString(s).replace(u'"', u"\"\""_s) + u'"';
        }
        return s;
    };

    // Format a QDateTime as HH:MM UTC; empty string if invalid
    auto timeHHMM = [](const QDateTime& dt) -> QString {
        return dt.isValid() ? dt.toUTC().toString(u"HH:mm"_s) : QString();
    };

    // Produce the exact two-table structure of the official ForeFlight
    // Logbook Import template: magic row, blank, Aircraft Table, blank, Flights Table.
    QString csv;

    // Row 1: magic identifier; row 2: blank
    csv += u"ForeFlight Logbook Import\r\n"_s;
    csv += u"\r\n"_s;

    // ── Aircraft Table ────────────────────────────────────────────────────
    csv += u"Aircraft Table\r\n"_s;
    csv += u"Text,Text,Text,YYYY,Text,Text,Text,Text,Text,Boolean,Boolean,Boolean,Boolean\r\n"_s;
    csv += u"AircraftID,EquipmentType,TypeCode,Year,Make,Model,GearType,EngineType,Category/Class,Complex,High Performance,Pressurized,TAA\r\n"_s;

    {
        QStringList seenAircraft;
        for (const auto& f : std::as_const(toExport))
        {
            const QString id = f.aircraftCallsign();
            if (!id.isEmpty() && !seenAircraft.contains(id))
            {
                seenAircraft.append(id);
                csv += csvField(id) + u",aircraft,,,,,,,,,,,\r\n"_s;
            }
        }
    }

    csv += u"\r\n"_s;

    // ── Flights Table ─────────────────────────────────────────────────────
    csv += u"Flights Table\r\n"_s;

    // Type row (matches ForeFlight template column order exactly)
    csv += u"Date,Text,Text,Text,Text,"
           u"HH:MM,HH:MM,HH:MM,HH:MM,HH:MM,HH:MM,"
           u"Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,"
           u"Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,"
           u"Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,"
           u"Decimal or HH:MM,Decimal or HH:MM,Number,Decimal,"
           u"Number,Number,Number,Number,Number,"
           u"Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,"
           u"Decimal,Decimal,Decimal,Decimal,Number,"
           u"Packed Detail,Packed Detail,Packed Detail,Packed Detail,Packed Detail,Packed Detail,"
           u"Decimal or HH:MM,Decimal or HH:MM,Decimal or HH:MM,Text,Text,"
           u"Packed Detail,Packed Detail,Packed Detail,Packed Detail,Packed Detail,Packed Detail,"
           u"Text,Boolean,Boolean,Boolean,Boolean,Boolean\r\n"_s;

    // Column names row
    csv += u"Date,AircraftID,From,To,Route,"
           u"TimeOut,TimeOff,TimeOn,TimeIn,OnDuty,OffDuty,"
           u"TotalTime,PIC,SIC,Night,Solo,CrossCountry,PICUS,MultiPilot,IFR,"
           u"Examiner,NVG,NVGOps,Distance,"
           u"DayTakeoffs,DayLandingsFullStop,NightTakeoffs,NightLandingsFullStop,AllLandings,"
           u"ActualInstrument,SimulatedInstrument,GroundTraining,GroundTrainingGiven,"
           u"HobbsStart,HobbsEnd,TachStart,TachEnd,Holds,"
           u"Approach1,Approach2,Approach3,Approach4,Approach5,Approach6,"
           u"DualGiven,DualReceived,SimulatedFlight,InstructorName,InstructorComments,"
           u"Person1,Person2,Person3,Person4,Person5,Person6,"
           u"PilotComments,Flight Review,IPC,Checkride,FAA 61.58,NVG Proficiency\r\n"_s;

    for (const auto& flight : std::as_const(toExport)) {
        const QString date = flight.startTime().isValid()
            ? flight.startTime().toUTC().date().toString(u"yyyy-MM-dd"_s)
            : QString();

        // TotalTime: wheel-off to wheel-on, in decimal hours (1 decimal place)
        const double flightSecs = flight.startTime().isValid() && flight.landingTime().isValid()
            ? static_cast<double>(flight.startTime().secsTo(flight.landingTime())) : 0.0;
        const QString totalTime = flightSecs > 0.0
            ? QString::number(flightSecs / 3600.0, 'f', 1) : QString();

        // AllLandings covers all landing types; we don't split day/night/touch-and-go
        const QString allLandings = flight.landingCount() > 0
            ? QString::number(flight.landingCount()) : QString();

        csv += csvField(date)                              + u","_s; // Date
        csv += csvField(flight.aircraftCallsign())         + u","_s; // AircraftID
        csv += csvField(flight.departureICAO())            + u","_s; // From
        csv += csvField(flight.arrivalICAO())              + u","_s; // To
        csv +=                                               u","_s; // Route
        csv += csvField(timeHHMM(flight.offBlockTime()))   + u","_s; // TimeOut
        csv += csvField(timeHHMM(flight.startTime()))      + u","_s; // TimeOff
        csv += csvField(timeHHMM(flight.landingTime()))    + u","_s; // TimeOn
        csv += csvField(timeHHMM(flight.onBlockTime()))    + u","_s; // TimeIn
        csv +=                                               u","_s; // OnDuty
        csv +=                                               u","_s; // OffDuty
        csv += csvField(totalTime)                         + u","_s; // TotalTime
        csv +=                                               u","_s; // PIC
        csv +=                                               u","_s; // SIC
        csv +=                                               u","_s; // Night
        csv +=                                               u","_s; // Solo
        csv +=                                               u","_s; // CrossCountry
        csv +=                                               u","_s; // PICUS
        csv +=                                               u","_s; // MultiPilot
        csv +=                                               u","_s; // IFR
        csv +=                                               u","_s; // Examiner
        csv +=                                               u","_s; // NVG
        csv +=                                               u","_s; // NVGOps
        csv +=                                               u","_s; // Distance
        csv +=                                               u","_s; // DayTakeoffs
        csv +=                                               u","_s; // DayLandingsFullStop
        csv +=                                               u","_s; // NightTakeoffs
        csv +=                                               u","_s; // NightLandingsFullStop
        csv += csvField(allLandings)                       + u","_s; // AllLandings
        csv +=                                               u","_s; // ActualInstrument
        csv +=                                               u","_s; // SimulatedInstrument
        csv +=                                               u","_s; // GroundTraining
        csv +=                                               u","_s; // GroundTrainingGiven
        csv +=                                               u","_s; // HobbsStart
        csv +=                                               u","_s; // HobbsEnd
        csv +=                                               u","_s; // TachStart
        csv +=                                               u","_s; // TachEnd
        csv +=                                               u","_s; // Holds
        csv +=                                               u","_s; // Approach1
        csv +=                                               u","_s; // Approach2
        csv +=                                               u","_s; // Approach3
        csv +=                                               u","_s; // Approach4
        csv +=                                               u","_s; // Approach5
        csv +=                                               u","_s; // Approach6
        csv +=                                               u","_s; // DualGiven
        csv +=                                               u","_s; // DualReceived
        csv +=                                               u","_s; // SimulatedFlight
        csv +=                                               u","_s; // InstructorName
        csv +=                                               u","_s; // InstructorComments
        csv += csvField(flight.pilotName())                + u","_s; // Person1
        csv +=                                               u","_s; // Person2
        csv +=                                               u","_s; // Person3
        csv +=                                               u","_s; // Person4
        csv +=                                               u","_s; // Person5
        csv +=                                               u","_s; // Person6
        csv += csvField(flight.comments())                 + u","_s; // PilotComments
        csv +=                                               u","_s; // Flight Review
        csv +=                                               u","_s; // IPC
        csv +=                                               u","_s; // Checkride
        csv +=                                               u","_s; // FAA 61.58
        csv +=                                             u"\r\n"_s; // NVG Proficiency (last)
    }

    return csv.toUtf8();
}


auto Flightlog::FlightLog::exportToJSON(const QStringList& uuids) -> QByteArray
{
    const auto toExport = flightsForUuids(uuids);
    if (toExport.isEmpty()) {
        return {};
    }

    const QJsonDocument doc = flightsToJsonDocument(toExport);
    return doc.toJson();
}


void Flightlog::FlightLog::removeTrack(const QString& uuid)
{
    auto targetUuid = QUuid::fromString(uuid);
    auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == m_flights.end()) {
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

    m_recorder.removeTrack(*it);

    save();
    emit flightsChanged();
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
    return m_recorder.trackGeoPath();
}


void Flightlog::FlightLog::showTrack(const QString& uuid)
{
    auto targetUuid = QUuid::fromString(uuid);
    auto it = std::ranges::find_if(m_flights, [&](const Flight& f) {
        return f.uuid() == targetUuid;
    });
    if (it == m_flights.end()) {
        return;
    }
    if (!it->hasTrack()) {
        return;
    }

    // Load into a local first — only commit state if successful
    auto path = m_recorder.loadTrackPath(*it);
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
    // UUID-based tracking makes re-finding unnecessary — sort freely.
    std::sort(m_flights.begin(), m_flights.end(),
              [](const Flight& a, const Flight& b) {
                  return a.startTime() > b.startTime();
              });
}


//
// Persistence
//

void Flightlog::FlightLog::save()
{
    const QJsonDocument doc = flightsToJsonDocument(m_flights);

    // Use QSaveFile so a failed or partial write cannot corrupt the existing
    // file: it writes to a temporary file and commit() atomically renames.
    QSaveFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "FlightLog::save: cannot open" << m_fileName
                   << "for writing:" << file.errorString();
        emit saveError(file.errorString());
        return;
    }

    const QByteArray json = doc.toJson();
    if (file.write(json) != json.size()) {
        qWarning() << "FlightLog::save: write failed for" << m_fileName
                   << ":" << file.errorString();
        file.cancelWriting();
        emit saveError(file.errorString());
        return;
    }

    if (!file.commit()) {
        qWarning() << "FlightLog::save: commit failed for" << m_fileName
                   << ":" << file.errorString();
        emit saveError(file.errorString());
    }
}


void Flightlog::FlightLog::load()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(raw, &parseError);
    if (!doc.isObject()) {
        quarantineFlightLogFile(parseError.errorString());
        return;
    }

    const auto root = doc.object();
    if (root.value(u"content"_s).toString() != u"flightLog"_s) {
        quarantineFlightLogFile(u"unexpected content tag"_s);
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


void Flightlog::FlightLog::quarantineFlightLogFile(const QString& reason)
{
    const QString ts = QDateTime::currentDateTimeUtc().toString(u"yyyyMMddTHHmmssZ"_s);
    const QString backup = m_fileName + u"."_s + ts + u".corrupt"_s;
    QFile::rename(m_fileName, backup);
    qWarning() << "FlightLog::load:" << reason
               << "- damaged file renamed to" << backup;
    // Emit via a queued connection: load() is called from the constructor,
    // before QML signal connections are established.
    QMetaObject::invokeMethod(this, [this, reason]() {
        emit saveError(tr("The flight log file could not be read and has been reset (%1). "
                          "Your previous flight log data is no longer available.")
                       .arg(reason));
    }, Qt::QueuedConnection);
}


auto Flightlog::FlightLog::flightsToJsonDocument(const QList<Flight>& flights) -> QJsonDocument
{
    QJsonArray array;
    for (const auto& flight : flights) {
        array.append(flight.toJSON());
    }

    QJsonObject root;
    root[u"content"_s] = u"flightLog"_s;
    root[u"version"_s] = 1;
    root[u"exportDate"_s] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    root[u"flights"_s] = array;

    return QJsonDocument(root);
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
        m_displayedTrackPath = {};
        emit displayedTrackPathChanged();
    }
    emit detectionStateChanged();
}


void Flightlog::FlightLog::onTakeoffDetected(const QString& departureICAO,
                                              const QGeoCoordinate& departureCoordinate,
                                              const QDateTime& startTime,
                                              const QString& aircraftCallsign,
                                              const QString& timeStr)
{
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
                                                int landingCount,
                                                const QString& timeStr)
{
    // Complete the in-progress flight by UUID lookup
    auto it = std::ranges::find_if(m_flights, [this](const Flight& f) {
        return f.uuid() == m_currentFlightUuid;
    });
    if (it != m_flights.end()) {
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
            if (m_recorder.saveTrack(flight)) {
                // Cache the geo path for map display, then free recorder RAM
                m_displayedTrackPath = m_recorder.trackGeoPath();
                m_displayedTrackFile = m_displayedTrackPath.path().isEmpty() ? QString{} : flight.trackFile();
            } else {
                emit saveError(tr("Failed to save GPS track for flight from %1.").arg(flight.departureICAO()));
            }
        }
        m_recorder.clearTrack();

        save();
        emit flightsChanged();
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
        m_recorder.processPositionUpdate(m_detector->detectionState(), info, pressAlt);
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
        // Detection was just disabled. If a flight was being recorded, discard
        // the in-memory GPS track so it doesn't appear frozen on the map or
        // bleed into the next flight. The flight entry itself (with its start
        // time) is kept in the log — the pilot can edit it manually.
        if (!m_currentFlightUuid.isNull()) {
            m_recorder.clearTrack();
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
