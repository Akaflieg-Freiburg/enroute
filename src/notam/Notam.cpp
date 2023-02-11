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
    auto notam = jsonObject["properties"]["coreNOTAMData"]["notam"].toObject();

    m_effectiveStart = QDateTime::fromString(notam["effectiveStart"].toString(), Qt::ISODate);
    m_effectiveEnd = QDateTime::fromString(notam["effectiveEnd"].toString(), Qt::ISODate);
    m_icaoLocation = notam["location"].toString();
    m_text = notam["text"].toString();
}


QString NOTAM::Notam::richText() const
{
    QStringList result;

    if (m_effectiveEnd.isValid())
    {
        result += QString("<strong>Effective from %1 to %2</strong>").arg(m_effectiveStart.toString("dd.MM.yy hh:00"), m_effectiveEnd.toString("dd.MM.yy hh:00"));
    }
    else
    {
        result += QString("<strong>Effective permanently from %1</strong>").arg(m_effectiveStart.toString("dd.MM.yy hh:00"));
    }
    result += m_text;
    return result.join(" • ");
}
