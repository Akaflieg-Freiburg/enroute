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
#include <QDomDocument>

#include "PLN.h"

#include "DataFileAbstract.h"



double convertPDMSToDecimal(QString pdms)
{
    bool ok;
    QStringList split = pdms.split(" ");
    if (split.size() != 3)
    {
        throw 1;
    }
    int d = split[0].chopped(1).remove(0, 1).toInt(&ok);
    if (!ok)
    {
        throw 2;
    }
    int m = split[1].chopped(1).toInt(&ok);
    if (!ok)
    {
        throw 3;
    }
    double s = split[2].chopped(1).toDouble(&ok);
    if (!ok)
    {
        throw 4;
    }

    double decimal = d + (m / 60.0) + (s / 3600);

    QChar p = pdms[0].toUpper();

    if (p == 'S' || p == 'W') return -decimal;

    return decimal;
}


FileFormats::PLN::PLN(const QString& fileName)
{
    bool ok;
    QString pos;
    QDomNodeList nodeList, posList;
    QGeoCoordinate waypoint;
    QStringList split;
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open PLN file %1 for reading.", "FileFormats::PLN").arg(fileName));
        return;
    }

    QDomDocument doc;
    QDomDocument::ParseResult result =
        doc.setContent(file->readAll(), QDomDocument::ParseOption::Default);
    if (!result)
    {
        setError(QObject::tr("Cannot parse PLN file %1 for XML. Reason: %2", "FileFormats::PLN").arg(fileName, result.errorMessage));
        return;
    }

    QDomElement root = doc.documentElement();
    nodeList = root.elementsByTagName("FlightPlan.FlightPlan");
    if ((QString::compare((QString) root.tagName(), u"SimBase.Document"_qs, Qt::CaseInsensitive) != 0) || (nodeList.size() != 1))
    {
            setError(QObject::tr("File %1 does not contain a flight plan", "FileFormats::PLN").arg(fileName));
            return;
    }

    nodeList = nodeList.at(0).toElement().elementsByTagName("ATCWaypoint");
    for (int i = 0; i < nodeList.size(); i++)
    {
        posList = nodeList.at(i).toElement().elementsByTagName("WorldPosition");
        if (posList.size() != 1)
        {
            setError(QObject::tr("Waypoint %1 does not have a unique position", "FileFormats::PLN").arg(i + 1));
            return;
        }
        pos = posList.at(0).toElement().text();
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
            addWarning(QObject::tr("Position of waypoint %1 is not a valid position", "FileFormats::PLN").arg(i + 1));
        }
    }
}


