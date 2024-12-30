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

#include "navigation/Atmosphere.h"
#include "navigation/Clock.h"
#include "weather/DensityAltitude.h"
#include "weather/METAR.h"

using namespace Qt::Literals::StringLiterals;


Weather::METAR::METAR(QXmlStreamReader& xml)
{
    while (true)
    {
        xml.readNextStartElement();
        QString const name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == u"station_id"_s) {
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

        // Read temperature
        if (xml.isStartElement() && name == u"temp_c"_s)
        {
            m_temperature = Units::Temperature::fromDegreeCelsius(xml.readElementText().toDouble());
            continue;
        }

        // Read dewpoint
        if (xml.isStartElement() && name == u"dewpoint_c"_s)
        {
            m_dewpoint = Units::Temperature::fromDegreeCelsius(xml.readElementText().toDouble());
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == u"altim_in_hg"_s)
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
        if (xml.isStartElement() && name == u"wind_speed_kt"_s)
        {
            auto content = xml.readElementText();
            m_wind = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Gust
        if (xml.isStartElement() && name == u"wind_gust_kt"_s)
        {
            auto content = xml.readElementText();
            m_gust = Units::Speed::fromKN(content.toDouble());
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == u"observation_time"_s)
        {
            auto content = xml.readElementText();
            m_observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == u"flight_category"_s)
        {
            auto content = xml.readElementText();
            if (content == u"VFR"_s) {
                m_flightCategory = VFR;
            }
            if (content == u"MVFR"_s) {
                m_flightCategory = MVFR;
            }
            if (content == u"IFR"_s) {
                m_flightCategory = IFR;
            }
            if (content == u"LIFR"_s) {
                m_flightCategory = LIFR;
            }
            continue;
        }

        if (xml.isEndElement() && name == u"METAR"_s)
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
    m_decoder = QSharedPointer<Weather::Decoder>(new Weather::Decoder(m_rawText, m_observationTime.date()));
}


QDateTime Weather::METAR::expiration() const
{
    if (m_rawText.contains(u"NOSIG"_s))
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
    if (m_ICAOCode.isEmpty())
    {
        return false;
    }
    if (!m_location.isValid())
    {
        return false;
    }
    if (!m_observationTime.isValid())
    {
        return false;
    }
    if (!m_qnh.isFinite())
    {
        return false;
    }
    if (m_rawText.isEmpty())
    {
        return false;
    }
    if (!m_wind.isFinite())
    {
        return false;
    }
    if (!m_temperature.isFinite())
    {
        return false;
    }
    if (!m_dewpoint.isFinite())
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


QString Weather::METAR::summary(const Navigation::Aircraft& aircraft, const QDateTime& currentTime) const
{
    // Paranoid safety checks
    if (m_decoder.isNull())
    {
        return {};
    }

    QStringList resultList;

    switch (m_flightCategory)
    {
    case VFR:
        if (m_rawText.contains(u"CAVOK"_s))
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
    if (m_gust.isFinite() && m_gust.toKN() > 15)
    {
        resultList << tr("gusts of %1").arg(aircraft.horizontalSpeedToString(m_gust));
    }
    else if (m_wind.isFinite() && m_wind.toKN() > 10)
    {
        resultList << tr("wind at %1").arg(aircraft.horizontalSpeedToString(m_wind));
    }

    // Weather
    auto curWeather = m_decoder->currentWeather();
    if (!curWeather.isEmpty())
    {
        resultList << curWeather;
    }

    if (resultList.isEmpty())
    {
        return tr("METAR %1").arg(Navigation::Clock::describeTimeDifference(m_observationTime, currentTime));
    }

    return tr("METAR %1: %2").arg(Navigation::Clock::describeTimeDifference(m_observationTime, currentTime), resultList.join(QStringLiteral(" • ")));
}


QString Weather::METAR::derivedData(const Navigation::Aircraft& aircraft, bool showPerformanceWarning, bool explainPerformanceWarning) const
{
    // Paranoid safety checks
    if (m_decoder.isNull())
    {
        return {};
    }

    QStringList items;

    // Density altitude
    if (m_densityAltitude.isFinite())
    {
        Units::Distance const altitude = Units::Distance::fromM(m_location.altitude());
        auto result = tr("Density Altitude: %1").arg(aircraft.verticalDistanceToString(m_densityAltitude));
        if (altitude.isFinite())
        {
            if (m_densityAltitude > altitude)
            {
                result = tr("Density Altitude: %1, %2 above airfield elevation").arg(aircraft.verticalDistanceToString(m_densityAltitude), aircraft.verticalDistanceToString(m_densityAltitude - altitude));
            }
            else
            {
                result = tr("Density Altitude: %1, %2 below airfield elevation").arg(aircraft.verticalDistanceToString(m_densityAltitude), aircraft.verticalDistanceToString(altitude - m_densityAltitude));
            }
        }
        items += result;
    }

    // Relative humidity
    auto relativeHumidity = Navigation::Atmosphere::relativeHumidity(m_temperature, m_dewpoint);
    if (!std::isnan(relativeHumidity))
    {
        items += tr("Relative Humidity: %1%").arg(qRound(relativeHumidity));
    }

    // Performance warnings
    bool performanceWarningsShown = false;
    if (m_densityAltitude.isFinite() && showPerformanceWarning)
    {
        // Koch chart approximation formulas based on https://groups.google.com/g/rec.aviation.piloting/c/SDlMioBVqaA/m/8_x9GYsnGwAJ
        const auto densityAltitudeInFt = m_densityAltitude.toFeet();

        // Estimate 15% increase in takeoff distance per 1000ft density height
        auto takeoffDistIncPercentage = 0.015 * densityAltitudeInFt;

        // Estimate 7.5% decrease in climb rate per 1000ft density height
        auto rateOfClimbDecreasePercentage = 0.0075 * densityAltitudeInFt;

        if (takeoffDistIncPercentage > 25)
        {
            performanceWarningsShown = true;
            items += "<strong>"
                     + tr("Performance")
                     + ":</strong> "
                     + tr("Expect %1\% increase in takeoff distance").arg(qRound(takeoffDistIncPercentage));
        }

        if (rateOfClimbDecreasePercentage > 25)
        {
            performanceWarningsShown = true;
            if (rateOfClimbDecreasePercentage <= 90)
            {
                items += "<strong>"
                         + tr("Performance")
                         + ":</strong> "
                         + tr("Expect %1\% decrease in climb rate").arg(qRound(rateOfClimbDecreasePercentage));
            }
            else
            {
                items += "<strong>"
                         + tr("Performance")
                         + ":</strong> "
                         + tr("Expect drastic decrease in climb rate. Flying might be inadvisable.");
            }
        }
    }

    if (items.isEmpty())
    {
        return {};
    }

    QString result;
    result += u"<strong>"_s + tr("Derived Data") + u"</strong>"_s;
    result += QStringLiteral("<ul style=\"margin-left:-25px;\">");
    for(auto& item : items)
    {
        result += u"<li>"_s;
        result += item;
        result += u"</li>"_s;
    }
    result += QStringLiteral("</ul>");

    if (performanceWarningsShown)
    {
        if (explainPerformanceWarning)
        {
            result += u"<p>"_s
                      + tr("Percentages are rough estimates, comparing performance of typical SEP aircraft at density altitude to standard sea level values. "
                           "Runway conditions might further degrade performance. "
                           "Always consult the flight manual for exact values.")
                      + u" <a href='hideExplanation'>"_s + tr("Hide this explanation.") + u"</a>"_s
                      + u"</p>"_s;
        }
        else
        {
            result += u"<p><a href='hidePerformanceWarning'>"_s + tr("Hide performance warnings.") + u"</a></p>"_s;
        }
    }
    return result;
}


QDataStream& Weather::operator<<(QDataStream& outstream, const Weather::METAR& metar)
{
    outstream << metar.m_flightCategory;
    outstream << metar.m_ICAOCode;
    outstream << metar.m_location;
    outstream << metar.m_observationTime;
    outstream << metar.m_qnh;
    outstream << metar.m_rawText;
    outstream << metar.m_wind;
    outstream << metar.m_gust;
    outstream << metar.m_temperature;
    outstream << metar.m_dewpoint;
    outstream << metar.m_densityAltitude;

    return outstream;
}


QDataStream& Weather::operator>>(QDataStream& instream, Weather::METAR& metar)
{
    instream >> metar.m_flightCategory;
    instream >> metar.m_ICAOCode;
    instream >> metar.m_location;
    instream >> metar.m_observationTime;
    instream >> metar.m_qnh;
    instream >> metar.m_rawText;
    instream >> metar.m_wind;
    instream >> metar.m_gust;
    instream >> metar.m_temperature;
    instream >> metar.m_dewpoint;
    instream >> metar.m_densityAltitude;

    // Interpret the METAR message
    metar.m_decoder = QSharedPointer<Weather::Decoder>(new Weather::Decoder(metar.m_rawText, metar.m_observationTime.date()));

    return instream;
}
