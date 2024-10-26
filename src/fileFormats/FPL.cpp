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
#include <QList>
#include <QTextStream>
#include <qxmlstream.h>
#include <qnamespace.h>
#include <qglobal.h>
#include <qgeocoordinate.h>
#include <qobject.h>

#include "FPL.h"

#include "DataFileAbstract.h"




FileFormats::FPL::FPL(const QString& fileName)
{
    struct Wpoint
    {
        QString id;
        double lon {};
        double lat {};
        double elev {};

        auto operator==(const Wpoint& wpoint) const
        {
            return id == wpoint.id;
        }

        void init()
        {
            id = nullptr;
            lat = NULL;
            lon = NULL;
            elev = NULL;
        }

        Wpoint() =default;
    } wpoint;
    QList<Wpoint> wplist;

    bool ok = false;
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    QGeoCoordinate waypoint;
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
    count1 = 0;
    count2 = 0;
    while(xmlReader.readNextStartElement())
    {
        if(xmlReader.name().compare("waypoint-table", Qt::CaseInsensitive) == 0)
        {
            count1++;
            count3 = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("waypoint", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                }
                else
                {
                    count3++;
                    wpoint.init();
                    while(xmlReader.readNextStartElement())
                    {
                        if(xmlReader.name().compare("identifier", Qt::CaseInsensitive) == 0)
                        {
                            if (!wpoint.id.isNull())
                            {
                                setError(QObject::tr("Waypoint %1 has more than one identifier", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            wpoint.id = xmlReader.readElementText();
                        }
                        else if(xmlReader.name().compare("lat", Qt::CaseInsensitive) == 0)
                        {
                            if (wpoint.lat != NULL)
                            {
                                setError(QObject::tr("Waypoint %1 has more than one latitude", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            wpoint.lat = xmlReader.readElementText().toDouble(&ok);
                            if (!ok)
                            {
                                setError(QObject::tr("Waypoint %1 does not have a valid latitude", "FileFormats::FPL").arg(count3));
                                return;
                            };
                        }
                        else if(xmlReader.name().compare("lon", Qt::CaseInsensitive) == 0)
                        {
                            if (wpoint.lon != NULL)
                            {
                                setError(QObject::tr("Waypoint %1 has more than one longitude", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            wpoint.lon = xmlReader.readElementText().toDouble(&ok);
                            if (!ok)
                            {
                                setError(QObject::tr("Waypoint %1 does not have a valid longitude", "FileFormats::FPL").arg(count3));
                                return;
                            };
                        }
                        else if(xmlReader.name().compare("elevation", Qt::CaseInsensitive) == 0)
                        {
                            if (wpoint.elev != NULL)
                            {
                                setError(QObject::tr("Waypoint %1 has more than one elevation", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            wpoint.elev = xmlReader.readElementText().toDouble(&ok);
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
                    if (wpoint.id.isNull())
                    {
                        setError(QObject::tr("Waypoint %1 does not have an identifier", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    if (wpoint.lat == NULL)
                    {
                        setError(QObject::tr("Waypoint %1 does not have a latitude", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    if (wpoint.lon == NULL)
                    {
                        setError(QObject::tr("Waypoint %1 does not have a longitude", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    wplist.append(wpoint);
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
            count2++;
            count3 = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("route-point", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                }
                else
                {
                    count3++;
                    count4 = 0;
                    wpoint.init();
                    while(xmlReader.readNextStartElement())
                    {
                        if(xmlReader.name().compare("waypoint-identifier", Qt::CaseInsensitive) != 0)
                        {
                            xmlReader.skipCurrentElement();
                        }
                        else
                        {
                            count4++;
                            if (!wpoint.id.isNull())
                            {
                                setError(QObject::tr("Waypoint %1 has more than one identifier", "FileFormats::FPL").arg(count3));
                                return;
                            }
                            wpoint.id = xmlReader.readElementText();
                        }
                    }
                    if (count4 != 1)
                    {
                        setError(QObject::tr("Route point %1 does not have a unique waypoint identifier", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    auto index = wplist.indexOf(wpoint, 0);
                    if (index < 0)
                    {
                        setError(QObject::tr("Waypoint identifier for route point %1 does not exist", "FileFormats::FPL").arg(count3));
                        return;
                    }
                    waypoint.setLatitude(wplist.at(index).lat);
                    waypoint.setLongitude(wplist.at(index).lon);
                    waypoint.setAltitude(wplist.at(index).elev);
                    m_waypoints.append(waypoint);
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
    if (count1 != 1)
    {
        setError(QObject::tr("File %1 does not contain a waypoint", "FileFormats::FPL").arg(fileName));
        return;
    }
    if (count2 != 1)
    {
        setError(QObject::tr("File %1 does not contain a route", "FileFormats::FPL").arg(fileName));
        return;
    }
}


