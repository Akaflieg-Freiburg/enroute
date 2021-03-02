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

#include "Clock.h"
#include "GlobalSettings.h"
#include "weather/METAR.h"


Weather::METAR::METAR(QObject *parent)
    : Weather::Decoder(parent)
{

}


Weather::METAR::METAR(QXmlStreamReader &xml, QObject *parent)
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

        // QNH
        if (xml.isStartElement() && name == "altim_in_hg") {
            auto content = xml.readElementText();
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200)) {
                _qnh = 0;
            }
            continue;
        }

        // Wind
        if (xml.isStartElement() && name == "wind_speed_kt") {
            auto content = xml.readElementText();
            _wind = AviationUnits::Speed::fromKT(content.toDouble());
            continue;
        }

        // Gust
        if (xml.isStartElement() && name == "wind_gust_kt") {
            auto content = xml.readElementText();
            _gust = AviationUnits::Speed::fromKT(content.toDouble());
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == "altim_in_hg") {
            auto content = xml.readElementText();
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200)) {
                _qnh = 0;
            }
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == "observation_time") {
            auto content = xml.readElementText();
            _observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == "flight_category") {
            auto content = xml.readElementText();
            if (content == "VFR") {
                _flightCategory = VFR;
            }
            if (content == "MVFR") {
                _flightCategory = MVFR;
            }
            if (content == "IFR") {
                _flightCategory = IFR;
            }
            if (content == "LIFR") {
                _flightCategory = LIFR;
            }
            continue;
        }

        if (xml.isEndElement() && name == "METAR") {
            break;
        }

        xml.skipCurrentElement();
    }

    // Interpret the METAR message
    setRawText(_raw_text, _observationTime.date());
    setupSignals();
}


Weather::METAR::METAR(QDataStream &inputStream, QObject *parent)
    : Weather::Decoder(parent)
{
    inputStream >> _flightCategory;
    inputStream >> _ICAOCode;
    inputStream >> _location;
    inputStream >> _observationTime;
    inputStream >> _qnh;
    inputStream >> _raw_text;
    inputStream >> _wind;
    inputStream >> _gust;

    // Interpret the METAR message
    setRawText(_raw_text, _observationTime.date());
    setupSignals();
}


auto Weather::METAR::expiration() const -> QDateTime
{
    if (_raw_text.contains("NOSIG")) {
        return _observationTime.addSecs(3*60*60);
    }
    return _observationTime.addSecs(1.5*60*60);
}


auto Weather::METAR::flightCategoryColor() const -> QString
{
    if (_flightCategory == VFR) {
        return "green";
    }
    if (_flightCategory == MVFR) {
        return "yellow";
    }
    if ((_flightCategory == IFR) || (_flightCategory == LIFR)) {
        return "red";
    }
    return "transparent";
}


auto Weather::METAR::isExpired() const -> bool
{
    auto exp = expiration();
    if (!exp.isValid()) {
        return false;
    }
    return QDateTime::currentDateTime() > exp;
}


auto Weather::METAR::isValid() const -> bool
{
    if (!_location.isValid()) {
        return false;
    }
    if (!_observationTime.isValid()) {
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


auto Weather::METAR::relativeObservationTime() const -> QString
{
    if (!_observationTime.isValid()) {
        return QString();
    }

    return Clock::describeTimeDifference(_observationTime);
}


void Weather::METAR::setupSignals() const
{
    // Emit notifier signals whenever the time changes
    connect(Clock::globalInstance(), &Clock::timeChanged, this, &Weather::METAR::summaryChanged);
    connect(Clock::globalInstance(), &Clock::timeChanged, this, &Weather::METAR::relativeObservationTimeChanged);

    connect(GlobalSettings::globalInstance(), &GlobalSettings::useMetricUnitsChanged, this, &Weather::METAR::summaryChanged);
}


auto Weather::METAR::summary() const -> QString {

    QStringList resultList;

    switch (_flightCategory) {
    case VFR:
        if (_raw_text.contains("CAVOK")) {
            resultList << tr("CAVOK");
        } else {
            resultList << tr("VMC");
        }
        break;
    case MVFR:
        resultList << tr("marginal VMC");
        break;
    case IFR:
        resultList << tr("IMC");
        break;
    case LIFR:
        resultList << tr("low IMC");
        break;
    default:
        break;
    }

    // Wind and Gusts
    if (_gust.toKT() > 15) {
        resultList << tr("gusts of %1").arg(_gust.toString() );
    } else if (_wind.toKT() > 10) {
        resultList << tr("wind at %1").arg(_wind.toString());
    }

    // Weather
    auto curWeather = currentWeather();
    if (!curWeather.isEmpty()) {
        resultList << curWeather;
    }

    if (resultList.isEmpty()) {
        return QString();
    }

    return tr("%1 %2: %3").arg(messageType(), Clock::describeTimeDifference(_observationTime), resultList.join(" • "));
}


void Weather::METAR::write(QDataStream &out)
{
    out << _flightCategory;
    out << _ICAOCode;
    out << _location;
    out << _observationTime;
    out << _qnh;
    out << _raw_text;
    out << _wind;
    out << _gust;
}
