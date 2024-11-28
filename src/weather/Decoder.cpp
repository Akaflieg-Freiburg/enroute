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

/* This is a heavily altered version of a file from the metaf library package
 * Copyright (C) 2018-2020 Nick Naumenko (https://gitlab.com/nnaumenko)
 * Distributed under the terms of the MIT license.
 */


#include <QDebug>
#include <QTimeZone>
#include <gsl/gsl>

#include "GlobalObject.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "weather/Decoder.h"

using namespace Qt::Literals::StringLiterals;


Weather::Decoder::Decoder()
{
}


void Weather::Decoder::setRawText(const QString& rawText, QDate referenceDate)
{
    m_referenceDate = referenceDate;
    m_parseResult = metaf::Parser::parse(rawText.toStdString());

#warning This use lazy computing here
    (void)decodedText();
}


QString Weather::Decoder::decodedText()
{
    QStringList decodedStrings;
    decodedStrings.reserve(64);
    QString const listStart = QStringLiteral("<ul style=\"margin-left:-25px;\">");
    QString const listEnd = QStringLiteral("</ul>");
    for (const auto& groupInfo : m_parseResult.groups)
    {
        auto decodedString = visit(groupInfo);
        if (decodedString.contains(u"<strong>"_s))
        {
            decodedStrings << listEnd+"<li>"+decodedString+"</li>"+listStart;
        }
        else
        {
            decodedStrings << "<li>"+decodedString+"</li>";
        }
    }

    return listStart+decodedStrings.join(QStringLiteral("\n"))+listEnd;
}


// explanation Methods

QString Weather::Decoder::explainCloudType(const metaf::CloudType &ct)
{
    const auto h = ct.height();
    if (h.isReported())
    {
        return QObject::tr("Cloud cover %1/8, %2, base height %3")
                .arg(ct.okta())
                .arg(cloudTypeToString(ct.type()),
                     explainDistance_FT(ct.height()));
    }
    return QObject::tr("Cloud cover %1/8, %2")
            .arg(ct.okta())
            .arg(cloudTypeToString(ct.type()));
}

QString Weather::Decoder::explainDirection(metaf::Direction direction, bool trueCardinalDirections)
{
    switch (direction.type())
    {
    case metaf::Direction::Type::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::Direction::Type::VARIABLE:
        return QObject::tr("variable");

    case metaf::Direction::Type::NDV:
        return QObject::tr("no directional variation");

    case metaf::Direction::Type::VALUE_DEGREES:
        if (const auto d = direction.degrees(); d.has_value())
        {
            return QStringLiteral("%1°").arg(*d);
        }
        return QObject::tr("[unable to produce value in °]");

    case metaf::Direction::Type::VALUE_CARDINAL:
        if (auto c = cardinalDirectionToString(direction.cardinal(trueCardinalDirections)); !c.isEmpty())
        {
            if (direction.type() == metaf::Direction::Type::VALUE_DEGREES)
            {
                return QStringLiteral("(%1)").arg(c);
            }
            return c;
        }
        break;

    case metaf::Direction::Type::OVERHEAD:
        return QObject::tr("overhead");

    case metaf::Direction::Type::ALQDS:
        return QObject::tr("all quadrants (all directions)");

    case metaf::Direction::Type::UNKNOWN:
        return QObject::tr("unknown direction");
    }

    return {};
}

QString Weather::Decoder::explainDirectionSector(const std::vector<metaf::Direction>& dir)
{
    std::string result;
    for (auto i=0U; i<dir.size(); i++)
    {
        if (i != 0U)
        {
            result += ", ";
        }
        result += cardinalDirectionToString(dir[i].cardinal()).toStdString();
    }
    return QString::fromStdString(result);
}

QString Weather::Decoder::explainDistance(metaf::Distance distance)
{
    if (!distance.isReported())
    {
        return QObject::tr("not reported");
    }

    QStringList results;
    switch (distance.modifier())
    {
    case metaf::Distance::Modifier::NONE:
        break;

    case metaf::Distance::Modifier::LESS_THAN:
        results << QStringLiteral("<");
        break;

    case metaf::Distance::Modifier::MORE_THAN:
        results << QStringLiteral(">");
        break;

    case metaf::Distance::Modifier::DISTANT:
        switch (GlobalObject::navigator()->aircraft().horizontalDistanceUnit())
        {
        case Navigation::Aircraft::Kilometer:
            results << QObject::tr("19 to 55 km");
            break;
        case Navigation::Aircraft::StatuteMile:
            results << QObject::tr("12 to 35 mil");
            break;
        case Navigation::Aircraft::NauticalMile:
            results << QObject::tr("10 to 30 nm");
            break;
        }
        break;

    case metaf::Distance::Modifier::VICINITY:
        switch (GlobalObject::navigator()->aircraft().horizontalDistanceUnit())
        {
        case Navigation::Aircraft::Kilometer:
            results << QObject::tr("9 to 19 km");
            break;
        case Navigation::Aircraft::StatuteMile:
            results << QObject::tr("6 to 12 mil");
            break;
        case Navigation::Aircraft::NauticalMile:
            results << QObject::tr("5 to 10 nm");
            break;
        }
        break;
    }

    if (!distance.isValue())
    {
        return results.join(QStringLiteral(" "));
    }

    // Give distance in natural units
    if (distance.unit() == metaf::Distance::Unit::STATUTE_MILES)
    {
        const auto d = distance.miles();
        if (!d.has_value()) {
            return QObject::tr("[unable to get distance value in miles]");
        }
        const auto integer = std::get<unsigned int>(d.value());
        const auto fraction = std::get<metaf::Distance::MilesFraction>(d.value());
        if ((integer != 0U) || fraction == metaf::Distance::MilesFraction::NONE)
        {
            results << QString::number(integer);
        }
        if (fraction != metaf::Distance::MilesFraction::NONE)
        {
            results << distanceMilesFractionToString(fraction);
        }
        results << distanceUnitToString(distance.unit());
    }
    else
        if (distance.unit() == metaf::Distance::Unit::METERS)
        {
            const auto d = distance.toUnit(metaf::Distance::Unit::METERS);
            if (d.has_value())
            {
                if ((*d) < 5000)
                {
                    results << QStringLiteral("%1 m").arg(qRound(*d));
                }
                else
                {
                    results << QStringLiteral("%1 km").arg( QString::number( *d/1000.0, 'f', 1));
                }
            }
            else
            {
                results << QObject::tr("[unable to convert distance to meters]");
            }
        }
        else
        {
            const auto d = distance.toUnit(distance.unit());
            if (!d.has_value())
            {
                return QObject::tr("[unable to get distance's floating-point value]");
            }
            results << QString::number(qRound(*d));
            results << distanceUnitToString(distance.unit());
        }

    if ((GlobalObject::navigator()->aircraft().horizontalDistanceUnit() == Navigation::Aircraft::Kilometer) && (distance.unit() != metaf::Distance::Unit::METERS))
    {
        const auto d = distance.toUnit(metaf::Distance::Unit::METERS);
        if (d.has_value())
        {
            if ((*d) < 5000)
            {
                results << QStringLiteral("(%1 m)").arg(qRound(*d));
            }
            else
            {
                results << QStringLiteral("(%1 km)").arg( QString::number( *d/1000.0, 'f', 1));
            }
        }
        else
        {
            results << QObject::tr("[unable to convert distance to meters]");
        }
    }
    return results.join(QStringLiteral(" "));
}

QString Weather::Decoder::explainDistance_FT(metaf::Distance distance)
{

    if (!distance.isReported())
    {
        return QObject::tr("not reported");
    }

    QString modifier;
    switch (distance.modifier())
    {
    case metaf::Distance::Modifier::LESS_THAN:
        modifier = QStringLiteral("≤ ");
        break;

    case metaf::Distance::Modifier::MORE_THAN:
        modifier = QStringLiteral("≥ ");
        break;

    default:
        break;
    }

    if (!distance.isValue())
    {
        return QObject::tr("no value");
    }

    const auto d = distance.toUnit(metaf::Distance::Unit::FEET);
    if (d.has_value())
    {
        return modifier + QStringLiteral("%1 ft").arg(qRound(*d));
    }

    return QStringLiteral("[unable to convert distance to feet]");
}

QString Weather::Decoder::explainMetafTime(metaf::MetafTime metafTime)
{
    // QTime for result
    auto metafQTime = QTime(gsl::narrow_cast<int>(metafTime.hour()), gsl::narrow_cast<int>(metafTime.minute()) );

    auto currentQDate = QDate::currentDate().addDays(5);
    auto currentDate = metaf::MetafTime::Date(currentQDate.year(), currentQDate.month(), currentQDate.day());
    if (m_referenceDate.isValid())
    {
        currentDate = metaf::MetafTime::Date(m_referenceDate.year(), m_referenceDate.month(), m_referenceDate.day());
    }
    auto metafDate = metafTime.dateBeforeRef(currentDate);
    auto metafQDate = QDate(gsl::narrow_cast<int>(metafDate.year), gsl::narrow_cast<int>(metafDate.month), gsl::narrow_cast<int>(metafDate.day) );

    auto metafQDateTime = QDateTime(metafQDate, metafQTime, QTimeZone::utc());
    return Navigation::Clock::describePointInTime(metafQDateTime);
}

QString Weather::Decoder::explainPrecipitation(metaf::Precipitation precipitation)
{
    if (!precipitation.isReported())
    {
        return QStringLiteral("not reported");
    }

    if (const auto p = precipitation.amount(); p.has_value() && (*p == 0.0F))
    {
        return QObject::tr("trace amount");
    }

    const auto p = precipitation.toUnit(metaf::Precipitation::Unit::MM);
    if (p.has_value())
    {
        return QStringLiteral("%1 mm").arg(QString::number(*p, 'f', 2));
    }
    return QObject::tr("[unable to convert precipitation to mm]");
}

QString Weather::Decoder::explainPressure(metaf::Pressure pressure)
{

    if (!pressure.pressure().has_value())
    {
        return QObject::tr("not reported");
    }

    const auto phpa = pressure.toUnit(metaf::Pressure::Unit::HECTOPASCAL);
    if (phpa.has_value())
    {
        return QStringLiteral("%1 hPa").arg(qRound(*phpa));
    }
    return QObject::tr("[unable to convert pressure to hPa]");
}

QString Weather::Decoder::explainRunway(metaf::Runway runway)
{
    if (runway.isAllRunways())
    {
        return QObject::tr("all runways");
    }
    if (runway.isMessageRepetition()) {
        return QObject::tr("same runway (repetition of last message)");
    }

    switch(runway.designator())
    {
    case metaf::Runway::Designator::NONE:
        return QObject::tr("runway %1").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::LEFT:
        return QObject::tr("runway %1 LEFT").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::CENTER:
        return QObject::tr("runway %1 CENTER").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::RIGHT:
        return QObject::tr("runway %1 RIGHT").arg(runway.number(), 2, 10, QChar('0'));

    }
    return {};
}

QString Weather::Decoder::explainSpeed(metaf::Speed speed)
{

    if (const auto s = speed.speed(); !s.has_value())
    {
        return QObject::tr("not reported");
    }

    if (GlobalObject::navigator()->aircraft().horizontalDistanceUnit() == Navigation::Aircraft::Kilometer)
    {
        const auto s = speed.toUnit(metaf::Speed::Unit::KILOMETERS_PER_HOUR);
        if (s.has_value())
        {
            return QStringLiteral("%1 km/h").arg(qRound(*s));
        }
        return QObject::tr("[unable to convert speed to km/h]");
    }

    const auto s = speed.toUnit(metaf::Speed::Unit::KNOTS);
    if (s.has_value())
    {
        return QStringLiteral("%1 kt").arg(qRound(*s));
    }
    return QObject::tr("[unable to convert speed to knots]");
}

QString Weather::Decoder::explainSurfaceFriction(metaf::SurfaceFriction surfaceFriction)
{
    const auto c = surfaceFriction.coefficient();

    switch (surfaceFriction.type())
    {
    case metaf::SurfaceFriction::Type::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::SurfaceFriction::Type::SURFACE_FRICTION_REPORTED:
        if (c.has_value())
        {
            return QObject::tr("friction coefficient %1").arg(QString::number(*c, 'f', 2));
        }
        return QObject::tr("[unable to produce a friction coefficient]");

    case metaf::SurfaceFriction::Type::BRAKING_ACTION_REPORTED:
        return QObject::tr("braking action %1").arg(brakingActionToString(surfaceFriction.brakingAction()));

    case metaf::SurfaceFriction::Type::UNRELIABLE:
        return QObject::tr("unreliable or unmeasurable");
    }
    return {};
}

QString Weather::Decoder::explainTemperature(metaf::Temperature temperature)
{
    const auto temp = temperature.temperature();
    if (!temp.has_value())
    {
        return QObject::tr("not reported");
    }

    QString temperatureString = QObject::tr("[unable to convert temperature to °C]");
    const auto t = temperature.toUnit(metaf::Temperature::Unit::C);
    if (t.has_value())
    {
        temperatureString = QStringLiteral("%1 °C").arg(qRound(*t));
    }

    if ((temp == 0.0F) && !temperature.isPrecise())
    {
        if (temperature.isFreezing())
        {
            return QObject::tr("slightly less than %1").arg(temperatureString);
        }
        if (!temperature.isFreezing()) {
            return QObject::tr("slightly more than %1").arg(temperatureString);
        }
    }

    return temperatureString;
}

QString Weather::Decoder::explainWaveHeight(metaf::WaveHeight waveHeight)
{
    switch (waveHeight.type()) {
    case metaf::WaveHeight::Type::STATE_OF_SURFACE:
        return QObject::tr("state of sea surface: %1").arg(stateOfSeaSurfaceToString(waveHeight.stateOfSurface()));

    case metaf::WaveHeight::Type::WAVE_HEIGHT:
        if (waveHeight.isReported())
        {
            const auto h = waveHeight.toUnit(metaf::WaveHeight::Unit::METERS);
            if (h.has_value())
            {
                return QObject::tr("wave height: %1 m").arg(qRound(*h));
            }
            return QObject::tr("[unable to convert wave height to meters]");
        }
        return QObject::tr("wave height not reported");
    }
    return {};
}

QString Weather::Decoder::explainWeatherPhenomena(const metaf::WeatherPhenomena & wp)
{
    /* Handle special cases */
    auto weatherStr = Weather::Decoder::specialWeatherPhenomenaToString(wp);
    if (!weatherStr.isEmpty())
    {
        return weatherStr;
    }

    // Obtain strings for qualifier & descriptor
    auto qualifier = Weather::Decoder::weatherPhenomenaQualifierToString(wp.qualifier()); // Qualifier, such as "light" or "moderate"
    auto descriptor = Weather::Decoder::weatherPhenomenaDescriptorToString(wp.descriptor()); // Descriptor, such as "freezing" or "blowing"

    // String that will hold the result
    QString result;

    QStringList weatherPhenomena;
    weatherPhenomena.reserve(8);
    for (const auto w : wp.weather())
    {
        // This is a string such as "hail" or "rain"
        auto wpString = Weather::Decoder::weatherPhenomenaWeatherToString(w);
        if (!wpString.isEmpty())
        {
            weatherPhenomena << Weather::Decoder::weatherPhenomenaWeatherToString(w);
        }
    }
    // Special case: "shower" is used as a phenomenom
    if (weatherPhenomena.isEmpty() && wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS)
    {
        weatherPhenomena << QObject::tr("shower");
        descriptor = QString();
    }
    if (weatherPhenomena.isEmpty() && wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM)
    {
        weatherPhenomena << QObject::tr("thunderstorm");
        descriptor = QString();
    }
    result += weatherPhenomena.join(QStringLiteral(", "));

    // Handle special qualifiers

    if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::RECENT)
    {
        result = QObject::tr("recent %1").arg(result);
        qualifier = QString();
    }
    if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::VICINITY)
    {
        result = QObject::tr("%1 in the vicinity").arg(result);
        qualifier = QString();
    }

    // The remaining descriptors and qualifiers go into a parenthesis text
    QStringList parenthesisTexts;
    if (!qualifier.isEmpty())
    {
        parenthesisTexts << qualifier;
    }
    if (!descriptor.isEmpty())
    {
        parenthesisTexts << descriptor;
    }
    auto parenthesisText = parenthesisTexts.join(QStringLiteral(", "));
    if (!parenthesisText.isEmpty())
    {
        result += QStringLiteral(" (%1)").arg(parenthesisText);
    }

    const auto time = wp.time();
    switch (wp.event()){
    case metaf::WeatherPhenomena::Event::BEGINNING:        
        if (!time.has_value())
        {
            break;
        }
        result += " " + QObject::tr("began:") + " " + Weather::Decoder::explainMetafTime(*time);
        break;

    case metaf::WeatherPhenomena::Event::ENDING:
        if (!time.has_value())
        {
            break;
        }
        result += " " + QObject::tr("ended:") + " " + Weather::Decoder::explainMetafTime(*time);
        break;

    case metaf::WeatherPhenomena::Event::NONE:
        break;
    }

    if (!parenthesisText.isEmpty())
    {
        qWarning() << "Weather phenomena w/o special handling code" << result;
    }

    return result;
}


// …toString Methods

QString Weather::Decoder::brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction)
{
    switch(brakingAction)
    {
    case metaf::SurfaceFriction::BrakingAction::NONE:
        return QObject::tr("not reported");

    case metaf::SurfaceFriction::BrakingAction::POOR:
        return QObject::tr("poor (friction coefficient 0.0 to 0.25)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM_POOR:
        return QObject::tr("medium/poor (friction coefficient 0.26 to 0.29)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM:
        return QObject::tr("medium (friction coefficient 0.30 to 0.35)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM_GOOD:
        return QObject::tr("medium/good (friction coefficient 0.36 to 0.40)");

    case metaf::SurfaceFriction::BrakingAction::GOOD:
        return QObject::tr("good (friction coefficient 0.40 to 1.00)");
    }
    return {};
}

QString Weather::Decoder::cardinalDirectionToString(metaf::Direction::Cardinal cardinal)
{
    switch(cardinal)
    {
    case metaf::Direction::Cardinal::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::Direction::Cardinal::N:
        return QObject::tr("north");

    case metaf::Direction::Cardinal::S:
        return QObject::tr("south");

    case metaf::Direction::Cardinal::W:
        return QObject::tr("west");

    case metaf::Direction::Cardinal::E:
        return QObject::tr("east");

    case metaf::Direction::Cardinal::NW:
        return QObject::tr("northwest");

    case metaf::Direction::Cardinal::NE:
        return QObject::tr("northeast");

    case metaf::Direction::Cardinal::SW:
        return QObject::tr("southwest");

    case metaf::Direction::Cardinal::SE:
        return QObject::tr("southeast");

    case metaf::Direction::Cardinal::TRUE_N:
        return QObject::tr("true north");

    case metaf::Direction::Cardinal::TRUE_W:
        return QObject::tr("true west");

    case metaf::Direction::Cardinal::TRUE_S:
        return QObject::tr("true south");

    case metaf::Direction::Cardinal::TRUE_E:
        return QObject::tr("true east");

    case metaf::Direction::Cardinal::NDV:
        return QObject::tr("no directional variations");

    case metaf::Direction::Cardinal::VRB:
        return QStringLiteral("variable");

    case metaf::Direction::Cardinal::OHD:
        return QStringLiteral("overhead");

    case metaf::Direction::Cardinal::ALQDS:
        return QStringLiteral("all quadrants (in all directions)");

    case metaf::Direction::Cardinal::UNKNOWN:
        return QStringLiteral("unknown direction");
    }
    return {};
}

QString Weather::Decoder::cloudAmountToString(metaf::CloudGroup::Amount amount)
{
    switch (amount)
    {
    case metaf::CloudGroup::Amount::NOT_REPORTED:
        return QObject::tr("Cloud amount not reported");

    case metaf::CloudGroup::Amount::NSC:
        return QObject::tr("No significant cloud");

    case metaf::CloudGroup::Amount::NCD:
        return QObject::tr("No cloud detected");

    case metaf::CloudGroup::Amount::NONE_CLR:
    case metaf::CloudGroup::Amount::NONE_SKC:
        return QObject::tr("Clear sky");

    case metaf::CloudGroup::Amount::FEW:
        return QObject::tr("Few clouds");

    case metaf::CloudGroup::Amount::SCATTERED:
        return QObject::tr("Scattered clouds");

    case metaf::CloudGroup::Amount::BROKEN:
        return QObject::tr("Broken clouds");

    case metaf::CloudGroup::Amount::OVERCAST:
        return QObject::tr("Overcast clouds");

    case metaf::CloudGroup::Amount::OBSCURED:
        return QObject::tr("Sky obscured");

    case metaf::CloudGroup::Amount::VARIABLE_FEW_SCATTERED:
        return QObject::tr("Few -- scattered clouds");

    case metaf::CloudGroup::Amount::VARIABLE_SCATTERED_BROKEN:
        return QObject::tr("Scattered -- broken clouds");

    case metaf::CloudGroup::Amount::VARIABLE_BROKEN_OVERCAST:
        return QObject::tr("Broken -- overcast clouds");
    }
    return {};
}

QString Weather::Decoder::cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer)
{
    switch(highLayer)
    {
    case metaf::LowMidHighCloudGroup::HighLayer::NONE:
        return QObject::tr("No high-layer clouds");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC:
        return QObject::tr("Cirrus fibratus or Cirrus uncinus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CI_CAS_CI_FLO:
        return QObject::tr("Cirrus spissatus or Cirrus castellanus or Cirrus floccus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CBGEN:
        return QObject::tr("Cirrus spissatus cumulonimbogenitus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC_SPREADING:
        return QObject::tr("Cirrus uncinus or Cirrus fibratus progressively invading the sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_LOW_ABOVE_HORIZON:
        return QObject::tr("Cirrus or Cirrostratus progressively invading the sky, but the continuous veil does not reach 45° above the horizon");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_HIGH_ABOVE_HORIZON:
        return QObject::tr("Cirrus or Cirrostratus progressively invading the sky, the continuous veil extends more than 45° above the horizon, without the sky being totally covered");

    case metaf::LowMidHighCloudGroup::HighLayer::CS_NEB_CS_FIB_COVERING_ENTIRE_SKY:
        return QObject::tr("Cirrostratus nebulosus or Cirrostratus fibratus covering the whole sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CS:
        return QObject::tr("Cirrostratus that is not invading the sky and that does not completely cover the whole sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CC:
        return QObject::tr("Cirrocumulus alone");

    case metaf::LowMidHighCloudGroup::HighLayer::NOT_OBSERVABLE:
        return QObject::tr("Clouds are not observable");
    }
    return {};
}

QString Weather::Decoder::cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer)
{
    switch(lowLayer)
    {
    case metaf::LowMidHighCloudGroup::LowLayer::NONE:
        return QObject::tr("No low layer clouds");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_HU_CU_FR:
        return QObject::tr("Cumulus humilis or Cumulus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_MED_CU_CON:
        return QObject::tr("Cumulus clouds with moderate or significant vertical extent");

    case metaf::LowMidHighCloudGroup::LowLayer::CB_CAL:
        return QObject::tr("Cumulonimbus calvus");

    case metaf::LowMidHighCloudGroup::LowLayer::SC_CUGEN:
        return QObject::tr("Stratocumulus cumulogenitus");

    case metaf::LowMidHighCloudGroup::LowLayer::SC_NON_CUGEN:
        return QObject::tr("Stratocumulus non-cumulogenitus");

    case metaf::LowMidHighCloudGroup::LowLayer::ST_NEB_ST_FR:
        return QObject::tr("Stratus nebulosus or Stratus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::ST_FR_CU_FR_PANNUS:
        return QObject::tr("Stratus fractus or Cumulus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_SC_NON_CUGEN_DIFFERENT_LEVELS:
        return QObject::tr("Cumulus and Stratocumulus with bases at different levels");

    case metaf::LowMidHighCloudGroup::LowLayer::CB_CAP:
        return QStringLiteral("Cumulonimbus capillatus or Cumulonimbus capillatus incus)");

    case metaf::LowMidHighCloudGroup::LowLayer::NOT_OBSERVABLE:
        return QObject::tr("Clouds are not observable due to fog, blowing dust or sand, or other similar phenomena");
    }
    return {};
}

QString Weather::Decoder::cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer)
{
    switch(midLayer)
    {
    case metaf::LowMidHighCloudGroup::MidLayer::NONE:
        return QObject::tr("No mid-layer clouds");

    case metaf::LowMidHighCloudGroup::MidLayer::AS_TR:
        return QObject::tr("Altostratus translucidus");

    case metaf::LowMidHighCloudGroup::MidLayer::AS_OP_NS:
        return QObject::tr("Altostratus opacus or Nimbostratus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR:
        return QObject::tr("Altocumulus translucidus at a single level");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_LEN_PATCHES:
        return QObject::tr("Patches of Altocumulus translucidus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_AC_OP_SPREADING:
        return QObject::tr("Altocumulus translucidus in bands");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_CUGEN_AC_CBGEN:
        return QObject::tr("Altocumulus cumulogenitus or Altocumulus cumulonimbogenitus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_DU_AC_OP_AC_WITH_AS_OR_NS:
        return QObject::tr("Altocumulus duplicatus, or Altocumulus opacus in a single layer");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_CAS_AC_FLO:
        return QObject::tr("Altocumulus castellanus or Altocumulus floccus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_OF_CHAOTIC_SKY:
        return QObject::tr("Broken cloud sheets of ill-defined species or varieties");

    case metaf::LowMidHighCloudGroup::MidLayer::NOT_OBSERVABLE:
        return QObject::tr("Clouds are not observable");
    }
    return {};
}

QString Weather::Decoder::cloudTypeToString(metaf::CloudType::Type type)
{
    switch(type)
    {
    case metaf::CloudType::Type::NOT_REPORTED:
        return QObject::tr("unknown cloud type");

    case metaf::CloudType::Type::CUMULONIMBUS:
        return QObject::tr("cumulonimbus");

    case metaf::CloudType::Type::TOWERING_CUMULUS:
        return QObject::tr("towering cumulus");

    case metaf::CloudType::Type::CUMULUS:
        return QObject::tr("cumulus");

    case metaf::CloudType::Type::CUMULUS_FRACTUS:
        return QObject::tr("cumulus fractus");

    case metaf::CloudType::Type::STRATOCUMULUS:
        return QObject::tr("stratocumulus");

    case metaf::CloudType::Type::NIMBOSTRATUS:
        return QObject::tr("nimbostratus");

    case metaf::CloudType::Type::STRATUS:
        return QObject::tr("stratus");

    case metaf::CloudType::Type::STRATUS_FRACTUS:
        return QObject::tr("stratus fractus");

    case metaf::CloudType::Type::ALTOSTRATUS:
        return QObject::tr("altostratus");

    case metaf::CloudType::Type::ALTOCUMULUS:
        return QObject::tr("altocumulus");

    case metaf::CloudType::Type::ALTOCUMULUS_CASTELLANUS:
        return QObject::tr("altocumulus castellanus");

    case metaf::CloudType::Type::CIRRUS:
        return QObject::tr("cirrus");

    case metaf::CloudType::Type::CIRROSTRATUS:
        return QObject::tr("cirrostratus");

    case metaf::CloudType::Type::CIRROCUMULUS:
        return QObject::tr("cirrocumulus");

    case metaf::CloudType::Type::BLOWING_SNOW:
        return QObject::tr("blowing snow");

    case metaf::CloudType::Type::BLOWING_DUST:
        return QObject::tr("blowing dust");

    case metaf::CloudType::Type::BLOWING_SAND:
        return QObject::tr("blowing sand");

    case metaf::CloudType::Type::ICE_CRYSTALS:
        return QObject::tr("ice crystals");

    case metaf::CloudType::Type::RAIN:
        return QObject::tr("rain");

    case metaf::CloudType::Type::DRIZZLE:
        return QObject::tr("drizzle");

    case metaf::CloudType::Type::SNOW:
        return QObject::tr("snow");

    case metaf::CloudType::Type::ICE_PELLETS:
        return QObject::tr("ice pellets");

    case metaf::CloudType::Type::SMOKE:
        return QObject::tr("smoke");

    case metaf::CloudType::Type::FOG:
        return QObject::tr("fog");

    case metaf::CloudType::Type::MIST:
        return QObject::tr("mist");

    case metaf::CloudType::Type::HAZE:
        return QObject::tr("haze");

    case metaf::CloudType::Type::VOLCANIC_ASH:
        return QObject::tr("volcanic ash");
    }
    return {};
}

QString Weather::Decoder::convectiveTypeToString(metaf::CloudGroup::ConvectiveType type)
{
    switch (type)
    {
    case metaf::CloudGroup::ConvectiveType::NONE:
        return {};

    case metaf::CloudGroup::ConvectiveType::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::CloudGroup::ConvectiveType::TOWERING_CUMULUS:
        return QObject::tr("towering cumulus");

    case metaf::CloudGroup::ConvectiveType::CUMULONIMBUS:
        return QObject::tr("cumulonimbus");
    }
    return {};
}

QString Weather::Decoder::distanceMilesFractionToString(metaf::Distance::MilesFraction f)
{
    switch (f)
    {
    case metaf::Distance::MilesFraction::NONE:
        return u""_s;

    case metaf::Distance::MilesFraction::F_1_16:
        return QStringLiteral("1/16");

    case metaf::Distance::MilesFraction::F_1_8:
        return QStringLiteral("1/8");

    case metaf::Distance::MilesFraction::F_3_16:
        return QStringLiteral("3/16");

    case metaf::Distance::MilesFraction::F_1_4:
        return QStringLiteral("1/4");

    case metaf::Distance::MilesFraction::F_5_16:
        return QStringLiteral("5/16");

    case metaf::Distance::MilesFraction::F_3_8:
        return QStringLiteral("3/8");

    case metaf::Distance::MilesFraction::F_1_2:
        return QStringLiteral("1/2");

    case metaf::Distance::MilesFraction::F_5_8:
        return QStringLiteral("5/8");

    case metaf::Distance::MilesFraction::F_3_4:
        return QStringLiteral("3/4");

    case metaf::Distance::MilesFraction::F_7_8:
        return QStringLiteral("7/8");
    }
    return {};
}

QString Weather::Decoder::distanceUnitToString(metaf::Distance::Unit unit)
{
    switch (unit)
    {
    case metaf::Distance::Unit::METERS:
        return QStringLiteral("m");

    case metaf::Distance::Unit::STATUTE_MILES:
        return QObject::tr("statute miles");

    case metaf::Distance::Unit::FEET:
        return QStringLiteral("ft");
    }
    return {};
}

QString Weather::Decoder::layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type)
{
    switch(type)
    {
    case metaf::LayerForecastGroup::Type::ICING_TRACE_OR_NONE:
        return QObject::tr("Trace icing or no icing");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_MIXED:
        return QObject::tr("Light mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_RIME_IN_CLOUD:
        return QObject::tr("Light rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_CLEAR_IN_PRECIPITATION:
        return QObject::tr("Light clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_MIXED:
        return QObject::tr("Moderate mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_RIME_IN_CLOUD:
        return QObject::tr("Moderate rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_CLEAR_IN_PRECIPITATION:
        return QObject::tr("Moderate clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_MIXED:
        return QObject::tr("Severe mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_RIME_IN_CLOUD:
        return QObject::tr("Severe rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_CLEAR_IN_PRECIPITATION:
        return QObject::tr("Severe clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::TURBULENCE_NONE:
        return QObject::tr("No turbulence");

    case metaf::LayerForecastGroup::Type::TURBULENCE_LIGHT:
        return QObject::tr("Light turbulence");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLEAR_AIR_OCCASIONAL:
        return QObject::tr("Occasional moderate turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLEAR_AIR_FREQUENT:
        return QObject::tr("Frequent moderate turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLOUD_OCCASIONAL:
        return QObject::tr("Occasional moderate turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLOUD_FREQUENT:
        return QObject::tr("Frequent moderate turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLEAR_AIR_OCCASIONAL:
        return QObject::tr("Occasional severe turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLEAR_AIR_FREQUENT:
        return QObject::tr("Frequent severe turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLOUD_OCCASIONAL:
        return QObject::tr("Occasional severe turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLOUD_FREQUENT:
        return QObject::tr("Frequent severe turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_EXTREME:
        return QObject::tr("Extreme turbulence");
    }
    return {};
}

QString Weather::Decoder::pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend)
{
    switch(trend)
    {
    case metaf::PressureTendencyGroup::Trend::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::PressureTendencyGroup::Trend::HIGHER:
        return QObject::tr("higher than");

    case metaf::PressureTendencyGroup::Trend::HIGHER_OR_SAME:
        return QObject::tr("higher or the same as");

    case metaf::PressureTendencyGroup::Trend::SAME:
        return QObject::tr("same as");

    case metaf::PressureTendencyGroup::Trend::LOWER_OR_SAME:
        return QObject::tr("lower or the same as");

    case metaf::PressureTendencyGroup::Trend::LOWER:
        return QObject::tr("lower than");
    }
    return {};
}

QString Weather::Decoder::pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type)
{
    switch(type)
    {
    case metaf::PressureTendencyGroup::Type::INCREASING_THEN_DECREASING:
        return QObject::tr("increasing, then decreasing");

    case metaf::PressureTendencyGroup::Type::INCREASING_MORE_SLOWLY:
        return QObject::tr("increasing more slowly");

    case metaf::PressureTendencyGroup::Type::INCREASING:
        return QObject::tr("increasing");

    case metaf::PressureTendencyGroup::Type::INCREASING_MORE_RAPIDLY:
        return QObject::tr("increasing more rapidly");

    case metaf::PressureTendencyGroup::Type::STEADY:
        return QObject::tr("steady");

    case metaf::PressureTendencyGroup::Type::DECREASING_THEN_INCREASING:
        return QObject::tr("decreasing, then increasing");

    case metaf::PressureTendencyGroup::Type::DECREASING_MORE_SLOWLY:
        return QObject::tr("decreasing more slowly");

    case metaf::PressureTendencyGroup::Type::DECREASING:
        return QObject::tr("decreasing");

    case metaf::PressureTendencyGroup::Type::DECREASING_MORE_RAPIDLY:
        return QObject::tr("decreasing more rapidly");

    case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
        return QObject::tr("rising rapidly");

    case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
        return QObject::tr("falling rapidly");
    }
    return {};
}

QString Weather::Decoder::probabilityToString(metaf::TrendGroup::Probability prob)
{
    switch (prob)
    {
    case metaf::TrendGroup::Probability::PROB_30:
        return QObject::tr("Probability 30%");

    case metaf::TrendGroup::Probability::PROB_40:
        return QObject::tr("Probability 40%");

    case metaf::TrendGroup::Probability::NONE:
        return {};
    }
    return {};
}

QString Weather::Decoder::runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits)
{
    switch(deposits)
    {
    case metaf::RunwayStateGroup::Deposits::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY:
        return QObject::tr("clear and dry");

    case metaf::RunwayStateGroup::Deposits::DAMP:
        return QObject::tr("damp");

    case metaf::RunwayStateGroup::Deposits::WET_AND_WATER_PATCHES:
        return QObject::tr("wet and water patches");

    case metaf::RunwayStateGroup::Deposits::RIME_AND_FROST_COVERED:
        return QObject::tr("rime and frost covered");

    case metaf::RunwayStateGroup::Deposits::DRY_SNOW:
        return QObject::tr("dry snow");

    case metaf::RunwayStateGroup::Deposits::WET_SNOW:
        return QObject::tr("wet snow");

    case metaf::RunwayStateGroup::Deposits::SLUSH:
        return QObject::tr("slush");

    case metaf::RunwayStateGroup::Deposits::ICE:
        return QObject::tr("ice");

    case metaf::RunwayStateGroup::Deposits::COMPACTED_OR_ROLLED_SNOW:
        return QObject::tr("compacted or rolled snow");

    case metaf::RunwayStateGroup::Deposits::FROZEN_RUTS_OR_RIDGES:
        return QObject::tr("frozen ruts or ridges");
    }
    return {};
}

QString Weather::Decoder::runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent)
{
    switch(extent)
    {
    case metaf::RunwayStateGroup::Extent::NOT_REPORTED:
    case metaf::RunwayStateGroup::Extent::RESERVED_3:
    case metaf::RunwayStateGroup::Extent::RESERVED_4:
    case metaf::RunwayStateGroup::Extent::RESERVED_6:
    case metaf::RunwayStateGroup::Extent::RESERVED_7:
    case metaf::RunwayStateGroup::Extent::RESERVED_8:
        return QObject::tr("not reported");

    case metaf::RunwayStateGroup::Extent::NONE:
        return QObject::tr("none");

    case metaf::RunwayStateGroup::Extent::LESS_THAN_10_PERCENT:
        return QStringLiteral("< 10%");

    case metaf::RunwayStateGroup::Extent::FROM_11_TO_25_PERCENT:
        return QStringLiteral("11% -- 25%");

    case metaf::RunwayStateGroup::Extent::FROM_26_TO_50_PERCENT:
        return QStringLiteral("26% -- 50%");

    case metaf::RunwayStateGroup::Extent::MORE_THAN_51_PERCENT:
        return QStringLiteral(">51%");
    }
    return {};
}

QString Weather::Decoder::specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp)
{
    QStringList results;
    for (const auto &weather : wp.weather())
    {
        // DRIZZLE
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::DRIZZLE)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("drizzle in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy drizzle");
                break;
            }
            continue;
        }

        // DRIZZLE, FREEZING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::DRIZZLE)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("freezing drizzle in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent freezing drizzle");
                break;
            }
            continue;
        }

        // DUST, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::DUST)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("blowing dust in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent blowing dust");
                break;
            }
            continue;
        }

        // FOG
        if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::NONE && weather == metaf::WeatherPhenomena::Weather::FOG)
        {
            switch(wp.descriptor())
            {
            case metaf::WeatherPhenomena::Descriptor::NONE:
                results << QObject::tr("fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::FREEZING:
                results << QObject::tr("freezing fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::PARTIAL:
                results << QObject::tr("partial fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::PATCHES:
                results << QObject::tr("patches of fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::SHALLOW:
                results << QObject::tr("shallow fog");
                break;
            default:
                return {};
            }
            continue;
        }

        // HAIL, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::HAIL)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("hail showers in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent hail showers");
                break;
            }
            continue;
        }

        // HAIL, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::HAIL)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("thunderstorm with hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent thunderstorm with hail");
                break;
            }
            continue;
        }

        // ICE PELLETS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::ICE_PELLETS)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("ice pellet precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent ice pellet precipitation");
                break;
            }
            continue;
        }

        // PRECIPITATION
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("precipitation in the vicinity");
                break;
            }
            continue;
        }

        // PRECIPITATION, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("showers in the vicinity with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy showers with undetermined precipitation");
                break;
            }
            continue;
        }

        // PRECIPITATION, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("thunderstorm with precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy thunderstorm with precipitation");
                break;
            }
            continue;
        }

        // RAIN
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::RAIN)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("rain in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent rain");
                break;
            }
            continue;
        }

        // RAIN, FREEZING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::RAIN)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("freezing rain in the vicinity");
                break;
            }
            continue;
        }

        // RAIN, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::RAIN)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("rain showers in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent rain showers");
                break;
            }
            continue;
        }

        // RAIN, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::RAIN)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("thunderstorm with rain in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent thunderstorm with rain");
                break;
            }
            continue;
        }

        // SAND, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::SAND)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("blowing sand in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent blowing sand");
                break;
            }
            continue;
        }

        // SAND, LOW DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING && weather == metaf::WeatherPhenomena::Weather::SAND)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("low drifting sand in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent low drifting sand");
                break;
            }
            continue;
        }

        // SNOW
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("snowfall in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent snowfall");
                break;
            }
            continue;
        }

        // SNOW, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("blowing snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent blowing snow");
                break;
            }
            continue;
        }

        // SNOW GRAINS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::SNOW_GRAINS)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("snow grain precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent snow grain precipitation");
                break;
            }
            continue;
        }

        // SNOW, LOW_DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("low drifting snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent low drifting snow");
                break;
            }
            continue;
        }

        // SNOW, LOW_DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("freezing snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent freezing snow");
                break;
            }
            continue;
        }

        // SNOW SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("snow showers in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent snow showers");
                break;
            }
            continue;
        }

        // SNOW, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::SNOW)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("thunderstorm with snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent thunderstorm with snow");
                break;
            }
            continue;
        }

        // SMALL HAIL, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::SMALL_HAIL)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("shower with small hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent shower with small hail");
                break;
            }
            continue;
        }

        // SMALL HAIL, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::SMALL_HAIL)
        {
            switch(wp.qualifier())
            {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << QObject::tr("thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << QObject::tr("thunderstorm with small hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << QObject::tr("light thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << QObject::tr("moderate thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << QObject::tr("heavy thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << QObject::tr("recent thunderstorm with small hail");
                break;
            }
            continue;
        }

        return {};
    }

    return results.join(QStringLiteral(" • "));
}

QString Weather::Decoder::stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface)
{
    switch(stateOfSurface)
    {
    case metaf::WaveHeight::StateOfSurface::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::WaveHeight::StateOfSurface::CALM_GLASSY:
        return QObject::tr("calm (glassy), no waves");

    case metaf::WaveHeight::StateOfSurface::CALM_RIPPLED:
        return QObject::tr("calm (rippled), wave height <0.1 meters");

    case metaf::WaveHeight::StateOfSurface::SMOOTH:
        return QObject::tr("smooth, wave height 0.1 to 0.5 meters");

    case metaf::WaveHeight::StateOfSurface::SLIGHT:
        return QObject::tr("slight, wave height 0.5 to 1.25 meters");

    case metaf::WaveHeight::StateOfSurface::MODERATE:
        return QObject::tr("moderate, wave height 1.25 to 2.5 meters");

    case metaf::WaveHeight::StateOfSurface::ROUGH:
        return QObject::tr("rough, wave height 2.5 to 4 meters");

    case metaf::WaveHeight::StateOfSurface::VERY_ROUGH:
        return QObject::tr("very rough, wave height 4 to 6 meters");

    case metaf::WaveHeight::StateOfSurface::HIGH:
        return QObject::tr("high, wave height 6 to 9 meters");

    case metaf::WaveHeight::StateOfSurface::VERY_HIGH:
        return QObject::tr("very high, wave height 9 to 14 meters");

    case metaf::WaveHeight::StateOfSurface::PHENOMENAL:
        return QObject::tr("phenomenal, wave height >14 meters");
    }
    return {};
}

QString Weather::Decoder::visTrendToString(metaf::VisibilityGroup::Trend trend)
{
    switch(trend)
    {
    case metaf::VisibilityGroup::Trend::NONE:
        return {};

    case metaf::VisibilityGroup::Trend::NOT_REPORTED:
        return QObject::tr("not reported");

    case metaf::VisibilityGroup::Trend::UPWARD:
        //: visibility trend
        return QObject::tr("upward");

    case metaf::VisibilityGroup::Trend::NEUTRAL:
        //: visibility trend
        return QObject::tr("neutral");

    case metaf::VisibilityGroup::Trend::DOWNWARD:
        //: visibility trend
        return QObject::tr("downward");
    }
    return {};
}

QString Weather::Decoder::weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor)
{
    switch(descriptor)
    {
    case metaf::WeatherPhenomena::Descriptor::NONE:
        return {};

    case metaf::WeatherPhenomena::Descriptor::SHALLOW:
        return QObject::tr("shallow");

    case metaf::WeatherPhenomena::Descriptor::PARTIAL:
        return QObject::tr("partial");

    case metaf::WeatherPhenomena::Descriptor::PATCHES:
        return QObject::tr("patches");

    case metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING:
        return QObject::tr("low drifting");

    case metaf::WeatherPhenomena::Descriptor::BLOWING:
        return QObject::tr("blowing");

    case metaf::WeatherPhenomena::Descriptor::SHOWERS:
        return QObject::tr("showers");

    case metaf::WeatherPhenomena::Descriptor::THUNDERSTORM:
        return QObject::tr("thunderstorm");

    case metaf::WeatherPhenomena::Descriptor::FREEZING:
        return QObject::tr("freezing");
    }
    return {};
}

QString Weather::Decoder::weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier)
{
    switch (qualifier)
    {
    case metaf::WeatherPhenomena::Qualifier::NONE:
        return {};

    case metaf::WeatherPhenomena::Qualifier::RECENT:
        return QStringLiteral("recent");

    case metaf::WeatherPhenomena::Qualifier::VICINITY:
        return QStringLiteral("in vicinity");

    case metaf::WeatherPhenomena::Qualifier::LIGHT:
        return QObject::tr("light");

    case metaf::WeatherPhenomena::Qualifier::MODERATE:
        return QObject::tr("moderate");

    case metaf::WeatherPhenomena::Qualifier::HEAVY:
        return QObject::tr("heavy");
    }
    return {};
}

QString Weather::Decoder::weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather)
{
    switch (weather)
    {
    case metaf::WeatherPhenomena::Weather::NOT_REPORTED:
        return {};

    case metaf::WeatherPhenomena::Weather::DRIZZLE:
        return QObject::tr("drizzle");

    case metaf::WeatherPhenomena::Weather::RAIN:
        return QObject::tr("rain");

    case metaf::WeatherPhenomena::Weather::SNOW:
        return QObject::tr("snow");

    case metaf::WeatherPhenomena::Weather::SNOW_GRAINS:
        return QObject::tr("snow grains");

    case metaf::WeatherPhenomena::Weather::ICE_CRYSTALS:
        return QObject::tr("ice crystals");

    case metaf::WeatherPhenomena::Weather::ICE_PELLETS:
        return QObject::tr("ice pellets");

    case metaf::WeatherPhenomena::Weather::HAIL:
        return QObject::tr("hail");

    case metaf::WeatherPhenomena::Weather::SMALL_HAIL:
        return QObject::tr("small hail");

    case metaf::WeatherPhenomena::Weather::UNDETERMINED:
        return QObject::tr("undetermined precipitation");

    case metaf::WeatherPhenomena::Weather::MIST:
        return QObject::tr("mist");

    case metaf::WeatherPhenomena::Weather::FOG:
        return QObject::tr("fog");

    case metaf::WeatherPhenomena::Weather::SMOKE:
        return QObject::tr("smoke");

    case metaf::WeatherPhenomena::Weather::VOLCANIC_ASH:
        return QObject::tr("volcanic ash");

    case metaf::WeatherPhenomena::Weather::DUST:
        return QObject::tr("dust");

    case metaf::WeatherPhenomena::Weather::SAND:
        return QObject::tr("sand");

    case metaf::WeatherPhenomena::Weather::HAZE:
        return QObject::tr("haze");

    case metaf::WeatherPhenomena::Weather::SPRAY:
        return QObject::tr("spray");

    case metaf::WeatherPhenomena::Weather::DUST_WHIRLS:
        return QObject::tr("dust or sand whirls");

    case metaf::WeatherPhenomena::Weather::SQUALLS:
        return QObject::tr("squalls");

    case metaf::WeatherPhenomena::Weather::FUNNEL_CLOUD:
        return QObject::tr("funnel cloud");

    case metaf::WeatherPhenomena::Weather::SANDSTORM:
        return QObject::tr("sand storm");

    case metaf::WeatherPhenomena::Weather::DUSTSTORM:
        return QObject::tr("dust storm");
    }
    return {};
}


// Visitor methods

QString Weather::Decoder::visitCloudGroup(const CloudGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    const auto rw = group.runway();
    const auto d = group.direction();

    switch (group.type())
    {
    case metaf::CloudGroup::Type::NO_CLOUDS:
        return cloudAmountToString(group.amount());

    case metaf::CloudGroup::Type::CLOUD_LAYER:
        if (group.convectiveType() != metaf::CloudGroup::ConvectiveType::NONE)
        {
            return QObject::tr("%1 (%2) in %3 AGL")
                    .arg(cloudAmountToString(group.amount()),
                         convectiveTypeToString(group.convectiveType()),
                         explainDistance_FT(group.height()));
        }
        return QObject::tr("%1 in %2 AGL")
                .arg(cloudAmountToString(group.amount()),
                     explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VERTICAL_VISIBILITY:
        return QObject::tr("Vertical visibility %1")
                .arg(explainDistance_FT(group.verticalVisibility()));

    case metaf::CloudGroup::Type::CEILING:
        if (rw.has_value() && d.has_value())
        {
            return QObject::tr("Ceiling height %1 AGL at %2 towards %3")
                    .arg(explainDistance_FT(group.height()),
                         explainRunway(*rw),
                         explainDirection(*d));
        }
        if (rw.has_value())
        {
            return QObject::tr("Ceiling height %1 AGL at %2")
                    .arg(explainDistance_FT(group.height()),
                         explainRunway(*rw));
        }
        if (d.has_value())
        {
            return QObject::tr("Ceiling height %1 AGL towards %2")
                    .arg(explainDistance_FT(group.height()),
                         explainDirection(*d));
        }
        return QObject::tr("Ceiling height %1")
                .arg(explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VARIABLE_CEILING:
        if (rw.has_value() && d.has_value())
        {
            return QObject::tr("Ceiling height %1 -- %2 AGL at %3 towards %4")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainRunway(*rw),
                         explainDirection(*d));
        }
        if (rw.has_value())
        {
            return QObject::tr("Ceiling height %1 -- %2 AGL at %3")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainRunway(*rw));
        }
        if (d.has_value())
        {
            return QObject::tr("Ceiling height %1 -- %2 AGL towards %3")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainDirection(*d));
        }
        return QObject::tr("Ceiling height %1 -- %2 AGL")
                .arg(explainDistance_FT(group.minHeight()),
                     explainDistance_FT(group.maxHeight()));

    case metaf::CloudGroup::Type::CHINO:
        return QObject::tr("Ceiling data not awailable");

    case metaf::CloudGroup::Type::CLD_MISG:
        return QObject::tr("Sky condition data (cloud data) is missing");

    case metaf::CloudGroup::Type::OBSCURATION:
        const auto h = group.height().distance();
        const auto ct = group.cloudType();
        if (h.has_value() && (h.value() == 0.0F) && ct.has_value())
        {
            return QObject::tr("Ground-based obscuration, %1").arg(explainCloudType(ct.value()));
        }
        if (h.has_value() && (h.value() == 0.0F)) {
            return QObject::tr("Ground-based obscuration");
        }
        if (h.has_value() && (h.value() != 0.0F) && ct.has_value())
        {
            return QObject::tr("Aloft obscuration, %1").arg(explainCloudType(ct.value()));
        }
        if (h.has_value() && (h.value() != 0.0F))
        {
            return QObject::tr("Aloft obscuration");
        }
        if (!ct.has_value())
        {
            return {};
        }
        return explainCloudType(ct.value());
    }
    return {};
}

QString Weather::Decoder::visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid()) {
        return QObject::tr("Invalid data");
    }

    QStringList layers;
    layers.reserve(5);
    const auto clouds = group.cloudTypes();
    for (const auto & cloud : clouds)
    {
        layers << explainCloudType(cloud);
    }
    return QObject::tr("Cloud layers: %1").arg(layers.join(QStringLiteral(" • ")));
}

QString Weather::Decoder::visitKeywordGroup(const KeywordGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    switch (group.type())
    {
    case metaf::KeywordGroup::Type::METAR:
        return QObject::tr("Report type: METAR");

    case metaf::KeywordGroup::Type::SPECI:
        return QObject::tr("Report type: unscheduled METAR");

    case metaf::KeywordGroup::Type::TAF:
        return QObject::tr("Report type: TAF");

    case metaf::KeywordGroup::Type::AMD:
        return QObject::tr("Amended report");

    case metaf::KeywordGroup::Type::NIL:
        return QObject::tr("Missing report");

    case metaf::KeywordGroup::Type::CNL:
        return QObject::tr("Cancelled report");

    case metaf::KeywordGroup::Type::COR:
        return QObject::tr("Correctional report");

    case metaf::KeywordGroup::Type::AUTO:
        return QObject::tr("Automated report");

    case metaf::KeywordGroup::Type::CAVOK:
        return QObject::tr("CAVOK");

    case metaf::KeywordGroup::Type::RMK:
        return QObject::tr("<strong>Remarks</strong>");

    case metaf::KeywordGroup::Type::MAINTENANCE_INDICATOR:
        return QObject::tr("Automated station requires maintenance");

    case metaf::KeywordGroup::Type::AO1:
        return QObject::tr("Automated station w/o precipitation discriminator");

    case metaf::KeywordGroup::Type::AO2:
        return QObject::tr("Automated station with precipitation discriminator");

    case metaf::KeywordGroup::Type::AO1A:
        return QObject::tr("Automated station w/o precipitation discriminator, report augmented by a human observer");

    case metaf::KeywordGroup::Type::AO2A:
        return QObject::tr("Automated station with precipitation discriminator, report augmented by a human observer");

    case metaf::KeywordGroup::Type::NOSPECI:
        return QObject::tr("Manual station, does not issue SPECI reports");
    }
    return {};
}

QString Weather::Decoder::visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    if (!group.baseHeight().isReported() && !group.topHeight().isReported())
    {
        return QObject::tr("%1 at all heights")
                .arg(layerForecastGroupTypeToString(group.type()));
    }

    return QObject::tr("%1 at heights from %2 to %3.")
            .arg(layerForecastGroupTypeToString(group.type()),
                 explainDistance(group.baseHeight()),
                 explainDistance(group.topHeight()));
}

QString Weather::Decoder::visitLightningGroup(const LightningGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    QStringList result;
    result << QObject::tr("Lightning strikes observed.");

    if (group.distance().isReported())
    {
        result << QObject::tr("Distance %1.").arg(explainDistance(group.distance()));
    }

    switch(group.frequency())
    {
    case metaf::LightningGroup::Frequency::NONE:
        break;

    case metaf::LightningGroup::Frequency::OCCASIONAL:
        result << QObject::tr("Less than 1 strike per minute.");
        break;

    case metaf::LightningGroup::Frequency::FREQUENT:
        result << QObject::tr("1 -- 6 strikes per minute.");
        break;

    case metaf::LightningGroup::Frequency::CONSTANT:
        result << QObject::tr("More than 6 strikes per minute.");
        break;
    }

    if (group.isCloudGround() || group.isInCloud() || group.isCloudCloud() || group.isCloudAir()) {
        QStringList typeList;
        if (group.isCloudGround())
        {
            typeList << QObject::tr("cloud-to-ground");
        }
        if (group.isInCloud()) {
            typeList << QObject::tr("in-cloud");
        }
        if (group.isCloudCloud())
        {
            typeList << QObject::tr("cloud-to-cloud");
        }
        if (group.isCloudAir())
        {
            typeList << QObject::tr("cloud-to-air without strike to ground");
        }
        if (!typeList.isEmpty())
        {
            result << QObject::tr("Lightning types: %1.").arg(typeList.join(QStringLiteral(", ")));
        }
    }

    if (group.isUnknownType())
    {
        result << QObject::tr("Lightning strike types not recognised by parser.");
    }

    QStringList directionList;
    if (const auto directions = group.directions(); !directions.empty())
    {
        directionList << explainDirectionSector(directions);
    }
    if (!directionList.isEmpty())
    {
        result << QObject::tr("Lightning strikes observed in the following directions: %1").arg(directionList.join(QStringLiteral(", ")));
    }

    return result.join(QStringLiteral(" "));
}

QString Weather::Decoder::visitLocationGroup(const LocationGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    return QObject::tr("Report for %1").arg(QString::fromStdString(group.toString()));
}

QString Weather::Decoder::visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    return QObject::tr("Low cloud layer: %1 • Mid cloud layer: %2 • High cloud layer: %3")
            .arg(cloudLowLayerToString(group.lowLayer()),
                 cloudMidLayerToString(group.midLayer()),
                 cloudHighLayerToString(group.highLayer()));
}

QString Weather::Decoder::visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid()) {
        return QObject::tr("Invalid data");
    }

    QString result;
    const auto minimumTime = group.minimumTime();
    const auto maximumTime = group.maximumTime();
    switch(group.type())
    {
    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_6_HOURLY:
        return QObject::tr("Observed 6-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()),
                     explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_24_HOURLY:
        return QObject::tr("Observed 24-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()),
                     explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::FORECAST:
        if (group.minimum().isReported() && minimumTime.has_value()) {
            result += QObject::tr("Minimum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.minimum()),
                         explainMetafTime(minimumTime.value()));
        }
        if (group.maximum().isReported() && (maximumTime.has_value())) {
            result += " " + QObject::tr("Maximum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.maximum()),
                         explainMetafTime(maximumTime.value()));
        }
        return result;
    }
    return {};
}

QString Weather::Decoder::visitMiscGroup(const MiscGroup & group,  ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    static const QString colourCodeBlack = QObject::tr("Colour code BLACK: aerodrome closed due to snow accumulation or non-weather reasons");
    QString result;
    auto data = group.data();

    switch (group.type())
    {
    case metaf::MiscGroup::Type::SUNSHINE_DURATION_MINUTES:
        if (const auto duration = data; duration)
        {
            return QObject::tr("Duration of sunshine that occurred the previous calendar day is %1 minutes.").arg(qRound(*duration));
        }
        return QObject::tr("No sunshine occurred the previous calendar day");

    case metaf::MiscGroup::Type::CORRECTED_WEATHER_OBSERVATION:
        if (!data.has_value())
        {
            return {};
        }
        return QObject::tr("This report is the corrected weather observation, correction number is %1").arg(static_cast<int>(*data));

    case metaf::MiscGroup::Type::DENSITY_ALTITUDE:
        if (!data.has_value())
        {
            return {};
        }
        return QObject::tr("Density altitude is %1 feet").arg(qRound(*data));

    case metaf::MiscGroup::Type::HAILSTONE_SIZE:
        if (!data.has_value())
        {
            return {};
        }
        return QObject::tr("Largest hailstone size is %1 inches").arg(*data);

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKBLUE:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_BLUE:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code BLUE: visibility >8000 m and lowest cloud base height >2500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKBLUE_PLUS:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_BLUE_PLUS:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code BLUE+: visibility >8000 m or lowest cloud base height >2000 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code YELLOW: visibility 1600-3700 m or lowest cloud base height 300-700 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKWHITE:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_WHITE:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code WHITE: visibility >5000 m and lowest cloud base height >1500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKGREEN:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_GREEN:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code GREEN: visibility >3700 m and lowest cloud base height >700 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW1:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW1:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code YELLOW 1: visibility >2500 m and lowest cloud base height >500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW2:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW2:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code YELLOW 2: visibility >1600 m and lowest cloud base height >300 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKAMBER:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_AMBER:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code AMBER: visibility >800 m and lowest cloud base height >200 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKRED:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_RED:
        if (!result.isEmpty())
        {
            result += u" "_s;
        }
        result += QObject::tr("Colour code RED: visibility <800 m or lowest cloud base height <200 ft");
        return result;

    case metaf::MiscGroup::Type::FROIN:
        return QObject::tr("Frost on the instrument (e.g. due to freezing fog depositing rime).");

    case metaf::MiscGroup::Type::ISSUER_ID_FN:
        if (!data.has_value())
        {
            return {}; 
        }
        return QObject::tr("Report issuer identifier is %1. This forecast is issued at The Fleet Weather Center Norfolk, VA.").arg(static_cast<int>(*data));

    case metaf::MiscGroup::Type::ISSUER_ID_FS:
        if (!data.has_value())
        {
            return {};
        }
        return QObject::tr("Report issuer identifier is %1. This forecast is issued at The Fleet Weather Center San Diego, CA (FS).").arg(static_cast<int>(*data));
    }
    return {};
}

QString Weather::Decoder::visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    switch(group.type())
    {
    case metaf::PrecipitationGroup::Type::TOTAL_PRECIPITATION_HOURLY:
        return QObject::tr("Total precipitation for the past hour: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_DEPTH_ON_GROUND:
        return QObject::tr("Snow depth on ground: %1")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_OR_6_HOURLY:
        return QObject::tr("Water equivalent of frozen precipitation for the last 3 or 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_HOURLY:
        return QObject::tr("Water equivalent of frozen precipitation for the last 3 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_6_HOURLY:
        return QObject::tr("Water equivalent of frozen precipitation for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_24_HOURLY:
        return QObject::tr("Water equivalent of frozen precipitation for the last 24 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_6_HOURLY:
        return QObject::tr("Snowfall for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::WATER_EQUIV_OF_SNOW_ON_GROUND:
        return QObject::tr("Water equivalent of snow on ground: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_HOUR:
        return QObject::tr("Ice accretion for the last hour: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_3_HOURS:
        return QObject::tr("Ice accretion for the last 3 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_6_HOURS:
        return QObject::tr("Ice accretion for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::PRECIPITATION_ACCUMULATION_SINCE_LAST_REPORT:
        return QObject::tr("Precipitation accumulation since last report: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_INCREASING_RAPIDLY:
        return QObject::tr("Snow increasing rapidly. For the last hour snow increased by %1. Total snowfall: %2.")
                .arg(explainPrecipitation(group.recent()),
                     explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::RAINFALL_9AM_10MIN:
        return QObject::tr("Rainfall for the last 10 minutes before report release time: %1. Rainfall since 9:00 local time: %2.")
                .arg(explainPrecipitation(group.recent()),
                     explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::PNO:
        return QObject::tr("Tipping bucket rain gauge INOP.");

    case metaf::PrecipitationGroup::Type::FZRANO:
        return QObject::tr("Freezing rain sensor INOP.");

    case metaf::PrecipitationGroup::Type::ICG_MISG:
        return QObject::tr("Icing data is missing.");

    case metaf::PrecipitationGroup::Type::PCPN_MISG:
        return QObject::tr("Precipitation data is missing.");
    }

    return {};
}

QString Weather::Decoder::visitPressureGroup(const PressureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    switch(group.type())
    {
    case metaf::PressureGroup::Type::OBSERVED_QNH:
        return QObject::tr("QNH: %1").arg(explainPressure(group.atmosphericPressure()));

    case metaf::PressureGroup::Type::FORECAST_LOWEST_QNH:
        return QObject::tr("Forecast lowest QNH: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::OBSERVED_QFE:
        return QObject::tr("QFE: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::OBSERVED_SLP:
        return QObject::tr("Standard sea level pressure: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::SLPNO:
        return QObject::tr("QNH is not available");
        break;

    case metaf::PressureGroup::Type::PRES_MISG:
        return QObject::tr("Atmospheric pressure data is missing");
    }
    return {};
}

QString Weather::Decoder::visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    switch (group.type())
    {
    case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
        return QObject::tr("3-hour pressure tendency is not reported. Absolute pressure change is %1.")
                .arg(explainPressure(group.difference()));

    case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
    case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
        return QObject::tr("Atmospheric pressure is %1")
                .arg(pressureTendencyTypeToString(group.type()));

    default:
        //: Note: the string %2 will be replaced by a text such as "less than"
        return QObject::tr("During last 3 hours the atmospheric pressure was %1. Now the atmospheric pressure is %2 3h ago. Absolute pressure change is %3")
                .arg(pressureTendencyTypeToString(group.type()),
                     pressureTendencyTrendToString(metaf::PressureTendencyGroup::trend(group.type())),explainPressure(group.difference()));
    }
    return {};
}

QString Weather::Decoder::visitReportTimeGroup(const ReportTimeGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    return QObject::tr("Issued at %1").arg(explainMetafTime(group.time()));
}

QString Weather::Decoder::visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    QString result = QObject::tr("State of %1:").arg(explainRunway(group.runway()));

    switch (group.type())
    {
    case metaf::RunwayStateGroup::Type::RUNWAY_STATE:
        result += runwayStateDepositsToString(group.deposits());
        if (group.deposits() != metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY)
        {
            result += ", " + QObject::tr("%1 of deposits, %2 of runway contaminated")
                    .arg(explainPrecipitation(group.depositDepth()),runwayStateExtentToString(group.contaminationExtent()));
        }
        result += ", " + explainSurfaceFriction(group.surfaceFriction());
        break;

    case metaf::RunwayStateGroup::Type::RUNWAY_CLRD:
        result += QObject::tr("deposits on runway were cleared or ceased to exist");
        result += ", " + explainSurfaceFriction(group.surfaceFriction());
        break;

    case metaf::RunwayStateGroup::Type::RUNWAY_SNOCLO:
        result += QObject::tr("runway closed due to snow accumulation");
        break;

    case metaf::RunwayStateGroup::Type::AERODROME_SNOCLO:
        return QObject::tr("Aerodrome closed due to snow accumulation");

    case metaf::RunwayStateGroup::Type::RUNWAY_NOT_OPERATIONAL:
        result += QObject::tr("runway is not operational");
    }
    return result;
}

QString Weather::Decoder::visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    return QObject::tr("Sea surface temperature: %1, %2")
            .arg(explainTemperature(group.surfaceTemperature()),
                 explainWaveHeight(group.waves()));
}

QString Weather::Decoder::visitTemperatureGroup(const TemperatureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    switch (group.type())
    {
    case metaf::TemperatureGroup::Type::TEMPERATURE_AND_DEW_POINT:
        return QObject::tr("Temperature %1, Dew point %2")
                .arg(explainTemperature(group.airTemperature()),
                     explainTemperature(group.dewPoint()));

    case metaf::TemperatureGroup::Type::T_MISG:
        return QObject::tr("Temperature data is missing");

    case metaf::TemperatureGroup::Type::TD_MISG:
        return QObject::tr("Dew point data is missing");
    }
    return {};
}

QString Weather::Decoder::visitTrendGroup(const TrendGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    const auto timeFrom = group.timeFrom();
    const auto timeUntil = group.timeUntil();
    const auto timeAt = group.timeAt();

    QString result;
    switch (group.type())
    {
    case metaf::TrendGroup::Type::NOSIG:
        return QObject::tr("No significant weather changes expected");

    case metaf::TrendGroup::Type::BECMG:
        result += QObject::tr("Gradually changing");
        if (timeFrom)
        {
            result += " " + QObject::tr("from %1").arg(explainMetafTime(*timeFrom));
        }
        if (timeUntil)
        {
            result += " " + QObject::tr("until %1").arg(explainMetafTime(*timeUntil));
        }
        if (timeAt)
        {
            result += " " + QObject::tr("at %1").arg(explainMetafTime(*timeAt));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::TEMPO:
    case metaf::TrendGroup::Type::INTER:
        result += QObject::tr("Temporarily");
        if (timeFrom)
        {
            result += " " + QObject::tr("from %1").arg(explainMetafTime(*timeFrom));
        }
        if (timeUntil)
        {
            result += " " + QObject::tr("until %1").arg(explainMetafTime(*timeUntil));
        }
        if (timeAt)
        {
            result += " " + QObject::tr("at %1").arg(explainMetafTime(*timeAt));
        }
        if (group.probability() != metaf::TrendGroup::Probability::NONE)
        {
            result += QStringLiteral(" (%1)").arg(probabilityToString(group.probability()));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::FROM:
        if (!timeFrom.has_value())
        {
            return {};
        }
        result = QObject::tr("Forecast: rapid weather change at %1")
                .arg(explainMetafTime(*timeFrom));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::UNTIL:
        if (!timeUntil.has_value())
        {
            return {};
        }
        result = QObject::tr("Forecast until %1")
                .arg(explainMetafTime(*timeUntil));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::AT:
        if (!timeAt.has_value())
        {
            return {};
        }    
        result = QObject::tr("Forecast for %1")
                .arg(explainMetafTime(*timeAt));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::TIME_SPAN:
        if ((!timeFrom.has_value()) || (!timeUntil.has_value()))
        {
            return {};
        } 

        if (group.probability() != metaf::TrendGroup::Probability::NONE)
        {
            result = QObject::tr("Forecast from %1 to %2 (%3)")
                    .arg(explainMetafTime(*timeFrom),
                         explainMetafTime(*timeUntil),
                         probabilityToString(group.probability()));
        }
        else
        {
            result = QObject::tr("Forecast from %1 to %2")
                    .arg(explainMetafTime(*timeFrom),
                         explainMetafTime(*timeUntil));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::PROB:
        return QObject::tr("Forecast %1")
                .arg(probabilityToString(group.probability()));
    }
    return {};
}

QString Weather::Decoder::visitUnknownGroup(const UnknownGroup & group, ReportPart /*reportPart*/, const std::string & rawString)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    return QObject::tr("Not recognised by parser: %1").arg(QString::fromStdString(rawString));
}

QString Weather::Decoder::visitVicinityGroup(const VicinityGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }


    QString type;
    switch (group.type())
    {
    case metaf::VicinityGroup::Type::THUNDERSTORM:
        type = QObject::tr("Thunderstorm");
        break;

    case metaf::VicinityGroup::Type::CUMULONIMBUS:
        type = QObject::tr("Cumulonimbus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::CUMULONIMBUS_MAMMATUS:
        type = QObject::tr("Cumulonimbus cloud(s) with mammatus");
        break;

    case metaf::VicinityGroup::Type::TOWERING_CUMULUS:
        type = QObject::tr("Towering cumulus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ALTOCUMULUS_CASTELLANUS:
        type = QObject::tr("Altocumulus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::STRATOCUMULUS_STANDING_LENTICULAR:
        type = QObject::tr("Stratocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ALTOCUMULUS_STANDING_LENTICULAR:
        type = QObject::tr("Altocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::CIRROCUMULUS_STANDING_LENTICULAR:
        type = QObject::tr("Cirrocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ROTOR_CLOUD:
        type = QObject::tr("Rotor cloud(s)");
        break;

    case metaf::VicinityGroup::Type::VIRGA:
        type = QObject::tr("Virga");
        break;

    case metaf::VicinityGroup::Type::PRECIPITATION_IN_VICINITY:
        type = QObject::tr("Precipitation");
        break;

    case metaf::VicinityGroup::Type::FOG:
        type = QObject::tr("Fog");
        break;

    case metaf::VicinityGroup::Type::FOG_SHALLOW:
        type = QObject::tr("Shallow fog");
        break;

    case metaf::VicinityGroup::Type::FOG_PATCHES:
        type = QObject::tr("Patches of fog");
        break;

    case metaf::VicinityGroup::Type::HAZE:
        type = QObject::tr("Haze");
        break;

    case metaf::VicinityGroup::Type::SMOKE:
        type = QObject::tr("Smoke");
        break;

    case metaf::VicinityGroup::Type::BLOWING_SNOW:
        type = QObject::tr("Blowing snow");
        break;

    case metaf::VicinityGroup::Type::BLOWING_SAND:
        type = QObject::tr("Blowing sand");
        break;

    case metaf::VicinityGroup::Type::BLOWING_DUST:
        type = QObject::tr("Blowing dust");
        break;
    }

    // Here %1 is string like 'Smoke'
    QStringList results;
    results << QObject::tr("%1 observed.");

    if (group.distance().isReported())
    {
        results << QObject::tr("Distance %1.").arg(explainDistance(group.distance()));
    }
    if (const auto directions = group.directions(); !directions.empty())
    {
        results << QObject::tr("Directions: %1").arg(explainDirectionSector(directions));
    }
    if (group.movingDirection().isReported())
    {
        //: %1 is string like 'west'
        results << QObject::tr("Moving towards %1.").arg(cardinalDirectionToString(group.movingDirection().cardinal()));
    }

    return results.join(QStringLiteral(" "));
}

QString Weather::Decoder::visitVisibilityGroup(const VisibilityGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }
    const auto direction = group.direction();
    const auto runway = group.runway();
    switch (group.type())
    {
    case metaf::VisibilityGroup::Type::PREVAILING:
        return QObject::tr("Visibility is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::PREVAILING_NDV:
        return QObject::tr("Visibility is %1. Station cannot differentiate the directional variation of visibility")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::DIRECTIONAL:
        if ((!direction.has_value()))
        {
            return {};
        } 
        return QObject::tr("Visibility toward %1 is %2")
                .arg(explainDirection(direction.value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::RUNWAY:
        if ((!runway.has_value()))
        {
            return {};
        } 
        return QObject::tr("Visibility for %1 is %2")
                .arg(explainRunway(runway.value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::RVR:
        if (!runway.has_value())
        {
            return {};
        }
        if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
            return QObject::tr("Runway visual range for %1 is %2 and the trend is %3")
                    .arg(explainRunway(runway.value()),
                         explainDistance(group.visibility()),
                         visTrendToString(group.trend()));
        }
        return QObject::tr("Runway visual range for %1 is %2")
                .arg(explainRunway(runway.value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::SURFACE:
        return QObject::tr("Visibility at surface level is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::TOWER:
        return QObject::tr("Visibility from air traffic control tower is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::SECTOR:
        return QObject::tr("Sector visibility is %1 in the following directions %2")
                .arg(explainDistance(group.visibility()),
                     explainDirectionSector(group.sectorDirections()));

    case metaf::VisibilityGroup::Type::VARIABLE_PREVAILING:
        return QObject::tr("Visibility is variable from %1 to %2")
                .arg(explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_DIRECTIONAL:
        if (!direction.has_value())
        {
            return {};
        }
        return QObject::tr("Directional visibility toward %1 is variable from %2 to %3")
                .arg(explainDirection(direction.value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_RUNWAY:
        if (!runway.has_value())
        {
            return {};
        }
        return QObject::tr("Visibility for %1 is variable from %2 to %3")
                .arg(explainRunway(runway.value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_RVR:
        if (!runway.has_value())
        {
            return {};
        }
        if (group.trend() != metaf::VisibilityGroup::Trend::NONE)
        {
            return QObject::tr("Runway visual range for %1 is variable from %2 to %3 and the trend is %4")
                    .arg(explainRunway(runway.value()),
                         explainDistance(group.minVisibility()),
                         explainDistance(group.maxVisibility()),
                         visTrendToString(group.trend()));
        }
        return QObject::tr("Runway visual range for %1 is variable from %2 to %3")
                .arg(explainRunway(runway.value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_SECTOR:
        return QObject::tr("Sector visibility is variable from %1 to %2 in the following directions: %3")
                .arg(explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()),
                     explainDirectionSector(group.sectorDirections()));

    case metaf::VisibilityGroup::Type::VIS_MISG:
        return QObject::tr("Visibility data missing");

    case metaf::VisibilityGroup::Type::RVR_MISG:
        return QObject::tr("Runway visual range data is missing");

    case metaf::VisibilityGroup::Type::RVRNO:
        return QObject::tr("Runway visual range should be reported but is missing");

    case metaf::VisibilityGroup::Type::VISNO:
        const auto r = group.runway();
        const auto d = group.direction();
        if (r.has_value() && d.has_value())
        {
            return QObject::tr("Visibility data not available for %1 in the direction of %2")
                    .arg(explainRunway(*r),
                         explainDirection(*d));
        }
        if (r.has_value())
        {
            return QObject::tr("Visibility data not available for %1")
                    .arg(explainRunway(*r));
        }
        if (d.has_value())
        {
            return QObject::tr("Visibility data not available in the direction of %1")
                    .arg(explainDirection(*d));
        }
        return QObject::tr("Visibility data not awailable");
    }
    return {};
}

QString Weather::Decoder::visitWeatherGroup(const WeatherGroup & group, ReportPart part, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    // Gather string with list of phenomena
    QStringList phenomenaList;
    phenomenaList.reserve(8);
    for (const auto p : group.weatherPhenomena())
    {
        phenomenaList << Weather::Decoder::explainWeatherPhenomena(p);
    }
    auto phenomenaString = phenomenaList.join(QStringLiteral(" • "));

    // If this is a METAR, then save the current weather
    if (part == ReportPart::METAR)
    {
        m_currentWeather = phenomenaString;
    }

    switch (group.type())
    {
    case metaf::WeatherGroup::Type::CURRENT:
        return phenomenaString; // QObject::tr("Current weather: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::RECENT:
        return QObject::tr("Recent weather: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::EVENT:
        return QObject::tr("Precipitation beginning/ending time: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::NSW:
        return QObject::tr("No significant weather");

    case metaf::WeatherGroup::Type::PWINO:
        return QObject::tr("Automated weather identifier INOP");

    case metaf::WeatherGroup::Type::TSNO:
        return QObject::tr("Lightning detector INOP");

    case metaf::WeatherGroup::Type::WX_MISG:
        return QObject::tr("Weather phenomena data is missing");

    case metaf::WeatherGroup::Type::TS_LTNG_TEMPO_UNAVBL:
        return QObject::tr("Thunderstorm / lightning data is missing");
    }

    return {};
}

QString Weather::Decoder::visitWindGroup(const WindGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/)
{
    if (!group.isValid())
    {
        return QObject::tr("Invalid data");
    }

    const auto eventTime = group.eventTime();
    switch (group.type())
    {
    case metaf::WindGroup::Type::SURFACE_WIND_CALM:
        return QObject::tr("No wind");

    case metaf::WindGroup::Type::SURFACE_WIND:
        if (group.gustSpeed().isReported())
        {
            return QObject::tr("Wind direction %1, wind speed %2, gusts at %3")
                    .arg(explainDirection(group.direction(), true),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return QObject::tr("Wind direction %1, wind speed %2")
                .arg(explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::VARIABLE_WIND_SECTOR:
        return QObject::tr("Variable wind direction %1 -- %2")
                .arg(explainDirection(group.varSectorBegin()),
                     explainDirection(group.varSectorEnd()));

    case metaf::WindGroup::Type::SURFACE_WIND_WITH_VARIABLE_SECTOR:
        if (group.gustSpeed().isReported())
        {
            return QObject::tr("Wind direction %1 (%2 -- %3), wind speed %4, gusts at %5")
                    .arg(explainDirection(group.direction(), true),
                         explainDirection(group.varSectorBegin()),
                         explainDirection(group.varSectorEnd()),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return QObject::tr("Wind direction %1 (%2 -- %3), wind speed %4")
                .arg(explainDirection(group.direction(), true),
                     explainDirection(group.varSectorBegin()),
                     explainDirection(group.varSectorEnd()),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR:
        if (group.gustSpeed().isReported()) {
            return QObject::tr("Wind shear at %1 AGL, wind direction %2, wind speed %3, gusts at %4")
                    .arg(explainDistance_FT(group.height()),
                         explainDirection(group.direction(), true),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return QObject::tr("Wind shear at %1 AGL, wind direction %2, wind speed %3")
                .arg(explainDistance_FT(group.height()),
                     explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));


    case metaf::WindGroup::Type::WIND_SHIFT:
        if (eventTime.has_value())
        {
            return QObject::tr("Wind direction changed at %1").arg(explainMetafTime(*eventTime));
        }
        return QObject::tr("Wind direction changed recently");

    case metaf::WindGroup::Type::WIND_SHIFT_FROPA:
        if (eventTime.has_value())
        {
            return QObject::tr("Wind direction changed at %1 because of weather front passage").arg(explainMetafTime(*eventTime));
        }
        return QObject::tr("Wind directed changed recently because of weather front passage");

    case metaf::WindGroup::Type::PEAK_WIND:
        if (!eventTime.has_value())
        {
            return {};
        }
        return QObject::tr("Peak wind observed at %1, wind direction %2, wind speed %3")
                .arg(explainMetafTime(eventTime.value()),
                     explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR_IN_LOWER_LAYERS:
        if (const auto rw = group.runway(); rw.has_value())
        {
            return  QObject::tr("Wind shear between runway level and 1.600 ft at runway %1").arg(explainRunway(*rw));
        }
        return QObject::tr("Wind shear between runway level and 1.600 ft");

    case metaf::WindGroup::Type::WSCONDS:
        return QObject::tr("Potential wind shear");

    case metaf::WindGroup::Type::WND_MISG:
        return QObject::tr("Wind data is missing");
    }
    return {};
}
