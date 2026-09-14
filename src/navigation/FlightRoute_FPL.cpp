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
#include <QXmlStreamWriter>

#include "FlightRoute.h"

using namespace Qt::Literals::StringLiterals;


namespace {

// Restricts a string to the character set [A-Z0-9], applying Unicode
// normalization so that letters with diacritics survive as their base letters.
// The result is truncated to maxLength characters. Garmin devices reject
// identifiers outside this character set.
QString sanitizedGarminString(const QString& proposal, qsizetype maxLength, bool allowSpaceAndDash)
{
    QString result;
    for (QChar c : proposal.normalized(QString::NormalizationForm_KD).toUpper())
    {
        if (((c >= u'A') && (c <= u'Z')) || ((c >= u'0') && (c <= u'9'))
                || (allowSpaceAndDash && ((c == u' ') || (c == u'-'))))
        {
            result += c;
        }
    }
    result.truncate(maxLength);
    return result.trimmed();
}

struct TableEntry
{
    GeoMaps::Waypoint waypoint;
    QString identifier;
    QString type;
};

// Assigns each waypoint an entry in the Garmin waypoint table. Identical
// waypoints share one entry; distinct waypoints always receive distinct
// identifiers, because the file format resolves route points through their
// identifier.
qsizetype tableEntryFor(const GeoMaps::Waypoint& waypoint, QVector<TableEntry>& table)
{
    for (qsizetype i = 0; i < table.size(); i++)
    {
        if (table[i].waypoint == waypoint)
        {
            return i;
        }
    }

    QString type = u"USER WAYPOINT"_s;
    QString identifier;
    if ((waypoint.type() == u"AD") && !waypoint.ICAOCode().isEmpty())
    {
        type = u"AIRPORT"_s;
        identifier = sanitizedGarminString(waypoint.ICAOCode(), 10, false);
    }
    else
    {
        identifier = sanitizedGarminString(waypoint.shortName(), 10, false);
    }
    if (identifier.isEmpty())
    {
        identifier = u"WP"_s;
    }

    auto isTaken = [&table](const QString& id) {
        return std::any_of(table.cbegin(), table.cend(),
                           [&id](const TableEntry& entry) { return entry.identifier == id; });
    };
    if (isTaken(identifier))
    {
        auto base = identifier.left(8);
        for (int count = 1; count <= 99; count++)
        {
            auto candidate = base + QStringLiteral("%1").arg(count, 2, 10, QChar(u'0'));
            if (!isTaken(candidate))
            {
                identifier = candidate;
                break;
            }
        }
    }

    table.append({waypoint, identifier, type});
    return table.size() - 1;
}

} // namespace


auto Navigation::FlightRoute::toFpl() const -> QByteArray
{
    QVector<TableEntry> table;
    QVector<qsizetype> routePoints;
    for (const auto& waypoint : m_waypoints.value())
    {
        if (!waypoint.isValid())
        {
            continue; // skip silently
        }
        routePoints.append(tableEntryFor(waypoint, table));
    }

    QByteArray result;
    QXmlStreamWriter stream(&result);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);

    const QString ns = u"http://www8.garmin.com/xmlschemas/FlightPlan/v1"_s;
    stream.writeStartDocument();
    stream.writeDefaultNamespace(ns);
    stream.writeStartElement(ns, u"flight-plan"_s);
    stream.writeTextElement(ns, u"created"_s, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    stream.writeStartElement(ns, u"waypoint-table"_s);
    for (const auto& entry : table)
    {
        stream.writeStartElement(ns, u"waypoint"_s);
        stream.writeTextElement(ns, u"identifier"_s, entry.identifier);
        stream.writeTextElement(ns, u"type"_s, entry.type);
        auto position = entry.waypoint.coordinate();
        stream.writeTextElement(ns, u"lat"_s, QString::number(position.latitude(), 'f', 8));
        stream.writeTextElement(ns, u"lon"_s, QString::number(position.longitude(), 'f', 8));
        auto comment = sanitizedGarminString(entry.waypoint.name(), 25, true);
        if (!comment.isEmpty())
        {
            stream.writeTextElement(ns, u"comment"_s, comment);
        }
        stream.writeEndElement(); // waypoint
    }
    stream.writeEndElement(); // waypoint-table

    stream.writeStartElement(ns, u"route"_s);
    auto routeName = sanitizedGarminString(suggestedFilename(), 25, true);
    if (routeName.isEmpty())
    {
        routeName = u"FLIGHT ROUTE"_s;
    }
    stream.writeTextElement(ns, u"route-name"_s, routeName);
    stream.writeTextElement(ns, u"flight-plan-index"_s, u"1"_s);
    for (auto index : routePoints)
    {
        stream.writeStartElement(ns, u"route-point"_s);
        stream.writeTextElement(ns, u"waypoint-identifier"_s, table[index].identifier);
        stream.writeTextElement(ns, u"waypoint-type"_s, table[index].type);
        stream.writeEndElement(); // route-point
    }
    stream.writeEndElement(); // route

    stream.writeEndElement(); // flight-plan
    stream.writeEndDocument();

    return result;
}
