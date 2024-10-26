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
#include <QTextStream>
#include <qxmlstream.h>
#include <qnamespace.h>
#include <qglobal.h>
#include <qgeocoordinate.h>
#include <qobject.h>

#include "PLN.h"

#include "DataFileAbstract.h"



auto convertPDMSToDecimal(QString pdms) ->double
{
    bool ok = false;
    QStringList split = pdms.split(" ");
    if (split.size() != 3)
    {
        throw 1;
    }
    int const degree = split[0].chopped(1).remove(0, 1).toInt(&ok);
    if (!ok)
    {
        throw 2;
    }
    int const minute = split[1].chopped(1).toInt(&ok);
    if (!ok)
    {
        throw 3;
    }
    double const second = split[2].chopped(1).toDouble(&ok);
    if (!ok)
    {
        throw 4;
    }

    double const decimal = degree + (minute / 60.0) + (second / 3600);

    QChar const pol = pdms[0].toUpper();

    if ((pol == 'S') || (pol == 'W'))
    {
        return -decimal;
    }
    return decimal;
}


FileFormats::PLN::PLN(const QString& fileName)
{
    bool ok = false;
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    QString pos;
    QGeoCoordinate waypoint;
    QStringList split;
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
        setError(QObject::tr("File %1 does not contain a flight plan", "FileFormats::PLN").arg(fileName));
        return;
    }
    count1 = 0;
    while(xmlReader.readNextStartElement())
    {
        if(xmlReader.name().compare("FlightPlan.FlightPlan", Qt::CaseInsensitive) != 0)
        {
            xmlReader.skipCurrentElement();
        }
        else
        {
            count1++;
            count2 = 0;
            while(xmlReader.readNextStartElement())
            {
                if(xmlReader.name().compare("ATCWaypoint", Qt::CaseInsensitive) != 0)
                {
                    xmlReader.skipCurrentElement();
                }
                else
                {
                    count2++;
                    count3 = 0;
                    while(xmlReader.readNextStartElement())
                    {
                        if(xmlReader.name().compare("WorldPosition", Qt::CaseInsensitive) != 0)
                        {
                            xmlReader.skipCurrentElement();
                        }
                        else
                        {
                            count3++;
                            pos = xmlReader.readElementText();
                            split = pos.split(",");
                            try
                            {
                                if (split.size() != 3)
                                {
                                    throw 1;
                                }
                                waypoint.setLatitude(convertPDMSToDecimal(split[0]));
                                waypoint.setLongitude(convertPDMSToDecimal(split[1]));
                                waypoint.setAltitude(split[2].toDouble(&ok));
                                if (!ok)
                                {
                                    throw 5;
                                }
                                m_waypoints.append(waypoint);
                            }
                            catch (...)
                            {
                                addWarning(QObject::tr("Position of waypoint %1 is not a valid position", "FileFormats::PLN").arg(count2));
                            }
                        }
                    }
                    if (count3 != 1)
                    {
                        setError(QObject::tr("Waypoint %1 does not have a unique position", "FileFormats::PLN").arg(count2));
                        return;
                    }
                }
            }
            if (count2 == 0)
            {
                setError(QObject::tr("File %1 does not contain way points", "FileFormats::PLN").arg(fileName));
                return;
            }
        }
    }
    if (count1 != 1)
    {
        setError(QObject::tr("File %1 does not contain a unique flight plan", "FileFormats::PLN").arg(fileName));
        return;
    }
}


