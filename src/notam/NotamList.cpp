/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


#include "notam/NotamList.h"



NOTAM::NotamList::NotamList(const QByteArray& jsonData, const QGeoCircle& region)
{
    auto doc = QJsonDocument::fromJson(jsonData);
    auto items = doc[u"items"_qs].toArray();

    foreach(auto item, items)
    {
        Notam notam(item.toObject());

        // Ignore IFR notams
        if (notam.m_traffic == u"I"_qs)
        {
            continue;
        }

        m_notams.append(notam);
    }

    m_retrieved = QDateTime::currentDateTimeUtc();
    m_region = region;
}


QString NOTAM::NotamList::summary() const
{
    QStringList results;

    if (m_notams.size() > 0)
    {
        results += u"NOTAMs available."_qs;
    }

    if (!m_retrieved.isValid() || (m_retrieved.addDays(1) <  QDateTime::currentDateTime()))
    {
        results += u"Data potentially outdated."_qs;
    }

    return results.join(u" • "_qs);
}


bool NOTAM::NotamList::covers(const GeoMaps::Waypoint& waypoint)
{
    if (!m_region.isValid() || !waypoint.coordinate().isValid())
    {
        return false;
    }
    return m_region.contains(waypoint.coordinate());
}


NOTAM::NotamList NOTAM::NotamList::restrict(const GeoMaps::Waypoint& waypoint) const
{
    /*
    if (!covers(waypoint))
    {
        return {};
    }
    */

    NotamList result;

    result.m_retrieved = m_retrieved;
    result.m_region = QGeoCircle(waypoint.coordinate(), 5000);
    foreach(auto notam, m_notams)
    {
        if (notam.m_region.contains(waypoint.coordinate()))
        {
            result.m_notams.append(notam);
        }
    }

    return result;
}



QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList& notamList)
{
    stream << notamList.m_notams;
    stream << notamList.m_region;
    stream << notamList.m_retrieved;

    return stream;
}


QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notamList)
{
    stream >> notamList.m_notams;
    stream >> notamList.m_region;
    stream >> notamList.m_retrieved;

    return stream;
}
