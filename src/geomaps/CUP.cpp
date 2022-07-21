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

#include "geomaps/CUP.h"

//
// Private helper functions
//

QStringList GeoMaps::CUP::parseCSV(const QString& string)
{
    // Thanks to https://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt

    enum State
    {
        Normal,
        Quote
    } state = Normal;
    QStringList fields;
    fields.reserve(10);
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
            {
                value += current;
            }
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
                    if (i + 1 < string.size() && string.at(i + 1) == '"')
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
            {
                value += current;
            }
        }
    }

    if (!value.isEmpty())
    {
        fields.append(value.trimmed());
    }

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The outermost quotes are removed here.
    for (int i = 0; i < fields.size(); ++i)
    {
        if (fields[i].length() >= 1 && fields[i].at(0) == '"')
        {
            fields[i] = fields[i].mid(1);
            if (fields[i].length() >= 1 && fields[i].right(1) == '"')
            {
                fields[i] = fields[i].left(fields[i].length() - 1);
            }
        }
    }

    return fields;
}

GeoMaps::Waypoint GeoMaps::CUP::readWaypoint(const QString &line)
{
    auto fields = parseCSV(line);
    if (fields.size() < 6)
    {
        return {};
    }

    // Get Name
    auto name = fields[0];

    // Get Latitude
    double lat;
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
    double lon;
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

    double ele;
    {
        auto eleString = fields[5];
        bool ok = false;
        if (eleString.endsWith(QLatin1String("m")))
        {
            ele = eleString.chopped(1).toDouble(&ok);
        }
        if (eleString.endsWith(QLatin1String("ft")))
        {
            ele = eleString.chopped(1).toDouble(&ok) * 0.3048;
        }
        if (!ok)
        {
            return {};
        }
    }

    return GeoMaps::Waypoint(QGeoCoordinate(lat, lon, ele)).renamed(name);
}


//
// Methods
//

bool GeoMaps::CUP::isValid(const QString &fileName)
{

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
    {
        return {};
    }

    QTextStream stream(&file);
    QString line;
    stream.readLineInto(&line);
    stream.readLineInto(&line);
    return readWaypoint(line).isValid();
}

auto GeoMaps::CUP::read(const QString &fileName) -> QVector<GeoMaps::Waypoint>
{
    QVector<GeoMaps::Waypoint> result;

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
    {
        return {};
    }

    QTextStream stream(&file);
    QString line;
    stream.readLineInto(&line);
    while (stream.readLineInto(&line))
    {
        if (line.contains(QLatin1String("-----Related Tasks-----")))
        {
            break;
        }
        auto wp = readWaypoint(line);
        if (!wp.isValid())
        {
            return {};
        }
        result << wp;
    }
    return result;
}
