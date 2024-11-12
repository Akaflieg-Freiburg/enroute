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
#include <cmath>

#include "fileFormats/CSV.h"
#include "fileFormats/CUP.h"

using namespace Qt::Literals::StringLiterals;


//
// Private helper functions
//

GeoMaps::Waypoint FileFormats::CUP::readWaypoint(const QStringList& fields)
{
    if (fields.size() < 6)
    {
        return {};
    }

    // Get Name
    const auto &name = fields[0];

    // Get Latitude
    double lat = NAN;
    {
        auto latString = fields[3];
        if (latString.size() != 9)
        {
            return {};
        }
        if ((latString[8] != 'N') && (latString[8] != 'S'))
        {
            return {};
        }
        bool ok = false;
        lat = latString.left(2).toDouble(&ok);
        if (!ok)
        {
            return {};
        }
        lat = lat + latString.mid(2, 6).toDouble(&ok) / 60.0;
        if (!ok)
        {
            return {};
        }
        if (latString[8] == 'S')
        {
            lat = -lat;
        }
    }

    // Get Longitude
    double lon = NAN;
    {
        auto longString = fields[4];
        if (longString.size() != 10)
        {
            return {};
        }
        if ((longString[9] != 'W') && (longString[9] != 'E'))
        {
            return {};
        }
        bool ok = false;
        lon = longString.left(3).toDouble(&ok);
        if (!ok)
        {
            return {};
        }
        lon = lon + longString.mid(3, 6).toDouble(&ok) / 60.0;
        if (!ok)
        {
            return {};
        }
        if (longString[9] == 'W')
        {
            lon = -lon;
        }
    }

    double ele = NAN;
    {
        const auto &eleString = fields[5];
        bool ok = false;
        if (eleString.endsWith(u"m"))
        {
            ele = eleString.chopped(1).toDouble(&ok);
        }
        if (eleString.endsWith(u"ft"))
        {
            ele = eleString.chopped(1).toDouble(&ok) * 0.3048;
        }
        if (!ok)
        {
            return {};
        }
    }

    // Get additional information
    QStringList notes;
    if ((fields.size() >= 8) && (!fields[7].isEmpty()))
    {
        notes += QObject::tr("Direction: %1°", "GeoMaps::CUP").arg(fields[7]);
    }
    if ((fields.size() >= 9) && (!fields[8].isEmpty()))
    {
        notes += QObject::tr("Length: %1", "GeoMaps::CUP").arg(fields[8]);
    }
    if ((fields.size() >= 11) && (!fields[10].isEmpty()))
    {
        notes += fields[10];
    }
    if ((fields.size() >= 12) && (!fields[11].isEmpty()))
    {
        notes += fields[11];
    }

    GeoMaps::Waypoint result(QGeoCoordinate(lat, lon, ele));
    result.setName(name);
    if (!notes.isEmpty())
    {
        result.setNotes(notes.join(u" • "_s));
    }
    return result;
}


FileFormats::CUP::CUP(const QString& fileName)
{
    CSV const csv(fileName);

    if (!csv.isValid())
    {
        setError(csv.error());
        return;
    }

    int lineNumber = 0;
    foreach (auto& line, csv.lines())
    {
        lineNumber++;
        if (line.contains(u"-----Related Tasks-----"))
        {
            break;
        }
        auto waypoint = readWaypoint(line);
        if (!waypoint.isValid())
        {
            setError(QObject::tr("Error reading line %1 in the CUP file %2.", "FileFormats::CUP").arg(lineNumber).arg(fileName));
            return;
        }
        m_waypoints << waypoint;
    }
}
