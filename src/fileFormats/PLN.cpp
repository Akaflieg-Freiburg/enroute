/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#include "PLN.h"

#include "DataFileAbstract.h"


// Converts a string of the form N53° 53' 55.87" or W166° 32' 40.78" to a decimal. Returns NAN on failure.
double convertPDMSToDecimal(QString pdms)
{
    QStringList split = pdms.split(u" "_s);
    if (split.size() != 3)
    {
        return NAN;
    }

    bool ok = false;
    auto degree = split[0].chopped(1).remove(0, 1).toInt(&ok);
    if (!ok)
    {
        return NAN;
    }
    auto minute = split[1].chopped(1).toInt(&ok);
    if (!ok)
    {
        return NAN;
    }
    auto second = split[2].chopped(1).toDouble(&ok);
    if (!ok)
    {
        return NAN;
    }

    auto decimal = degree + (minute / 60.0) + (second / 3600);

    auto pol = pdms[0].toUpper();
    if ((pol == 'S') || (pol == 'W'))
    {
        return -decimal;
    }
    return decimal;
}


FileFormats::PLN::PLN(const QString& fileName)
{
    // Open file and open QXmlStreamReader
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open PLN file %1 for reading.", "FileFormats::PLN").arg(fileName));
        return;
    }
    QXmlStreamReader xmlReader(file.data());
    if (!xmlReader.readNextStartElement())
    {
        setError(QObject::tr("Cannot parse PLN file %1 for XML.", "FileFormats::PLN").arg(fileName));
        return;
    }
    if (xmlReader.name().compare("SimBase.Document", Qt::CaseInsensitive) != 0)
    {
        setError(QObject::tr("File %1 does not contain a flight plan.", "FileFormats::PLN").arg(fileName));
        return;
    }


    int flightPlanCounter = 0;
    while(xmlReader.readNextStartElement())
    {
        if(xmlReader.name().compare("FlightPlan.FlightPlan", Qt::CaseInsensitive) != 0)
        {
            xmlReader.skipCurrentElement();
            continue;
        }
        flightPlanCounter++;

        int atcWaypointCounter = 0;
        while(xmlReader.readNextStartElement())
        {
            if(xmlReader.name().compare("ATCWaypoint", Qt::CaseInsensitive) != 0)
            {
                xmlReader.skipCurrentElement();
                continue;
            }
            atcWaypointCounter++;

            auto id = xmlReader.attributes().value(u"id"_s).toString();
            int worldPositionCounter = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("WorldPosition", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                    continue;
                }
                worldPositionCounter++;

                auto pos = xmlReader.readElementText();
                auto split = pos.split(u","_s);
                if (split.size() != 3)
                {
                    setError(QObject::tr("Position of waypoint %1 is invalid.", "FileFormats::PLN").arg(atcWaypointCounter));
                    return;
                }
                bool ok = false;
                QGeoCoordinate coord;
                coord.setLatitude(convertPDMSToDecimal(split[0]));
                coord.setLongitude(convertPDMSToDecimal(split[1]));
                coord.setAltitude(split[2].toDouble(&ok));
                if (!ok || !coord.isValid())
                {
                    setError(QObject::tr("Position of waypoint %1 is invalid.", "FileFormats::PLN").arg(atcWaypointCounter));
                    return;
                }
                m_waypoints.append(GeoMaps::Waypoint(coord, id));
            }
            if (worldPositionCounter != 1)
            {
                setError(QObject::tr("More than one position specified for waypoint %1.", "FileFormats::PLN").arg(atcWaypointCounter));
                return;
            }
        }

        if (atcWaypointCounter == 0)
        {
            setError(QObject::tr("File %1 does not contain waypoints.", "FileFormats::PLN").arg(fileName));
            return;
        }


    }
    if (flightPlanCounter != 1)
    {
        setError(QObject::tr("File %1 contains more than one flight plan.", "FileFormats::PLN").arg(fileName));
        return;
    }
}


