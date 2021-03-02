/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include "Clock.h"
#include "weather/TAF.h"


Weather::TAF::TAF(QXmlStreamReader &xml, QObject *parent)
    : Weather::Decoder(parent)
{

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == "station_id") {
            _ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == "latitude") {
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "longitude") {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "elevation_m") {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == "raw_text") {
            _raw_text = xml.readElementText();
            continue;
        }

        // Read issue time
        if (xml.isStartElement() && name == "issue_time") {
            _issueTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        // Read expiration date
        if (xml.isStartElement() && name == "valid_time_to") {
            _expirationTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            continue;
        }

        if (xml.isEndElement() && name == "TAF") {
            break;
}

        xml.skipCurrentElement();
    }

    setRawText(_raw_text, _issueTime.date().addDays(5));
    setupSignals();
}


Weather::TAF::TAF(QDataStream &inputStream, QObject *parent)
    : Weather::Decoder(parent)
{
    inputStream >> _expirationTime;
    inputStream >> _ICAOCode;
    inputStream >> _issueTime;
    inputStream >> _location;
    inputStream >> _raw_text;

    setRawText(_raw_text, _issueTime.date().addDays(5));
    setupSignals();
}


auto Weather::TAF::isExpired() const -> bool
{
    if (!_expirationTime.isValid()) {
        return true;
}
    return QDateTime::currentDateTime() > _expirationTime;
}


auto Weather::TAF::isValid() const -> bool
{
    if (!_location.isValid()) {
        return false;
}
    if (!_expirationTime.isValid()) {
        return false;
}
    if (!_issueTime.isValid()) {
        return false;
}
    if (_ICAOCode.isEmpty()) {
        return false;
}
    if (hasParseError()) {
        return false;
}

    return true;
}


auto Weather::TAF::relativeIssueTime() const -> QString
{
    if (!_issueTime.isValid()) {
        return QString();
}

    return Clock::describeTimeDifference(_issueTime);
}


void Weather::TAF::setupSignals() const
{
    // Emit notifier signals whenever the time changes
    connect(Clock::globalInstance(), &Clock::timeChanged, this, &Weather::TAF::relativeIssueTimeChanged);
}


void Weather::TAF::write(QDataStream &out)
{
    out << _expirationTime;
    out << _ICAOCode;
    out << _issueTime;
    out << _location;
    out << _raw_text;
}
