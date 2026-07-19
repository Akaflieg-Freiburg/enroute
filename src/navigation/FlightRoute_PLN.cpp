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

#include <QXmlStreamWriter>
#include <QtMath>

#include "FlightRoute.h"
#include "units/Distance.h"

using namespace Qt::Literals::StringLiterals;


namespace {

// Converts an angle to a DMS string of the form N53° 53' 55.87", as used in
// the WorldPosition elements of PLN files. Rounding of the seconds carries
// over into minutes and degrees, so that the result never contains 60.00".
QString toPDMS(double angle, QChar positiveHemisphere, QChar negativeHemisphere)
{
    QChar const hemisphere = (angle < 0) ? negativeHemisphere : positiveHemisphere;
    auto absoluteValue = qAbs(angle);
    auto degree = static_cast<int>(absoluteValue);
    auto minute = static_cast<int>((absoluteValue - degree) * 60.0);
    auto second = (absoluteValue - degree - minute / 60.0) * 3600.0;
    second = qRound(second * 100.0) / 100.0;
    if (second >= 60.0)
    {
        second -= 60.0;
        minute += 1;
    }
    if (minute >= 60)
    {
        minute -= 60;
        degree += 1;
    }
    return hemisphere + QString::number(degree) + u"° "_s + QString::number(minute) + u"' "_s
            + QString::number(second, 'f', 2) + u"\""_s;
}

// Converts a coordinate to an LLA string of the form
// N53° 53' 55.87",E10° 41' 21.10",+000110.00 with the altitude in feet, as
// used in the WorldPosition, DepartureLLA and DestinationLLA elements of PLN
// files.
QString toLLA(const QGeoCoordinate& coordinate)
{
    double altitudeInFeet = 0.0;
    if (coordinate.type() == QGeoCoordinate::Coordinate3D)
    {
        altitudeInFeet = Units::Distance::fromM(coordinate.altitude()).toFeet();
    }
    QChar const sign = (altitudeInFeet < 0) ? u'-' : u'+';
    return toPDMS(coordinate.latitude(), u'N', u'S') + u","_s
            + toPDMS(coordinate.longitude(), u'E', u'W') + u","_s
            + sign + QStringLiteral("%1").arg(qAbs(altitudeInFeet), 9, 'f', 2, QChar(u'0'));
}

// Restricts a string to the character set [A-Z0-9], applying Unicode
// normalization so that letters with diacritics survive as their base letters.
// The result is truncated to 10 characters; the fallback for empty results is
// "WP".
QString sanitizedIdentifier(const QString& proposal)
{
    QString result;
    for (QChar c : proposal.normalized(QString::NormalizationForm_KD).toUpper())
    {
        if (((c >= u'A') && (c <= u'Z')) || ((c >= u'0') && (c <= u'9')))
        {
            result += c;
        }
    }
    result.truncate(10);
    if (result.isEmpty())
    {
        return u"WP"_s;
    }
    return result;
}

} // namespace


auto Navigation::FlightRoute::toPln() const -> QByteArray
{
    QVector<GeoMaps::Waypoint> waypoints;
    for (const auto& waypoint : m_waypoints.value())
    {
        if (!waypoint.isValid())
        {
            continue; // skip silently
        }
        waypoints.append(waypoint);
    }

    // Cruising altitude in feet: highest waypoint altitude, rounded up to the
    // next 100 ft; VFR default if no waypoint carries altitude information.
    double cruisingAltitude = 5500.0;
    double maxAltitudeInFeet = 0.0;
    for (const auto& waypoint : waypoints)
    {
        if (waypoint.coordinate().type() == QGeoCoordinate::Coordinate3D)
        {
            maxAltitudeInFeet = qMax(maxAltitudeInFeet, Units::Distance::fromM(waypoint.coordinate().altitude()).toFeet());
        }
    }
    if (maxAltitudeInFeet > 0.0)
    {
        cruisingAltitude = qCeil(maxAltitudeInFeet / 100.0) * 100.0;
    }

    QByteArray result;
    QXmlStreamWriter stream(&result);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);

    stream.writeStartDocument();
    stream.writeStartElement(u"SimBase.Document"_s);
    stream.writeAttribute(u"Type"_s, u"AceXML"_s);
    stream.writeAttribute(u"version"_s, u"1,0"_s);
    stream.writeTextElement(u"Descr"_s, u"AceXML Document"_s);

    stream.writeStartElement(u"FlightPlan.FlightPlan"_s);
    stream.writeTextElement(u"Title"_s, suggestedFilename());
    stream.writeTextElement(u"FPType"_s, u"VFR"_s);
    stream.writeTextElement(u"RouteType"_s, u"Direct"_s);
    stream.writeTextElement(u"CruisingAlt"_s, QString::number(qRound(cruisingAltitude)));
    if (!waypoints.isEmpty())
    {
        const auto& departure = waypoints.constFirst();
        const auto& destination = waypoints.constLast();
        stream.writeTextElement(u"DepartureID"_s, sanitizedIdentifier(departure.shortName()));
        stream.writeTextElement(u"DepartureLLA"_s, toLLA(departure.coordinate()));
        stream.writeTextElement(u"DestinationID"_s, sanitizedIdentifier(destination.shortName()));
        stream.writeTextElement(u"DestinationLLA"_s, toLLA(destination.coordinate()));
        stream.writeTextElement(u"Descr"_s, u"Created by Enroute Flight Navigation"_s);
        if (!departure.name().isEmpty())
        {
            stream.writeTextElement(u"DepartureName"_s, departure.name());
        }
        if (!destination.name().isEmpty())
        {
            stream.writeTextElement(u"DestinationName"_s, destination.name());
        }
    }
    stream.writeStartElement(u"AppVersion"_s);
    stream.writeTextElement(u"AppVersionMajor"_s, u"11"_s);
    stream.writeTextElement(u"AppVersionBuild"_s, u"282174"_s);
    stream.writeEndElement(); // AppVersion

    for (const auto& waypoint : waypoints)
    {
        bool const isAirport = (waypoint.type() == u"AD") && !waypoint.ICAOCode().isEmpty();
        stream.writeStartElement(u"ATCWaypoint"_s);
        stream.writeAttribute(u"id"_s, sanitizedIdentifier(waypoint.shortName()));
        stream.writeTextElement(u"ATCWaypointType"_s, isAirport ? u"Airport"_s : u"User"_s);
        stream.writeTextElement(u"WorldPosition"_s, toLLA(waypoint.coordinate()));
        if (isAirport)
        {
            stream.writeStartElement(u"ICAO"_s);
            stream.writeTextElement(u"ICAOIdent"_s, waypoint.ICAOCode());
            stream.writeEndElement(); // ICAO
        }
        stream.writeEndElement(); // ATCWaypoint
    }

    stream.writeEndElement(); // FlightPlan.FlightPlan
    stream.writeEndElement(); // SimBase.Document
    stream.writeEndDocument();

    return result;
}
