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

#include "GlobalObject.h"
#include "navigation/Aircraft.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "weather/METAR.h"

using namespace Qt::Literals::StringLiterals;


Weather::METAR::METAR(QObject *parent)
    : Weather::Decoder(parent)
{

}


Weather::METAR::METAR(QXmlStreamReader &xml, QObject *parent)
    : Weather::Decoder(parent)
{

    while (true) {
        xml.readNextStartElement();
        QString const name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == u"station_id"_s) {
            m_ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == u"latitude"_s) {
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"longitude"_s) {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"elevation_m"_s) {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == u"raw_text"_s) {
            _raw_text = xml.readElementText();
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == u"altim_in_hg"_s) {
            auto content = xml.readElementText();
            m_qnh = Units::Pressure::fromInHg(content.toDouble());
            if ((m_qnh.toHPa() < 800) || (m_qnh.toHPa() > 1200))
            {
                m_qnh = Units::Pressure::fromPa(qQNaN());
            }
            continue;
        }

        // Wind
        if (xml.isStartElement() && name == u"wind_speed_kt"_s) {
            auto content = xml.readElementText();
            _wind = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Gust
        if (xml.isStartElement() && name == u"wind_gust_kt"_s) {
            auto content = xml.readElementText();
            _gust = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == u"observation_time"_s) {
            auto content = xml.readElementText();
            _observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == u"flight_category"_s) {
            auto content = xml.readElementText();
            if (content == u"VFR"_s) {
                _flightCategory = VFR;
            }
            if (content == u"MVFR"_s) {
                _flightCategory = MVFR;
            }
            if (content == u"IFR"_s) {
                _flightCategory = IFR;
            }
            if (content == u"LIFR"_s) {
                _flightCategory = LIFR;
            }
            continue;
        }

        if (xml.isEndElement() && name == u"METAR"_s) {
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
    inputStream >> m_ICAOCode;
    inputStream >> _location;
    inputStream >> _observationTime;
    inputStream >> m_qnh;
    inputStream >> _raw_text;
    inputStream >> _wind;
    inputStream >> _gust;

    // Interpret the METAR message
    setRawText(_raw_text, _observationTime.date());
    setupSignals();
}


auto Weather::METAR::expiration() const -> QDateTime
{
    if (_raw_text.contains(u"NOSIG"_s)) {
        return _observationTime.addSecs(3LL*60LL*60LL);
    }
    return _observationTime.addSecs(1.5*60*60);
}


auto Weather::METAR::flightCategoryColor() const -> QString
{
    if (_flightCategory == VFR) {
        return QStringLiteral("green");
    }
    if (_flightCategory == MVFR) {
        return QStringLiteral("yellow");
    }
    if ((_flightCategory == IFR) || (_flightCategory == LIFR)) {
        return QStringLiteral("red");
    }
    return QStringLiteral("transparent");
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
    if (m_ICAOCode.isEmpty()) {
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
        return {};
    }

    return Navigation::Clock::describeTimeDifference(_observationTime);
}


void Weather::METAR::setupSignals() const
{
    // Emit notifier signals whenever the time changes
    connect(Navigation::Navigator::clock(), &Navigation::Clock::timeChanged, this, &Weather::METAR::summaryChanged);
    connect(Navigation::Navigator::clock(), &Navigation::Clock::timeChanged, this, &Weather::METAR::relativeObservationTimeChanged);

    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Weather::METAR::summaryChanged);
}


auto Weather::METAR::summary() const -> QString {

    QStringList resultList;

    switch (_flightCategory) {
    case VFR:
        if (_raw_text.contains(u"CAVOK"_s)) {
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
    if (_gust.toKN() > 15) {
        switch (GlobalObject::navigator()->aircraft().horizontalDistanceUnit()) {
        case Navigation::Aircraft::Kilometer:
            resultList << tr("gusts of %1 km/h").arg( qRound(_gust.toKMH()) );
            break;
        case Navigation::Aircraft::StatuteMile:
            resultList << tr("gusts of %1 mph").arg( qRound(_gust.toMPH()) );
            break;
        case Navigation::Aircraft::NauticalMile:
            resultList << tr("gusts of %1 kn").arg( qRound(_gust.toKN()) );
            break;
        }
    } else if (_wind.toKN() > 10) {
        switch (GlobalObject::navigator()->aircraft().horizontalDistanceUnit()) {
        case Navigation::Aircraft::Kilometer:
            resultList << tr("wind at %1 km/h").arg( qRound(_wind.toKMH()) );
            break;
        case Navigation::Aircraft::StatuteMile:
            resultList << tr("wind at %1 mph").arg( qRound(_wind.toMPH()) );
            break;
        case Navigation::Aircraft::NauticalMile:
            resultList << tr("wind at %1 kn").arg( qRound(_wind.toKN()) );
            break;
        }
    }

    // Weather
    auto curWeather = currentWeather();
    if (!curWeather.isEmpty()) {
        resultList << curWeather;
    }

    if (resultList.isEmpty()) {
        return tr("%1 %2").arg(messageType(), Navigation::Clock::describeTimeDifference(_observationTime));
    }

    return tr("%1 %2: %3").arg(messageType(), Navigation::Clock::describeTimeDifference(_observationTime), resultList.join(QStringLiteral(" • ")));
}


void Weather::METAR::write(QDataStream &out)
{
    out << _flightCategory;
    out << m_ICAOCode;
    out << _location;
    out << _observationTime;
    out << m_qnh;
    out << _raw_text;
    out << _wind;
    out << _gust;
}
