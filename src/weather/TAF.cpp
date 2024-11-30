/***************************************************************************
 *   Copyright (C) 2020-2024 by Stefan Kebekus                             *
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

#include "weather/TAF.h"

using namespace Qt::Literals::StringLiterals;


Weather::TAF::TAF(QXmlStreamReader &xml)
{
    while (true)
    {
        xml.readNextStartElement();
        QString const name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == u"station_id"_s)
        {
            m_ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == u"latitude"_s)
        {
            m_location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"longitude"_s)
        {
            m_location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"elevation_m"_s)
        {
            m_location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == u"raw_text"_s)
        {
            m_rawText = xml.readElementText();
            continue;
        }

        // Read issue time
        if (xml.isStartElement() && name == u"issue_time"_s)
        {
            m_issueTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        // Read expiration date
        if (xml.isStartElement() && name == u"valid_time_to"_s)
        {
            m_expirationTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        if (xml.isEndElement() && name == u"TAF"_s)
        {
            break;
        }

        xml.skipCurrentElement();
    }
    m_decoder = QSharedPointer<Weather::Decoder>(new Weather::Decoder(m_rawText, m_issueTime.date().addDays(5)));
}


Weather::TAF::TAF(QDataStream &inputStream)
{
    inputStream >> m_expirationTime;
    inputStream >> m_ICAOCode;
    inputStream >> m_issueTime;
    inputStream >> m_location;
    inputStream >> m_rawText;

    m_decoder = QSharedPointer<Weather::Decoder>(new Weather::Decoder(m_rawText, m_issueTime.date().addDays(5)));
}


bool Weather::TAF::isValid() const
{
    if (!m_expirationTime.isValid())
    {
        return false;
    }
    if (m_ICAOCode.isEmpty())
    {
        return false;
    }
    if (!m_issueTime.isValid())
    {
        return false;
    }
    if (!m_location.isValid())
    {
        return false;
    }
    if (m_decoder.isNull())
    {
        return false;
    }
    if (!m_decoder->isValid())
    {
        return false;
    }

    return true;
}


QDataStream& Weather::operator<<(QDataStream& stream, const Weather::TAF& taf)
{
    stream << taf.m_expirationTime;
    stream << taf.m_ICAOCode;
    stream << taf.m_issueTime;
    stream << taf.m_location;
    stream << taf.m_rawText;

    return stream;
}

