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

void NOTAM::Notam::read(const QJsonObject& jsonObject)
{
    auto notam = jsonObject[u"properties"_qs][u"coreNOTAMData"_qs][u"notam"_qs].toObject();

    m_effectiveStartString = notam[u"effectiveStart"_qs].toString();
    m_effectiveStart = QDateTime::fromString(m_effectiveStartString, Qt::ISODate);
    m_effectiveEndString = notam[u"effectiveEnd"_qs].toString();
    m_effectiveEnd = QDateTime::fromString(m_effectiveEndString, Qt::ISODate);
    m_icaoLocation = notam[u"location"_qs].toString();
    m_text = notam[u"text"_qs].toString();
    m_traffic = notam[u"traffic"_qs].toString();
}


QString NOTAM::Notam::richText() const
{
    QStringList result;

    if (m_effectiveEnd.isValid())
    {
        result += u"<strong>Effective from %1 to %2</strong>"_qs.arg(m_effectiveStart.toString(u"dd.MM.yy hh:00"_qs), m_effectiveEnd.toString(u"dd.MM.yy hh:00"_qs));
    }
    else
    {
        result += u"<strong>Effective from %1</strong>"_qs.arg(m_effectiveStart.toString(u"dd.MM.yy hh:00"_qs));
        result += u"<strong>%1</strong>"_qs.arg(m_effectiveEndString);
    }
    result += m_text;
    return result.join(u" • "_qs);
}
