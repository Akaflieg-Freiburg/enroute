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

#include "flightlog/FlightLogExportForeFlight.h"

using namespace Qt::Literals::StringLiterals;


auto Flightlog::FlightLogExportForeFlight::toCSV(const QList<Flight>& flights) -> QByteArray
{
    if (flights.isEmpty()) {
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
        for (const auto& f : std::as_const(flights))
        {
            const QString id = f.aircraftCallsign();
            const bool alreadySeen = std::ranges::any_of(seenAircraft, [&](const QString& s) {
                return s.compare(id, Qt::CaseInsensitive) == 0;
            });
            if (!id.isEmpty() && !alreadySeen)
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

    for (const auto& flight : std::as_const(flights)) {
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
