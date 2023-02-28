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

#include <QJsonObject>

#include "notam/Notam.h"



NOTAM::Notam::Notam(const QJsonObject& jsonObject)
{
    auto notamObject = jsonObject[u"properties"_qs][u"coreNOTAMData"_qs][u"notam"_qs].toObject();

    m_coordinates = interpretNOTAMCoordinates(notamObject[u"coordinates"_qs].toString());
    m_effectiveEndString = notamObject[u"effectiveEnd"_qs].toString();
    m_effectiveStartString = notamObject[u"effectiveStart"_qs].toString();
    m_icaoLocation = notamObject[u"location"_qs].toString();
    m_number = notamObject[u"number"_qs].toString();
    m_text = notamObject[u"text"_qs].toString();
    m_traffic = notamObject[u"traffic"_qs].toString();
    m_radius = Units::Distance::fromNM( notamObject[u"radius"_qs].toDouble() );

    m_effectiveEnd = QDateTime::fromString(m_effectiveEndString, Qt::ISODate);
    m_effectiveStart = QDateTime::fromString(m_effectiveStartString, Qt::ISODate);
    m_region = QGeoCircle(m_coordinates, qMax( Units::Distance::fromNM(1).toM(), m_radius.toM() ));
}


//
// Getter Methods
//

bool NOTAM::Notam::isValid() const
{
    if (!m_coordinates.isValid())
    {
        return false;
    }
    if (m_icaoLocation.size() != 4)
    {
        return false;
    }
    if (m_text.isEmpty())
    {
        return false;
    }
    return true;
}



//
// Methods
//


QString NOTAM::Notam::richText() const
{
    QStringList result;

    auto effectiveStartString = m_effectiveStartString;
    if (m_effectiveStart.isValid())
    {
        if (m_effectiveStart < QDateTime::currentDateTime())
        {
            effectiveStartString = {};
        }
        else
        {
            effectiveStartString = m_effectiveStart.toString(u"ddMMMyy hh:00"_qs);
        }
    }
    auto effectiveEndString = m_effectiveEndString;
    if (m_effectiveEnd.isValid())
    {
        effectiveEndString = m_effectiveEnd.toString(u"ddMMMyy hh:00"_qs);
    }

    if (m_effectiveEnd.isValid())
    {
        result += u"<strong>Effective %1 to %2</strong>"_qs.arg(effectiveStartString, effectiveEndString);
    }
    else
    {
        result += u"<strong>Effective %1</strong>"_qs.arg(effectiveStartString);
        result += u"<strong>%1</strong>"_qs.arg(effectiveEndString);
    }

    result += m_text;
    return result.join(u" • "_qs).replace(u"  "_qs, u" "_qs);
}



//
// Non-Member Methods
//

QGeoCoordinate NOTAM::interpretNOTAMCoordinates(const QString& string)
{
    if (string.length() != 11)
    {
        return {};
    }

    bool ok;
    auto latD = string.left(2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    auto latM = string.mid(2, 2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }

    double lat = latD+latM/60.0;
    if (string[4] == 'S')
    {
        lat *= -1.0;
    }

    auto lonD = string.mid(5, 3).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    auto lonM = string.mid(8, 2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    double lon = lonD+lonM/60.0;
    if (string[10] == 'W')
    {
        lon *= -1.0;
    }

    return QGeoCoordinate(lat, lon);
}


QDataStream& NOTAM::operator<<(QDataStream& stream, const NOTAM::Notam& notam)
{
    stream << notam.m_coordinates;
    stream << notam.m_effectiveEnd;
    stream << notam.m_effectiveEndString;
    stream << notam.m_effectiveStart;
    stream << notam.m_effectiveStartString;
    stream << notam.m_icaoLocation;
    stream << notam.m_number;
    stream << notam.m_radius;
    stream << notam.m_region;
    stream << notam.m_text;
    stream << notam.m_traffic;

    return stream;
}


QDataStream& NOTAM::operator>>(QDataStream& stream, NOTAM::Notam& notam)
{
    stream >> notam.m_coordinates;
    stream >> notam.m_effectiveEnd;
    stream >> notam.m_effectiveEndString;
    stream >> notam.m_effectiveStart;
    stream >> notam.m_effectiveStartString;
    stream >> notam.m_icaoLocation;
    stream >> notam.m_number;
    stream >> notam.m_radius;
    stream >> notam.m_region;
    stream >> notam.m_text;
    stream >> notam.m_traffic;

    return stream;

}
