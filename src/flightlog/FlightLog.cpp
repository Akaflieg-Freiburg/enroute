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


auto Flightlog::FlightLog::exportToIGC(int index) -> QByteArray
{
    if (index < 0 || index >= m_flights.size()) {
        return {};
    }
    return m_recorder.exportToIGC(m_flights.at(index));
}


auto Flightlog::FlightLog::exportToForeFlight(const QVariantList& indices) -> QByteArray
{
    // See Foreflight Docu: 
    // https://support.foreflight.com/hc/en-us/articles/215998368-Is-there-a-properly-formatted-sample-logbook-available-for-viewing
    // https://support.foreflight.com/hc/en-us/article_attachments/35060835400727

    // Collect flights to export (empty indices = export all)
    QList<Flight> toExport;
    if (indices.isEmpty()) {
        toExport = m_flights;
    } else {
        for (const QVariant& v : indices) {
            const int idx = v.toInt();
            if (idx >= 0 && idx < m_flights.size()) {
                toExport.append(m_flights.at(idx));
            }
        }
    }
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


auto Flightlog::FlightLog::exportToJSON(const QVariantList& indices) -> QByteArray
{
    // Collect flights to export (empty indices = export all)
    QList<Flight> toExport;
    if (indices.isEmpty()) {
        toExport = m_flights;
    } else {
        for (const QVariant& v : indices) {
            const int idx = v.toInt();
            if (idx >= 0 && idx < m_flights.size()) {
                toExport.append(m_flights.at(idx));
            }
        }
    }
    if (toExport.isEmpty()) {
        return {};
    }

    const QJsonDocument doc = flightsToJsonDocument(toExport);
    return doc.toJson();
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
    if (m_displayedTrackPath.isEmpty()) {
        return;
    }
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
        m_displayedTrackPath.clear();
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

    // Track that a flight is now in progress (always at index 0 due to prepending in addFlight)
    m_currentFlightIndex = 0;

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
    m_detector->processPositionUpdate(info);

    // Forward to recorder — it manages preflight/inflight/postlanding internally
    if (trackRecording()) {
        auto pressAlt = GlobalObject::positionProvider()->pressureAltitude();
        m_recorder.processPositionUpdate(m_detector->detectionState(), info, pressAlt);
    }
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
