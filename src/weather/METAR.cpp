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
#include "navigation/Atmosphere.h"
#include "navigation/Clock.h"
#include "weather/DensityAltitude.h"
#include "weather/METAR.h"


Weather::METAR::METAR(QObject *parent)
    : Weather::Decoder(parent)
{

}


Weather::METAR::METAR(QXmlStreamReader& xml, QObject* parent)
    : Weather::Decoder(parent)
{

    while (true)
    {
        xml.readNextStartElement();
        QString const name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == u"station_id"_qs)
        {
            m_ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == u"latitude"_qs)
        {
            m_location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"longitude"_qs)
        {
            m_location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == u"elevation_m"_qs)
        {
            m_location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == u"raw_text"_qs)
        {
            m_raw_text = xml.readElementText();
            continue;
        }

        // Read temperature
        if (xml.isStartElement() && name == u"temp_c"_qs)
        {
            m_temperature = Units::Temperature::fromDegreeCelsius(xml.readElementText().toDouble());
            continue;
        }

        // Read dewpoint
        if (xml.isStartElement() && name == u"dewpoint_c"_qs)
        {
            m_dewpoint = Units::Temperature::fromDegreeCelsius(xml.readElementText().toDouble());
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == u"altim_in_hg"_qs)
        {
            auto content = xml.readElementText();
            m_qnh = Units::Pressure::fromInHg(content.toDouble());
            if ((m_qnh.toHPa() < 800) || (m_qnh.toHPa() > 1200))
            {
                m_qnh = Units::Pressure::fromPa(qQNaN());
            }
            continue;
        }

        // Wind
        if (xml.isStartElement() && name == u"wind_speed_kt"_qs)
        {
            auto content = xml.readElementText();
            m_wind = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Gust
        if (xml.isStartElement() && name == u"wind_gust_kt"_qs)
        {
            auto content = xml.readElementText();
            m_gust = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == u"observation_time"_qs)
        {
            auto content = xml.readElementText();
            m_observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == u"flight_category"_qs)
        {
            auto content = xml.readElementText();
            if (content == u"VFR"_qs) {
                m_flightCategory = VFR;
            }
            if (content == u"MVFR"_qs) {
                m_flightCategory = MVFR;
            }
            if (content == u"IFR"_qs) {
                m_flightCategory = IFR;
            }
            if (content == u"LIFR"_qs) {
                m_flightCategory = LIFR;
            }
            continue;
        }

        if (xml.isEndElement() && name == u"METAR"_qs)
        {
            break;
        }

        xml.skipCurrentElement();
    }

    // Calculate density altitude
    Units::Distance const altitude = Units::Distance::fromM(m_location.altitude());
    if (m_temperature.isFinite() && m_qnh.isFinite() && altitude.isFinite() && m_dewpoint.isFinite() )
    {
        m_densityAltitude = Weather::DensityAltitude::calculateDensityAltitude(m_temperature, m_qnh, altitude, m_dewpoint);
    }

    // Interpret the METAR message
    setRawText(m_raw_text, m_observationTime.date());
}


Weather::METAR::METAR(QDataStream& inputStream, QObject* parent)
    : Weather::Decoder(parent)
{
    inputStream >> m_flightCategory;
    inputStream >> m_ICAOCode;
    inputStream >> m_location;
    inputStream >> m_observationTime;
    inputStream >> m_qnh;
    inputStream >> m_raw_text;
    inputStream >> m_wind;
    inputStream >> m_gust;
    inputStream >> m_temperature;
    inputStream >> m_dewpoint;
    inputStream >> m_densityAltitude;

    // Interpret the METAR message
    setRawText(m_raw_text, m_observationTime.date());
}

QDateTime Weather::METAR::expiration() const
{
    if (m_raw_text.contains(u"NOSIG"_qs))
    {
        return m_observationTime.addSecs(3LL*60LL*60LL);
    }
    return m_observationTime.addSecs(1.5*60*60);
}


QString Weather::METAR::flightCategoryColor() const
{
    if (m_flightCategory == VFR)
    {
        return QStringLiteral("green");
    }
    if (m_flightCategory == MVFR)
    {
        return QStringLiteral("yellow");
    }
    if ((m_flightCategory == IFR) || (m_flightCategory == LIFR))
    {
        return QStringLiteral("red");
    }
    return QStringLiteral("transparent");
}


bool Weather::METAR::isValid() const
{
    if (!m_location.isValid())
    {
        return false;
    }
    if (!m_observationTime.isValid())
    {
        return false;
    }
    if (m_ICAOCode.isEmpty())
    {
        return false;
    }
    if (hasParseError())
    {
        return false;
    }

    return true;
}


QString Weather::METAR::summary(const Navigation::Aircraft& aircraft, const QDateTime& currentTime) const
{
    QStringList resultList;

    switch (m_flightCategory)
    {
    case VFR:
        if (m_raw_text.contains(u"CAVOK"_qs))
        {
            resultList << tr("CAVOK");
        }
        else
        {
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
    if (m_gust.toKN() > 15)
    {
        switch (aircraft.horizontalDistanceUnit())
        {
        case Navigation::Aircraft::Kilometer:
            resultList << tr("gusts of %1 km/h").arg( qRound(m_gust.toKMH()) );
            break;
        case Navigation::Aircraft::StatuteMile:
            resultList << tr("gusts of %1 mph").arg( qRound(m_gust.toMPH()) );
            break;
        case Navigation::Aircraft::NauticalMile:
            resultList << tr("gusts of %1 kn").arg( qRound(m_gust.toKN()) );
            break;
        }
    }
    else if (m_wind.toKN() > 10)
    {
        switch (aircraft.horizontalDistanceUnit())
        {
        case Navigation::Aircraft::Kilometer:
            resultList << tr("wind at %1 km/h").arg( qRound(m_wind.toKMH()) );
            break;
        case Navigation::Aircraft::StatuteMile:
            resultList << tr("wind at %1 mph").arg( qRound(m_wind.toMPH()) );
            break;
        case Navigation::Aircraft::NauticalMile:
            resultList << tr("wind at %1 kn").arg( qRound(m_wind.toKN()) );
            break;
        }
    }

    // Weather
    auto curWeather = currentWeather();
    if (!curWeather.isEmpty())
    {
        resultList << curWeather;
    }

    if (resultList.isEmpty())
    {
        return tr("%1 %2").arg(messageType(), Navigation::Clock::describeTimeDifference(m_observationTime, currentTime));
    }

    return tr("%1 %2: %3").arg(messageType(), Navigation::Clock::describeTimeDifference(m_observationTime, currentTime), resultList.join(QStringLiteral(" • ")));
}


QString Weather::METAR::derivedData(const Navigation::Aircraft& aircraft) const
{
    QStringList items;
    if (m_densityAltitude.isFinite())
    {
        Units::Distance const altitude = Units::Distance::fromM(m_location.altitude());
        items += tr("Density Altitude: %1 (Δ %2)").arg(
            aircraft.verticalDistanceToString(m_densityAltitude), 
            aircraft.verticalDistanceToString(m_densityAltitude - altitude, /*forceSign=*/ true)
            );

        // koch chart approximation formulas based on https://groups.google.com/g/rec.aviation.piloting/c/SDlMioBVqaA/m/8_x9GYsnGwAJ
        const double densityAltitudeInFt = m_densityAltitude.toFeet();
        int takeoffDistIncPercentage = 100 * (densityAltitudeInFt / 1000) * .15;
        takeoffDistIncPercentage = (takeoffDistIncPercentage > 5) ? (((takeoffDistIncPercentage + 9) / 10) * 10) : 0; // round up to next group of ten; less than 6 percent considered as 0
        int rateOfClimbDecreasePercentage = 100 * (densityAltitudeInFt / 1000) * .075;
        rateOfClimbDecreasePercentage = (rateOfClimbDecreasePercentage > 5) ? (((rateOfClimbDecreasePercentage + 9) / 10) * 10) : 0;
        
        if (takeoffDistIncPercentage > 0)
        {
            if (takeoffDistIncPercentage <= 100)
            {
                items += tr("Takeoff distance increases by ≈ %1\%").arg(takeoffDistIncPercentage);
            }
            else 
            {
                items += tr("Takeoff distance increases by > 100\%");
            }
        } 
        
        if (rateOfClimbDecreasePercentage > 0)
        {
            if (rateOfClimbDecreasePercentage <= 100)
            {
                items += tr("Rate of climb decreases by ≈ %1\%").arg(rateOfClimbDecreasePercentage);
            }
            else
            {
                items += tr("Rate of climb decreases by > 100\%");
            }
        }
    }
    auto relativeHumidity = Navigation::Atmosphere::relativeHumidity(m_temperature, m_dewpoint);
    if (!std::isnan(relativeHumidity))
    {
        items += tr("Relative Humidity: %1%").arg(qRound(relativeHumidity));
    }

    if (items.isEmpty())
    {
        return {};
    }

    QString result;
    result += u"<strong>"_qs + tr("Derived Data") + u"</strong>"_qs;
    result += QStringLiteral("<ul style=\"margin-left:-25px;\">");
    for(auto& item : items)
    {
        result += u"<li>"_qs;
        result += item;
        result += u"</li>"_qs;
    }
    result += QStringLiteral("</ul>");
    return result;
}


QDataStream& Weather::operator<<(QDataStream& stream, const Weather::METAR& metar)
{
    stream << metar.m_flightCategory;
    stream << metar.m_ICAOCode;
    stream << metar.m_location;
    stream << metar.m_observationTime;
    stream << metar.m_qnh;
    stream << metar.m_raw_text;
    stream << metar.m_wind;
    stream << metar.m_gust;
    stream << metar.m_temperature;
    stream << metar.m_dewpoint;
    stream << metar.m_densityAltitude;

    return stream;
}
