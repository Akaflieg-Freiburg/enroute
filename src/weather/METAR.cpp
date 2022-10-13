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

#include "GlobalObject.h"
#include "Settings.h"
#include "navigation/Aircraft.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
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
        if (xml.isStartElement() && name == QLatin1String("station_id")) {
            m_ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == QLatin1String("latitude")) {
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == QLatin1String("longitude")) {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == QLatin1String("elevation_m")) {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == QLatin1String("raw_text")) {
            _raw_text = xml.readElementText();
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == QLatin1String("altim_in_hg")) {
            auto content = xml.readElementText();
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200)) {
                _qnh = 0;
            }
            continue;
        }

        // Wind
        if (xml.isStartElement() && name == QLatin1String("wind_speed_kt")) {
            auto content = xml.readElementText();
            _wind = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Gust
        if (xml.isStartElement() && name == QLatin1String("wind_gust_kt")) {
            auto content = xml.readElementText();
            _gust = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == QLatin1String("altim_in_hg")) {
            auto content = xml.readElementText();
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200)) {
                _qnh = 0;
            }
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == QLatin1String("observation_time")) {
            auto content = xml.readElementText();
            _observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == QLatin1String("flight_category")) {
            auto content = xml.readElementText();
            if (content == QLatin1String("VFR")) {
                _flightCategory = VFR;
            }
            if (content == QLatin1String("MVFR")) {
                _flightCategory = MVFR;
            }
            if (content == QLatin1String("IFR")) {
                _flightCategory = IFR;
            }
            if (content == QLatin1String("LIFR")) {
                _flightCategory = LIFR;
            }
            continue;
        }

        if (xml.isEndElement() && name == QLatin1String("METAR")) {
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
    if (_raw_text.contains(QLatin1String("NOSIG"))) {
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
    connect(GlobalObject::navigator()->clock(), &Navigation::Clock::timeChanged, this, &Weather::METAR::summaryChanged);
    connect(GlobalObject::navigator()->clock(), &Navigation::Clock::timeChanged, this, &Weather::METAR::relativeObservationTimeChanged);

    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Weather::METAR::summaryChanged);
}


auto Weather::METAR::summary() const -> QString {

    QStringList resultList;

    switch (_flightCategory) {
    case VFR:
        if (_raw_text.contains(QLatin1String("CAVOK"))) {
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
        return {};
    }

    return tr("%1 %2: %3").arg(messageType(), Navigation::Clock::describeTimeDifference(_observationTime), resultList.join(QStringLiteral(" • ")));
}


void Weather::METAR::write(QDataStream &out)
{
    out << _flightCategory;
    out << m_ICAOCode;
    out << _location;
    out << _observationTime;
    out << _qnh;
    out << _raw_text;
    out << _wind;
    out << _gust;
}
