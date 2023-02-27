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

#include <QJsonArray>
#include <QJsonDocument>
#include <QtGlobal>

#include "notam/NotamList.h"


NOTAM::NotamList::NotamList(const QByteArray& jsonData, const QGeoCircle& region)
{
    auto doc = QJsonDocument::fromJson(jsonData);
    auto items = doc[u"items"_qs].toArray();

    foreach(auto item, items)
    {
        Notam notam(item.toObject());

        // Ignore invalid notams
        if (!notam.isValid())
        {
            continue;
        }

        // Ignore outdated notams
        if (notam.isOutdated())
        {
            continue;
        }

        // Ignore IFR notams
        if (notam.traffic() == u"I"_qs)
        {
            continue;
        }

        // Ignore duplicated entries
        if (m_notams.contains(notam))
        {
            continue;
        }

        m_notams.append(notam);
    }

    m_retrieved = QDateTime::currentDateTimeUtc();
    m_region = region;
}



//
// Getter Methods
//

QString NOTAM::NotamList::summary() const
{
    QStringList results;

    if (m_notams.empty())
    {
        results += QObject::tr("No NOTAMs", "NOTAM::NotamList");
    }
    else
    {
        results += QObject::tr("NOTAMs available", "NOTAM::NotamList");
    }

    if (!isValid() || isOutdated())
    {
        results += QObject::tr("Data potentially outdated. Update requested.", "NOTAM::NotamList");
    }

    return results.join(u" • "_qs);
}



//
// Methods
//

Units::Time NOTAM::NotamList::age() const
{
    if (!m_retrieved.isValid())
    {
        return {};
    }

    return Units::Time::fromS( double(m_retrieved.secsTo(QDateTime::currentDateTimeUtc()) ));
}


NOTAM::NotamList NOTAM::NotamList::cleaned() const
{
    NotamList result;
    result.m_region = m_region;
    result.m_retrieved = m_retrieved;

    foreach(auto notam, m_notams)
    {
        if (!notam.isValid())
        {
            continue;
        }
        if (notam.isOutdated())
        {
            continue;
        }
        if (result.m_notams.contains(notam))
        {
            continue;
        }
        result.m_notams.append(notam);
    }

    return result;
}


NOTAM::NotamList NOTAM::NotamList::restricted(const GeoMaps::Waypoint& waypoint) const
{
    NotamList result;
    result.m_retrieved = m_retrieved;
    auto radius = qMax(0.0, m_region.radius() - m_region.center().distanceTo(waypoint.coordinate()));

    result.m_region = QGeoCircle(waypoint.coordinate(), radius);

    foreach(auto notam, m_notams)
    {
        if (!notam.isValid())
        {
            continue;
        }
        if (notam.isOutdated())
        {
            continue;
        }
        if (result.m_notams.contains(notam))
        {
            continue;
        }
        if (!notam.region().contains(waypoint.coordinate()))
        {
            continue;
        }
        result.m_notams.append(notam);
    }

    std::sort(result.m_notams.begin(), result.m_notams.end(),
              [](const Notam& a, const Notam& b)
    {
        auto cur = QDateTime::currentDateTime();
        auto a_effectiveStart = qMax(a.effectiveStart(), cur);
        auto b_effectiveStart = qMax(b.effectiveStart(), cur);

        if (a_effectiveStart != b_effectiveStart)
        {
            return a_effectiveStart < b_effectiveStart;
        }
        return a.effectiveEnd() < b.effectiveEnd();
    });

    return result;
}



//
// Non-Member Methods
//

QDataStream& NOTAM::operator<<(QDataStream& stream, const NOTAM::NotamList& notamList)
{
    stream << notamList.m_notams;
    stream << notamList.m_region;
    stream << notamList.m_retrieved;

    return stream;
}


QDataStream& NOTAM::operator>>(QDataStream& stream, NOTAM::NotamList& notamList)
{
    stream >> notamList.m_notams;
    stream >> notamList.m_region;
    stream >> notamList.m_retrieved;

    return stream;
}
