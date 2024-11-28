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

#include <QDataStream>
#include <QXmlStreamAttribute>

#include "GlobalObject.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "weather/TAF.h"

using namespace Qt::Literals::StringLiterals;


Weather::TAF::TAF(QObject *parent)
    : QObject(parent)
{

}


Weather::TAF::TAF(QXmlStreamReader &xml, QObject *parent)
    : QObject(parent)
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
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"longitude"_s)
        {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"elevation_m"_s)
        {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == u"raw_text"_s)
        {
            _raw_text = xml.readElementText();
            continue;
        }

        // Read issue time
        if (xml.isStartElement() && name == u"issue_time"_s)
        {
            _issueTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        // Read expiration date
        if (xml.isStartElement() && name == u"valid_time_to"_s)
        {
            _expirationTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        if (xml.isEndElement() && name == u"TAF"_s)
        {
            break;
        }

        xml.skipCurrentElement();
    }

    m_decoder.setRawText(_raw_text, _issueTime.date().addDays(5));
    setupSignals();
}


Weather::TAF::TAF(QDataStream &inputStream, QObject *parent)
    : QObject(parent)
{
    inputStream >> _expirationTime;
    inputStream >> m_ICAOCode;
    inputStream >> _issueTime;
    inputStream >> _location;
    inputStream >> _raw_text;

    m_decoder.setRawText(_raw_text, _issueTime.date().addDays(5));
    setupSignals();
}


auto Weather::TAF::isExpired() const -> bool
{
    if (!_expirationTime.isValid())
    {
        return true;
    }
    return QDateTime::currentDateTime() > _expirationTime;
}


auto Weather::TAF::isValid() const -> bool
{
    if (!_location.isValid())
    {
        return false;
    }
    if (!_expirationTime.isValid())
    {
        return false;
    }
    if (!_issueTime.isValid())
    {
        return false;
    }
    if (m_ICAOCode.isEmpty())
    {
        return false;
    }
    if (m_decoder.hasParseError())
    {
        return false;
    }

    return true;
}


auto Weather::TAF::relativeIssueTime() const -> QString
{
    if (!_issueTime.isValid())
    {
        return {};
    }

    return Navigation::Clock::describeTimeDifference(_issueTime);
}


void Weather::TAF::setupSignals() const
{
    // Emit notifier signals whenever the time changes
    connect(Navigation::Navigator::clock(), &Navigation::Clock::timeChanged, this, &Weather::TAF::relativeIssueTimeChanged);
}


void Weather::TAF::write(QDataStream &out)
{
    out << _expirationTime;
    out << m_ICAOCode;
    out << _issueTime;
    out << _location;
    out << _raw_text;
}
