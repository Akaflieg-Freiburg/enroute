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

#include "fileFormats/MapURL.h"

using namespace Qt::Literals::StringLiterals;


namespace {
Q_GLOBAL_STATIC(QRegularExpression, regexGoogleMap1, u"@(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"_s)
Q_GLOBAL_STATIC(QRegularExpression, regexGoogleMap2, u"!3d(-?\\d+\\.\\d+)!4d(-?\\d+\\.\\d+)"_s)
Q_GLOBAL_STATIC(QRegularExpression, regexOpenStreetMap, u"#map=\\d+/(-?\\d+\\.\\d+)/(-?\\d+\\.\\d+)"_s)
Q_GLOBAL_STATIC(QRegularExpression, regexWeGo1, u"map=(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"_s)
Q_GLOBAL_STATIC(QRegularExpression, regexWeGo2, u"l/(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"_s)
Q_GLOBAL_STATIC(QRegularExpression, regexGeo, u"geo:(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"_s)
} // namespace


//
// Constructor
//


FileFormats::MapURL::MapURL(const QString& urlName)
{
    {
        // Try to parse a Google Map URL
        auto match = regexGoogleMap1->match(urlName);
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
        // Try to parse a Google Map URL
        auto match = regexGoogleMap2->match(urlName);
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
        auto match = regexWeGo1->match(urlName);
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
        auto match = regexWeGo2->match(urlName);
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
        // Try to parse a geo: URL
        auto match = regexGeo->match(urlName);
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
