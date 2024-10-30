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

#include <QXmlStreamReader>

#include "FPL.h"

#include "DataFileAbstract.h"
#include "Waypoint.h"


FileFormats::FPL::FPL(const QString& fileName)
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open FPL file %1 for reading.", "FileFormats::FPL").arg(fileName));
        return;
    }

    QXmlStreamReader xmlReader(file.data());
    if (!xmlReader.readNextStartElement())
    {
        setError(QObject::tr("Cannot parse FPL file %1 for XML.", "FileFormats::FPL").arg(fileName));
        return;
    }
    if (xmlReader.name().compare("flight-plan") != 0)
    {
        setError(QObject::tr("File %1 does not contain a flight plan", "FileFormats::FPL").arg(fileName));
        return;
    }

    int waypointTableCounter = 0;
    int routeCounter = 0;
    QMap<QString, QGeoCoordinate> waypointTable;
    while(xmlReader.readNextStartElement())
    {
        if(xmlReader.name().compare("waypoint-table", Qt::CaseInsensitive) == 0)
        {
            waypointTableCounter++;
            int count3 = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("waypoint", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                }
                else
                {
                    count3++;
                    QString m_name;
                    QGeoCoordinate m_coordinate;
                    while(xmlReader.readNextStartElement())
                    {
                        if(xmlReader.name().compare("identifier", Qt::CaseInsensitive) == 0)
                        {
                            if (!m_name.isEmpty())
                            {
                                setError(QObject::tr("Waypoint %1 has more than one identifier", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            m_name = xmlReader.readElementText();
                        }
                        else if(xmlReader.name().compare("lat", Qt::CaseInsensitive) == 0)
                        {
                            if (!std::isnan(m_coordinate.latitude()))
                            {
                                setError(QObject::tr("Waypoint %1 has more than one latitude", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            bool ok = false;
                            m_coordinate.setLatitude(xmlReader.readElementText().toDouble(&ok));
                            if (!ok)
                            {
                                setError(QObject::tr("Waypoint %1 does not have a valid latitude", "FileFormats::FPL").arg(count3));
                                return;
                            };
                        }
                        else if(xmlReader.name().compare("lon", Qt::CaseInsensitive) == 0)
                        {
                            if (!std::isnan(m_coordinate.longitude()))
                            {
                                setError(QObject::tr("Waypoint %1 has more than one longitude", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            bool ok = false;
                            m_coordinate.setLongitude(xmlReader.readElementText().toDouble(&ok));
                            if (!ok)
                            {
                                setError(QObject::tr("Waypoint %1 does not have a valid longitude", "FileFormats::FPL").arg(count3));
                                return;
                            };
                        }
                        else if(xmlReader.name().compare("elevation", Qt::CaseInsensitive) == 0)
                        {
                            if (!std::isnan(m_coordinate.altitude()))
                            {
                                setError(QObject::tr("Waypoint %1 has more than one elevation", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            bool ok = false;
                            m_coordinate.setAltitude(xmlReader.readElementText().toDouble(&ok));
                            if (!ok)
                            {
                                setError(QObject::tr("Waypoint %1 does not have a valid elevation", "FileFormats::FPL").arg(count3));
                                return;
                            };
                        }
                        else
                        {
                            xmlReader.skipCurrentElement();
                        }
                    }
                    if (m_name.isEmpty())
                    {
                        setError(QObject::tr("Waypoint %1 does not have an identifier", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    if (!m_coordinate.isValid())
                    {
                        setError(QObject::tr("Waypoint %1 does not have valid coordinats", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    waypointTable[m_name] = m_coordinate;
                }
            }
            if (count3 == 0)
            {
                setError(QObject::tr("File %1 does not contain a waypoint", "FileFormats::FPL").arg(fileName));
                return;
            }
        }
        else if (xmlReader.name().compare("route", Qt::CaseInsensitive) == 0)
        {
            routeCounter++;
            int count3 = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("route-point", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                }
                else
                {
                    count3++;
                    int count4 = 0;
                    QString id;
                    while(xmlReader.readNextStartElement())
                    {
                        if(xmlReader.name().compare("waypoint-identifier", Qt::CaseInsensitive) != 0)
                        {
                            xmlReader.skipCurrentElement();
                        }
                        else
                        {
                            count4++;
                            id = xmlReader.readElementText();
                        }
                    }
                    if (count4 != 1)
                    {
                        setError(QObject::tr("Route point %1 does not have a unique waypoint identifier", "FileFormats::FPL").arg(count3));
                        return;
                    }

                    if (waypointTable.contains(id))
                    {
                        m_waypoints.append(GeoMaps::Waypoint(waypointTable[id], id));
                    }
                    else
                    {
                        setError(QObject::tr("Waypoint identifier for route point %1 does not exist", "FileFormats::FPL").arg(count3));
                        return;
                    }

                }
            }
            if (count3 == 0)
            {
                setError(QObject::tr("File %1 does not contain a waypoint", "FileFormats::FPL").arg(fileName));
                return;
            }
        }
        else
        {
            xmlReader.skipCurrentElement();
        }
    }
    if (waypointTableCounter != 1)
    {
        setError(QObject::tr("File %1 does not contain a waypoint", "FileFormats::FPL").arg(fileName));
        return;
    }
    if (routeCounter != 1)
    {
        setError(QObject::tr("File %1 does not contain a route", "FileFormats::FPL").arg(fileName));
        return;
    }
}


