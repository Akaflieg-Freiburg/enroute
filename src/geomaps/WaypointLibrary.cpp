/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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
#include <QXmlStreamWriter>

#include "Librarian.h"
#include "geomaps/WaypointLibrary.h"

GeoMaps::WaypointLibrary::WaypointLibrary(QObject *parent)
    : GlobalObject(parent)
{
    (void)loadFromGeoJSON();
    connect(this, &GeoMaps::WaypointLibrary::waypointsChanged, this, [this]()
            { (void)save(); });
}

//
// Methods
//

void GeoMaps::WaypointLibrary::add(const GeoMaps::Waypoint &waypoint)
{
    if (!waypoint.isValid())
    {
        return;
    }

    m_waypoints.append(waypoint);
    std::sort(m_waypoints.begin(), m_waypoints.end(), [](const Waypoint &a, const Waypoint &b)
              { return a.name() < b.name(); });
    emit waypointsChanged();
}

void GeoMaps::WaypointLibrary::clear()
{
    if (m_waypoints.isEmpty())
    {
        return;
    }

    m_waypoints.clear();
    emit waypointsChanged();
}

QVector<GeoMaps::Waypoint> GeoMaps::WaypointLibrary::filteredWaypoints(const QString &filter) const
{
    QVector<GeoMaps::Waypoint> result;

    QString simplifiedFilter = GlobalObject::librarian()->simplifySpecialChars(filter);
    foreach (auto waypoint, m_waypoints)
    {
        auto simplifiedName = GlobalObject::librarian()->simplifySpecialChars(waypoint.name());
        if (simplifiedName.contains(simplifiedFilter, Qt::CaseInsensitive))
        {
            result.append(waypoint);
        }
    }
    return result;
}

bool GeoMaps::WaypointLibrary::hasNearbyEntry(const GeoMaps::Waypoint &waypoint) const
{
    for (const auto &wp : qAsConst(m_waypoints))
    {
        if (wp.isNear(waypoint))
        {
            return true;
        }
    }
    return false;
}

QStringList parseCSV(const QString& string)
{
    // Thanks to https://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt

    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        const QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                state = Quote;
                value += current;
            }

            // Other character
            else
                value += current;
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i < string.size())
                {
                    // A double double-quote?
                    if (i+1 < string.size() && string.at(i+1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        state = Normal;
                        value += '"';
                    }
                }
            }

            // Other character
            else
                value += current;
        }
    }

    if (!value.isEmpty())
        fields.append(value.trimmed());

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The outermost quotes are removed here.
    for (int i=0; i<fields.size(); ++i)
        if (fields[i].length()>=1 && fields[i].left(1)=='"')
        {
            fields[i]=fields[i].mid(1);
            if (fields[i].length()>=1 && fields[i].right(1)=='"')
                fields[i]=fields[i].left(fields[i].length()-1);
        }

    return fields;
}

auto GeoMaps::WaypointLibrary::importCUP(const QString& fileName) -> QString
{
    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
    {
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    }

    QTextStream stream(&file);
    QString line;
    while (stream.readLineInto(&line))
    {
        if (line.contains("-----Related Tasks-----"))
        {
            break;
        }

        auto fields = parseCSV(line);

#warning implement

    }

    return "not implemented";
}

auto GeoMaps::WaypointLibrary::loadFromGeoJSON(QString fileName) -> QString
{
    if (fileName.isEmpty())
    {
        fileName = stdFileName;
    }

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
    {
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    }
    auto fileContent = file.readAll();
    if (fileContent.isEmpty())
    {
        return tr("Cannot read data from file '%1'.").arg(fileName);
    }
    file.close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return tr("Cannot parse file '%1'. Reason: %2.").arg(fileName, parseError.errorString());
    }

    QVector<GeoMaps::Waypoint> newWaypoints;
    foreach (auto value, document.object()[QStringLiteral("features")].toArray())
    {
        auto wp = GeoMaps::Waypoint(value.toObject());
        if (!wp.isValid())
        {
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
        newWaypoints.append(wp);
    }

    m_waypoints = newWaypoints;
    emit waypointsChanged();

    return {};
}

bool GeoMaps::WaypointLibrary::remove(const GeoMaps::Waypoint &waypoint)
{
    if (m_waypoints.removeOne(waypoint))
    {
        emit waypointsChanged();
        return true;
    }
    return false;
}

bool GeoMaps::WaypointLibrary::replace(const GeoMaps::Waypoint &oldWaypoint, const GeoMaps::Waypoint &newWaypoint)
{
    if (!newWaypoint.isValid())
    {
        return false;
    }

    if (m_waypoints.removeOne(oldWaypoint))
    {
        m_waypoints.append(newWaypoint);
        std::sort(m_waypoints.begin(), m_waypoints.end(), [](const Waypoint &a, const Waypoint &b)
                  { return a.name() < b.name(); });
        emit waypointsChanged();
        return true;
    }
    return false;
}

auto GeoMaps::WaypointLibrary::save(QString fileName) const -> QString
{
    if (fileName.isEmpty())
    {
        fileName = stdFileName;
    }

    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success)
    {
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    }
    auto numBytesWritten = file.write(toGeoJSON());
    if (numBytesWritten == -1)
    {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1'.").arg(fileName);
    }
    file.close();
    return {};
}

auto GeoMaps::WaypointLibrary::toGeoJSON() const -> QByteArray
{
    QJsonArray waypointArray;
    foreach (auto waypoint, m_waypoints)
    {
        if (waypoint.isValid())
        {
            waypointArray.append(waypoint.toJSON());
        }
    }
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("enroute"), QStringLiteral("waypoint library"));
    jsonObj.insert(QStringLiteral("features"), waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

auto GeoMaps::WaypointLibrary::toGpx() const -> QByteArray
{
    QByteArray result;

    QXmlStreamWriter stream(&result);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement(QStringLiteral("gpx"));
    stream.writeAttribute(QStringLiteral("version"), QStringLiteral("1.1"));
    stream.writeAttribute(QStringLiteral("creator"), QStringLiteral("Enroute Flight Navigation"));
    stream.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://www.topografix.com/GPX/1/1"));
    stream.writeAttribute(QStringLiteral("xmlns:xsi"), QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"));

    stream.writeStartElement(QStringLiteral("metadata"));
    stream.writeTextElement(QStringLiteral("name"), QStringLiteral("Waypoint Library"));
    stream.writeEndElement(); // metadata

    for(const auto& _waypoint : m_waypoints)
    {
        _waypoint.toGPX(stream);
    }

    stream.writeEndElement(); // gpx
    stream.writeEndDocument();

    return result;
}
