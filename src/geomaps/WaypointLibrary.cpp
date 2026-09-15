/***************************************************************************
 *   Copyright (C) 2022-2026 by Stefan Kebekus                             *
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
#include <algorithm>
#include <iterator>

#include "fileFormats/CUP.h"
#include "fileFormats/DataFileAbstract.h"
#include "fileFormats/FPL.h"
#include "fileFormats/PLN.h"
#include "geomaps/GPX.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/WaypointLibrary.h"

GeoMaps::WaypointLibrary::WaypointLibrary(QObject *parent)
    : QAbstractListModel(parent)
{
    // Load first and connect afterwards, so that the initial load does not
    // write the file back.
    (void)loadFromGeoJSON();
    connect(this, &GeoMaps::WaypointLibrary::waypointsChanged, this, [this]() { (void)save(); });
}


//
// Getter Methods
//

QByteArray GeoMaps::WaypointLibrary::GeoJSON() const
{
    QJsonArray waypointArray;
    foreach (const auto& waypoint, m_waypoints)
    {
        if (waypoint.isValid())
        {
            waypointArray.append(waypoint.toJSON());
        }
    }

    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("enroute"), GeoMaps::GeoJSON::indicatorWaypointLibrary());
    jsonObj.insert(QStringLiteral("features"), waypointArray);

    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


//
// Model API
//

int GeoMaps::WaypointLibrary::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_waypoints.size());
}


QVariant GeoMaps::WaypointLibrary::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() < 0) || (index.row() >= m_waypoints.size()))
    {
        return {};
    }

    auto const& waypoint = m_waypoints.at(index.row());
    switch (role)
    {
    case WaypointRole:
        return QVariant::fromValue(waypoint);
    case NameRole:
    case Qt::DisplayRole:
        return waypoint.name();
    default:
        return {};
    }
}


QHash<int, QByteArray> GeoMaps::WaypointLibrary::roleNames() const
{
    return {{WaypointRole, "waypoint"}, {NameRole, "name"}};
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

    auto const row = insertionRow(waypoint.name());
    beginInsertRows({}, row, row);
    m_waypoints.insert(row, waypoint);
    endInsertRows();
    emit waypointsChanged();
}


void GeoMaps::WaypointLibrary::clear()
{
    if (m_waypoints.isEmpty())
    {
        return;
    }

    beginRemoveRows({}, 0, static_cast<int>(m_waypoints.size()) - 1);
    m_waypoints.clear();
    endRemoveRows();
    emit waypointsChanged();
}


bool GeoMaps::WaypointLibrary::hasNearbyEntry(const GeoMaps::Waypoint &waypoint) const
{
    for (const auto &wp :  std::as_const(m_waypoints))
    {
        if (wp.isNear(waypoint))
        {
            return true;
        }
    }
    return false;
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

    QList<GeoMaps::Waypoint> newWaypoints;
    const auto features = document.object()[QStringLiteral("features")].toArray();
    for (const auto value : features)
    {
        auto wp = GeoMaps::Waypoint(value.toObject());
        if (!wp.isValid())
        {
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
        newWaypoints.append(wp);
    }
    sortByName(newWaypoints);

    beginResetModel();
    m_waypoints = newWaypoints;
    endResetModel();
    emit waypointsChanged();

    return {};
}


auto GeoMaps::WaypointLibrary::import(const QString& fileName, bool skip) -> QString
{
    auto result = FileFormats::CUP(fileName).waypoints();
    if (result.isEmpty())
    {
        result = GeoMaps::GeoJSON::read(fileName);
    }
    if (result.isEmpty())
    {
        result = GeoMaps::GPX::read(fileName);
    }
    if (result.isEmpty())
    {
        auto pln = FileFormats::PLN(fileName);
        result.reserve(pln.waypoints().size());
        for(const auto& coordinate : pln.waypoints())
        {
            result += coordinate;
        }
    }
    if (result.isEmpty())
    {
        auto fpl = FileFormats::FPL(fileName);
        result.reserve(fpl.waypoints().size());
        for(const auto& coordinate : fpl.waypoints())
        {
            result += coordinate;
        }
    }
    if (result.isEmpty())
    {
        return tr("Error reading waypoints from file '%1'.").arg(fileName);
    }

    auto newWaypoints = m_waypoints;
    if (skip)
    {
        foreach(const auto& newWaypoint, result)
        {
            bool skipWaypoint = false;
            foreach(const auto& existingWaypoint, newWaypoints)
            {
                if (newWaypoint.isNear(existingWaypoint))
                {
                    skipWaypoint = true;
                    break;
                }
            }
            if (!skipWaypoint)
            {
                newWaypoints.append(newWaypoint);
            }
        }
    }
    else
    {
        newWaypoints += result;
    }
    sortByName(newWaypoints);

    beginResetModel();
    m_waypoints = newWaypoints;
    endResetModel();
    emit waypointsChanged();
    return {};
}


bool GeoMaps::WaypointLibrary::remove(const GeoMaps::Waypoint &waypoint)
{
    auto const row = static_cast<int>(m_waypoints.indexOf(waypoint));
    if (row < 0)
    {
        return false;
    }

    beginRemoveRows({}, row, row);
    m_waypoints.removeAt(row);
    endRemoveRows();
    emit waypointsChanged();
    return true;
}


bool GeoMaps::WaypointLibrary::replace(const GeoMaps::Waypoint& oldWaypoint, const GeoMaps::Waypoint& newWaypoint)
{
    if (!newWaypoint.isValid())
    {
        return false;
    }

    auto const row = static_cast<int>(m_waypoints.indexOf(oldWaypoint));
    if (row < 0)
    {
        return false;
    }

    if (oldWaypoint.name() == newWaypoint.name())
    {
        // The sorted position does not change
        m_waypoints[row] = newWaypoint;
        auto const idx = index(row);
        emit dataChanged(idx, idx, {WaypointRole, NameRole});
    }
    else
    {
        beginRemoveRows({}, row, row);
        m_waypoints.removeAt(row);
        endRemoveRows();

        auto const newRow = insertionRow(newWaypoint.name());
        beginInsertRows({}, newRow, newRow);
        m_waypoints.insert(newRow, newWaypoint);
        endInsertRows();
    }
    emit waypointsChanged();
    return true;
}


auto GeoMaps::WaypointLibrary::save(QString fileName) const -> QString
{
    if (fileName.isEmpty())
    {
        fileName = stdFileName;
    }

    QString error;
    if (!FileFormats::DataFileAbstract::saveFileAtomically(fileName, GeoJSON(), &error))
    {
        return tr("Unable to write to file '%1': %2").arg(fileName, error);
    }
    return {};
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


//
// Private Methods
//

int GeoMaps::WaypointLibrary::insertionRow(const QString& name) const
{
    auto const it = std::ranges::upper_bound(m_waypoints, name, {}, &GeoMaps::Waypoint::name);
    return static_cast<int>(std::distance(m_waypoints.cbegin(), it));
}


void GeoMaps::WaypointLibrary::sortByName(QList<GeoMaps::Waypoint>& waypoints)
{
    std::ranges::stable_sort(waypoints, {}, &GeoMaps::Waypoint::name);
}
