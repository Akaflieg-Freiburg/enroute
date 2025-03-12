/***************************************************************************
 *   Copyright (C) 2022-2025 by Stefan Kebekus                             *
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
#include <QJsonParseError>

#include "fileFormats/DataFileAbstract.h"
#include "geomaps/GeoJSON.h"

//
// Methods
//

GeoMaps::GeoJSON::fileContent GeoMaps::GeoJSON::inspect(const QString& fileName)
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!file->open(QIODevice::ReadOnly))
    {
        return GeoMaps::GeoJSON::invalid;
    }
    auto fileContent = file->readAll();
    if (fileContent.isEmpty())
    {
        return GeoMaps::GeoJSON::invalid;
    }
    file->close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return GeoMaps::GeoJSON::invalid;
    }

    auto features = document.object()[QStringLiteral("features")].toArray();
    if (features.isEmpty())
    {
        return GeoMaps::GeoJSON::invalid;
    }

    auto wp = GeoMaps::Waypoint(features[0].toObject());
    if (!wp.isValid())
    {
        return GeoMaps::GeoJSON::invalid;
    }

    auto typeString = document.object()[QStringLiteral("enroute")].toString();
    if (typeString == indicatorFlightRoute())
    {
        return GeoMaps::GeoJSON::flightRoute;
    }
    if (typeString == indicatorWaypointLibrary())
    {
        return GeoMaps::GeoJSON::waypointLibrary;
    }
    return GeoMaps::GeoJSON::valid;
}


auto GeoMaps::GeoJSON::read(const QString &fileName) -> QVector<GeoMaps::Waypoint>
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!file->open(QIODevice::ReadOnly))
    {
        return {};
    }
    auto fileContent = file->readAll();
    if (fileContent.isEmpty())
    {
        return {};
    }
    file->close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return {};
    }

    QVector<GeoMaps::Waypoint> result;
    const auto features = document.object()[QStringLiteral("features")].toArray();
    for(const auto value : features)
    {
        auto wp = GeoMaps::Waypoint(value.toObject());
        if (!wp.isValid())
        {
            return {};
        }
        result.append(wp);
    }

    return result;
}
