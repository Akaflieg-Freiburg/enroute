/***************************************************************************
 *   Copyright (C) 2023-2025 by Stefan Kebekus                             *
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

#include "notam/NOTAMList.h"
#include "notam/NOTAMProvider.h"

using namespace Qt::Literals::StringLiterals;


NOTAM::NOTAMList::NOTAMList(const QJsonDocument& jsonDoc, const QGeoCircle& region, QSet<QString>* cancelledNotamNumbers)
{
    QSet<QString> numbersSeen;

    const auto items = jsonDoc[u"items"_s].toArray();
    for(const auto item : items)
    {
        NOTAM const notam(item.toObject());

        // Ignore invalid notams
        if (!notam.isValid())
        {
            continue;
        }
        if (notam.cancels() != u""_s)
        {
            if (cancelledNotamNumbers != nullptr)
            {
                cancelledNotamNumbers->insert(notam.cancels());
            }
            continue;
        }

        // Ignore outdated NOTAMs
        if (notam.isOutdated())
        {
            continue;
        }

        // Ignore NOTAMs that do not pertain to VFR traffic. This excludes IFR-only NOTAMs and checklist NOTAMs.
        if (!notam.traffic().contains(u"V"_s))
        {
            continue;
        }

        // Ignore duplicated entries. It turns out that the FAA duplicates NOTAMS, with multiple
        // FIR entries, providing one copy for each FIR.
        if (numbersSeen.contains(notam.number()))
        {
            continue;
        }

        m_notams.append(notam);
        numbersSeen += notam.number();
    }

    m_retrieved = QDateTime::currentDateTimeUtc();
    m_region = region;
}



//
// Getter Methods
//

QString NOTAM::NOTAMList::summary() const
{
    QStringList results;

    if (m_notams.empty())
    {
        results += QObject::tr("No NOTAMs known", "NOTAM::NotamList");
    }
    else
    {
        results += QObject::tr("NOTAMs available", "NOTAM::NotamList");
    }

    if (!isValid() || isOutdated())
    {
        results += QObject::tr("Update requested.", "NOTAM::NotamList");
    }

    return results.join(u" • "_s);
}



//
// Methods
//

Units::Timespan NOTAM::NOTAMList::age() const
{
    if (!m_retrieved.isValid())
    {
        return {};
    }
    
    return Units::Timespan::fromS( double(m_retrieved.secsTo(QDateTime::currentDateTimeUtc()) ));
}


NOTAM::NOTAMList NOTAM::NOTAMList::cleaned(const QSet<QString>& cancelledNotamNumbers) const
{
    NOTAMList result;
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
        if (cancelledNotamNumbers.contains(notam.number()))
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


NOTAM::NOTAMList NOTAM::NOTAMList::restricted(const GeoMaps::Waypoint& waypoint) const
{
    NOTAMList result;
    result.m_retrieved = m_retrieved;
    auto radius = qMin(restrictionRadius.toM(), qMax(0.0, m_region.radius() - m_region.center().distanceTo(waypoint.coordinate())));

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
        if (notam.coordinate().distanceTo(waypoint.coordinate()) > restrictionRadius.toM())
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
        notam.updateSectionTitle();
        result.m_notams.append(notam);
    }

    std::sort(result.m_notams.begin(), result.m_notams.end(),
              [](const NOTAM& first, const NOTAM& second)
    {
        auto aRead = GlobalObject::notamProvider()->isRead(first.number());
        auto bRead = GlobalObject::notamProvider()->isRead(second.number());
        if (aRead != bRead)
        {
            return !aRead;
        }

        auto cur = QDateTime::currentDateTime();
        auto a_effectiveStart = qMax(first.effectiveStart(), cur);
        auto b_effectiveStart = qMax(second.effectiveStart(), cur);

        if (a_effectiveStart != b_effectiveStart)
        {
            return a_effectiveStart < b_effectiveStart;
        }
        return first.effectiveEnd() < second.effectiveEnd();
    });

    return result;
}



//
// Non-Member Methods
//

QDataStream& NOTAM::operator<<(QDataStream& stream, const NOTAMList& notamList)
{
    stream << notamList.m_notams;
    stream << notamList.m_region;
    stream << notamList.m_retrieved;

    return stream;
}


QDataStream& NOTAM::operator>>(QDataStream& stream, NOTAMList& notamList)
{
    stream >> notamList.m_notams;
    stream >> notamList.m_region;
    stream >> notamList.m_retrieved;

    return stream;
}
