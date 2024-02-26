/***************************************************************************
 *   Copyright (C) 2022-2024 by Stefan Kebekus                             *
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

#include "fileFormats/MapURL.h"

namespace {
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, regexBingMap, ("cp=(-?\\d+\\.\\d+)%7E(-?\\d+\\.\\d+)"))
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, regexGoogleMap, ("@(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"))
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, regexOpenStreetMap, ("#map=\\d+/(\\d+\\.\\d+)/(\\d+\\.\\d+)"))
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, regexWeGo, ("map=(\\d+\\.\\d+),(\\d+\\.\\d+)"))
}


//
// Constructor
//


FileFormats::MapURL::MapURL(const QString& urlName)
{
    {
        // Try to parse a Google Map URL
        auto match = regexGoogleMap->match(urlName);
        if (match.hasMatch()) {
            auto latitude = match.captured(1);
            auto longitude = match.captured(2);
            GeoMaps::Waypoint const waypoint({latitude.toDouble(), longitude.toDouble()});
            if (waypoint.isValid())
            {
                m_waypoint = waypoint;
                return;
            }
        }
    }

    {
        // Try to parse a Bing Map URL
        auto match = regexBingMap->match(urlName);
        if (match.hasMatch()) {
            auto latitude = match.captured(1);
            auto longitude = match.captured(2);
            GeoMaps::Waypoint const waypoint({latitude.toDouble(), longitude.toDouble()});
            if (waypoint.isValid())
            {
                m_waypoint = waypoint;
                return;
            }
        }
    }

    {
        // Try to parse an openStreetMap URL
        auto match = regexOpenStreetMap->match(urlName);
        if (match.hasMatch()) {
            auto latitude = match.captured(1);
            auto longitude = match.captured(2);
            GeoMaps::Waypoint const waypoint({latitude.toDouble(), longitude.toDouble()});
            if (waypoint.isValid())
            {
                m_waypoint = waypoint;
                return;
            }
        }
    }

    {
        // Try to parse a WeGO URL
        auto match = regexWeGo->match(urlName);
        if (match.hasMatch()) {
            auto latitude = match.captured(1);
            auto longitude = match.captured(2);
            GeoMaps::Waypoint const waypoint({latitude.toDouble(), longitude.toDouble()});
            if (waypoint.isValid())
            {
                m_waypoint = waypoint;
                return;
            }
        }
    }

    setError(QObject::tr("Cannot extract valid coordinates from the URL.", "FileFormats::CSV"));
}
