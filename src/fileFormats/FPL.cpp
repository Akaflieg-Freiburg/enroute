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

#include "FPL.h"

#include "DataFileAbstract.h"




FileFormats::FPL::FPL(const QString& fileName)
{
    struct Wp
    {
        QString id;
        double lon;
        double lat;
        double elev;

        bool operator==(const Wp& wp) const
        {
            return id == wp.id;
        }
    } wp;
    QList<Wp> wplist;

    bool ok;
    int j;
    QDomNodeList nodeList, wayList;
    QGeoCoordinate waypoint;
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open FPL file %1 for reading.", "FileFormats::FPL").arg(fileName));
        return;
    }

    QDomDocument doc;
    QDomDocument::ParseResult result =
        doc.setContent(file->readAll(), QDomDocument::ParseOption::Default);
    if (!result)
    {
        setError(QObject::tr("Cannot parse FPL file %1 for XML. Reason: %2", "FileFormats::FPL").arg(fileName, result.errorMessage));
        return;
    }

    QDomElement root = doc.documentElement();
    if (QString::compare((QString) root.tagName(), u"flight-plan"_qs, Qt::CaseInsensitive) != 0)
    {
        setError(QObject::tr("File %1 does not contain a flight plan", "FileFormats::FPL").arg(fileName));
        return;
    }

    nodeList = doc.elementsByTagName("waypoint-table");
    if (nodeList.size() == 1)
    {
        wayList = nodeList.at(0).toElement().elementsByTagName("waypoint");
    }
    if ((nodeList.size() != 1) || (wayList.size() == 0))
    {
        setError(QObject::tr("File %1 does not contain a waypoint", "FileFormats::FPL").arg(fileName));
        return;
    }
    for (int i = 0; i < wayList.size(); i++)
    {
        nodeList = wayList.at(i).toElement().elementsByTagName("identifier");
        if (nodeList.size() != 1)
        {
            setError(QObject::tr("Waypoint %1 does not have a unique identifier", "FileFormats::FPL").arg(i + 1));
            return;
        }
        wp.id = nodeList.at(0).toElement().text();

        nodeList = wayList.at(i).toElement().elementsByTagName("lat");
        if (nodeList.size() != 1)
        {
            setError(QObject::tr("Waypoint %1 does not have a unique latitude", "FileFormats::FPL").arg(i + 1));
            return;
        }
        wp.lat = nodeList.at(0).toElement().text().toDouble(&ok);
        if (!ok)
        {
            setError(QObject::tr("Waypoint %1 does not have a valid latitude", "FileFormats::FPL").arg(i + 1));
            return;
        }

        nodeList = wayList.at(i).toElement().elementsByTagName("lon");
        if (nodeList.size() != 1)
        {
            setError(QObject::tr("Waypoint %1 does not have a unique longitude", "FileFormats::FPL").arg(i + 1));
            return;
        }
        wp.lon = nodeList.at(0).toElement().text().toDouble(&ok);
        if (!ok)
        {
            setError(QObject::tr("Waypoint %1 does not have a valid longitude", "FileFormats::FPL").arg(i + 1));
            return;
        }

        nodeList = wayList.at(i).toElement().elementsByTagName("elevation");
        if (nodeList.size() != 1)
        {
            wp.elev = 0;
        }
        else
        {
            wp.elev = nodeList.at(0).toElement().text().toDouble(&ok);
            if (!ok)
            {
                wp.elev = 0;
            }
        }

        wplist.append(wp);
    }

    nodeList = doc.elementsByTagName("route");
    if (nodeList.size() == 1)
    {
        wayList = nodeList.at(0).toElement().elementsByTagName("route-point");
    }
    if ((nodeList.size() != 1) || (wayList.size() == 0))
    {
        setError(QObject::tr("File %1 does not contain a route", "FileFormats::FPL").arg(fileName));
        return;
    }
    for (int i = 0; i < wayList.size(); i++)
    {
        nodeList = wayList.at(i).toElement().elementsByTagName("waypoint-identifier");
        if (nodeList.size() != 1)
        {
            setError(QObject::tr("Route point %1 does not have a unique waypoint identifier", "FileFormats::FPL").arg(i + 1));
            return;
        }
        wp.id = nodeList.at(0).toElement().text();
        j = wplist.indexOf(wp, 0);
        if (j < 0)
        {
            setError(QObject::tr("Waypoint identifier for route point %1 does not exist", "FileFormats::FPL").arg(i + 1));
            return;
        }
        waypoint.setLatitude(wplist.at(j).lat);
        waypoint.setLongitude(wplist.at(j).lon);
        waypoint.setAltitude(wplist.at(j).elev);
        m_waypoints.append(waypoint);
    }
}


