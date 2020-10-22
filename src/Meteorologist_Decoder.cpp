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
#include <QTimeZone>
#include <QXmlStreamAttribute>

#include <QDebug>
#include "../3rdParty/metaf/include/metaf.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <cmath>

#include "Clock.h"
#include "Meteorologist_Decoder.h"
#include "Meteorologist_WeatherStation.h"


Meteorologist::Decoder::Decoder(QString rawText, QObject *parent)
    : QObject(parent)
{

    const auto result = metaf::Parser::parse(rawText.toStdString());

    // Error handling
    /*
    if (result.reportMetadata.error == metaf::ReportError::NONE)
        qDebug() << "parsed successfully";
    */

    QStringList decodedStrings;
    for (const auto &groupInfo : result.groups) {
        auto description = visit(groupInfo);
        decodedStrings.append(description);
/*
           if (std::holds_alternative<metaf::WeatherGroup>(groupInfo.group))
            _weather = description;
 */
    }

    _decodedText = decodedStrings.join("<br>");
}


// explanation Methods

QString Meteorologist::Decoder::explainCloudType(const metaf::CloudType ct) {
    const auto h = ct.height();
    if (h.isReported())
        return tr("Cloud cover %1/8, %2, base height %3")
                .arg(ct.okta())
                .arg(cloudTypeToString(ct.type()))
                .arg(explainDistance_FT(ct.height()));
    return tr("Cloud cover %1/8, %2")
            .arg(ct.okta())
            .arg(cloudTypeToString(ct.type()));
}

QString Meteorologist::Decoder::explainDirection(const metaf::Direction & direction, bool trueCardinalDirections)
{
    std::ostringstream result;
    switch (direction.type()) {
    case metaf::Direction::Type::NOT_REPORTED:
        return tr("not reported");

    case metaf::Direction::Type::VARIABLE:
        return tr("variable");

    case metaf::Direction::Type::NDV:
        return tr("no directional variation");

    case metaf::Direction::Type::VALUE_DEGREES:
        if (const auto d = direction.degrees(); d.has_value())
            return QString("%1°").arg(*d);
        return tr("[unable to produce value in °]");

    case metaf::Direction::Type::VALUE_CARDINAL:
        if (const auto c = cardinalDirectionToString(direction.cardinal(trueCardinalDirections)); !c.isEmpty()) {
            if (direction.type() == metaf::Direction::Type::VALUE_DEGREES)
                return QString("(%1)").arg(c);
            return c;
        }
        break;

    case metaf::Direction::Type::OVERHEAD:
        return tr("overhead");

    case metaf::Direction::Type::ALQDS:
        return tr("all quadrants (all directions)");

    case metaf::Direction::Type::UNKNOWN:
        return tr("unknown direction");
    }

    return QString();
}

QString Meteorologist::Decoder::explainDistance_FT(const metaf::Distance & distance) {

    if (!distance.isReported())
        return tr("not reported");

    QString modifier;
    switch (distance.modifier()) {
    case metaf::Distance::Modifier::LESS_THAN:
        modifier = "≤ ";
        break;

    case metaf::Distance::Modifier::MORE_THAN:
        modifier = "≥ ";
        break;

    default:
        break;
    }

    if (!distance.isValue())
        return tr("no value");

    const auto d = distance.toUnit(metaf::Distance::Unit::FEET);
    if (d.has_value())
        return modifier + QString("%1 ft").arg(qRound(*d));

    return "[unable to convert distance to feet]";
}

QString Meteorologist::Decoder::explainMetafTime(const metaf::MetafTime & metafTime)
{
    // QTime for result
    auto metafQTime = QTime(metafTime.hour(), metafTime.minute());

#warning We should use the reporting time of the METAR/TAF here, but for now this is a good approximation
    // Compute QDime for result. This is complicated because METARs/TAFs contain only the day of the month, but
    // not the day.
    auto currentQDate = QDate::currentDate();
    auto currentDate = metaf::MetafTime::Date(currentQDate.year(), currentQDate.month(), currentQDate.day());
    auto metafDate = metafTime.dateBeforeRef(currentDate);
    auto metafQDate = QDate(metafDate.year, metafDate.month, metafDate.day);

    auto metafQDateTime = QDateTime(metafQDate, metafQTime, QTimeZone::utc());
    return Clock::describePointInTime(metafQDateTime);


    auto currentDateTime = QDateTime::currentDateTimeUtc();
    auto metarDateTime = currentDateTime;

    const auto day = metafTime.day();
    if (day.has_value()) {
        if (currentDateTime.date().day() > 25 && *day < 5)
            metarDateTime = currentDateTime.addMonths(1);
        if (currentDateTime.date().day() < 5 && *day > 25)
            metarDateTime = currentDateTime.addMonths(-1);
    }
    metarDateTime.setTime(QTime(metafTime.hour(), metafTime.minute()));

    return Clock::describePointInTime(metarDateTime);
}

QString Meteorologist::Decoder::explainPrecipitation(const metaf::Precipitation & precipitation)
{
    if (!precipitation.isReported())
        return "not reported";

    if (const auto p = precipitation.amount(); p.has_value() && !*p)
        return tr("trace amount");

    const auto p = precipitation.toUnit(metaf::Precipitation::Unit::MM);
    if (p.has_value())
        return QString("%1 mm").arg(QString::number(*p, 'f', 2));
    return tr("[unable to convert precipitation to mm]");
}

QString Meteorologist::Decoder::explainPressure(const metaf::Pressure & pressure) {

    if (!pressure.pressure().has_value())
        return tr("not reported");

    const auto phpa = pressure.toUnit(metaf::Pressure::Unit::HECTOPASCAL);
    if (phpa.has_value())
        return QString("%1 hPa").arg(qRound(*phpa));
    return tr("[unable to convert pressure to hPa]");
}

QString Meteorologist::Decoder::explainRunway(const metaf::Runway & runway) {
    if (runway.isAllRunways())
        return tr("all runways");
    if (runway.isMessageRepetition())
        return tr("same runway (repetition of last message)");

    switch(runway.designator()) {
    case metaf::Runway::Designator::NONE:
        return tr("runway %1").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::LEFT:
        return tr("runway %1 LEFT").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::CENTER:
        return tr("runway %1 CENTER").arg(runway.number(), 2, 10, QChar('0'));

    case metaf::Runway::Designator::RIGHT:
        return tr("runway %1 RIGHT").arg(runway.number(), 2, 10, QChar('0'));

    }
    return QString();
}

QString Meteorologist::Decoder::explainSpeed(const metaf::Speed & speed) {

    if (const auto s = speed.speed(); !s.has_value())
        return tr("not reported");

    bool useMetric = false;
    auto globalSettings = GlobalSettings::globalInstance();
    if (globalSettings)
        useMetric = globalSettings->useMetricUnits();

    if (useMetric) {
        const auto s = speed.toUnit(metaf::Speed::Unit::KILOMETERS_PER_HOUR);
        if (s.has_value())
            return QString("%1 km/h").arg(qRound(*s));
        return tr("[unable to convert speed to km/h]");
    }

    const auto s = speed.toUnit(metaf::Speed::Unit::KNOTS);
    if (s.has_value())
        return QString("%1 kt").arg(qRound(*s));
    return tr("[unable to convert speed to knots]");
}

QString Meteorologist::Decoder::explainSurfaceFriction(const metaf::SurfaceFriction & surfaceFriction)
{
    const auto c = surfaceFriction.coefficient();

    switch (surfaceFriction.type()) {
    case metaf::SurfaceFriction::Type::NOT_REPORTED:
        return tr("not reported");

    case metaf::SurfaceFriction::Type::SURFACE_FRICTION_REPORTED:
        if (c.has_value())
            return tr("friction coefficient %1").arg(QString::number(*c, 'f', 2));
        return tr("[unable to produce a friction coefficient]");

    case metaf::SurfaceFriction::Type::BRAKING_ACTION_REPORTED:
        return tr("braking action %1").arg(brakingActionToString(surfaceFriction.brakingAction()));

    case metaf::SurfaceFriction::Type::UNRELIABLE:
        return tr("unreliable or unmeasurable");
    }
    return QString();
}

QString Meteorologist::Decoder::explainTemperature(const metaf::Temperature & temperature)
{
    if (!temperature.temperature().has_value())
        return tr("not reported");

    QString temperatureString = tr("[unable to convert temperature to °C]");
    const auto t = temperature.toUnit(metaf::Temperature::Unit::C);
    if (t.has_value())
        temperatureString = QString("%1 °C").arg(qRound(*t));

    if (!(*temperature.temperature()) && !temperature.isPrecise()) {
        if (temperature.isFreezing())
            return tr("slightly less than %1").arg(temperatureString);
        if (!temperature.isFreezing())
            return tr("slightly more than %1").arg(temperatureString);
    }

    return temperatureString;
}

QString Meteorologist::Decoder::explainWaveHeight(const metaf::WaveHeight & waveHeight)
{
    switch (waveHeight.type()) {
    case metaf::WaveHeight::Type::STATE_OF_SURFACE:
        return tr("state of sea surface: %1").arg(stateOfSeaSurfaceToString(waveHeight.stateOfSurface()));

    case metaf::WaveHeight::Type::WAVE_HEIGHT:
        if (waveHeight.isReported()) {
            const auto h = waveHeight.toUnit(metaf::WaveHeight::Unit::METERS);
            if (h.has_value())
                return tr("wave height: %1 m").arg(qRound(*h));
            return tr("[unable to convert wave height to meters]");
        }
        return tr("wave height not reported");
    }
}

QString Meteorologist::Decoder::explainWeatherPhenomena(const metaf::WeatherPhenomena & wp)
{
    /* Handle special cases */
    const auto weatherStr = Meteorologist::Decoder::specialWeatherPhenomenaToString(wp);
    if (!weatherStr.isEmpty())
        return weatherStr;

    // Obtain strings for qualifier & descriptor
    auto qualifier = Meteorologist::Decoder::weatherPhenomenaQualifierToString(wp.qualifier()); // Qualifier, such as "light" or "moderate"
    auto descriptor = Meteorologist::Decoder::weatherPhenomenaDescriptorToString(wp.descriptor()); // Descriptor, such as "freezing" or "blowing"

    // String that will hold the result
    QString result;

    QStringList weatherPhenomena;
    for (const auto w : wp.weather()) {
        // This is a string such as "hail" or "rain"
        auto wpString = Meteorologist::Decoder::weatherPhenomenaWeatherToString(w);
        if (!wpString.isEmpty())
            weatherPhenomena << Meteorologist::Decoder::weatherPhenomenaWeatherToString(w);
    }
    // Special case: "shower" is used as a phenomenom
    if (weatherPhenomena.isEmpty() && wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS) {
        weatherPhenomena << tr("shower");
        descriptor = QString();
    }
    if (weatherPhenomena.isEmpty() && wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM) {
        weatherPhenomena << tr("thunderstorm");
        descriptor = QString();
    }
    result += weatherPhenomena.join(", ");

    // Handle special qualifiers

    if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::RECENT) {
        result = tr("recent %1").arg(result);
        qualifier = QString();
    }
    if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::VICINITY) {
        result = tr("%1 in the vicinity").arg(result);
        qualifier = QString();
    }

    // The remaining descriptors and qualifiers go into a parenthesis text
    QStringList parenthesisTexts;
    if (!qualifier.isEmpty())
        parenthesisTexts << qualifier;
    if (!descriptor.isEmpty())
        parenthesisTexts << descriptor;
    auto parenthesisText = parenthesisTexts.join(", ");
    if (!parenthesisText.isEmpty())
        result += QString(" (%1)").arg(parenthesisText);


    const auto time = wp.time();
    switch (wp.event()){
    case metaf::WeatherPhenomena::Event::BEGINNING:
        if (!time.has_value())
            break;
        result += " " + tr("began:") + " " + Meteorologist::Decoder::explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::ENDING:
        if (!time.has_value())
            break;
        result += " " + tr("ended:") + " " + Meteorologist::Decoder::explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::NONE:
        break;
    }

    if (!parenthesisText.isEmpty())
        qWarning() << "Weather phenomena w/o special handling code" << result;

    return result;
}


// …toString Methods

QString Meteorologist::Decoder::brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction)
{
    switch(brakingAction) {
    case metaf::SurfaceFriction::BrakingAction::NONE:
        return tr("not reported");

    case metaf::SurfaceFriction::BrakingAction::POOR:
        return tr("poor (friction coefficient 0.0 to 0.25)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM_POOR:
        return tr("medium/poor (friction coefficient 0.26 to 0.29)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM:
        return tr("medium (friction coefficient 0.30 to 0.35)");

    case metaf::SurfaceFriction::BrakingAction::MEDIUM_GOOD:
        return tr("medium/good (friction coefficient 0.36 to 0.40)");

    case metaf::SurfaceFriction::BrakingAction::GOOD:
        return tr("good (friction coefficient 0.40 to 1.00)");
    }
}

QString Meteorologist::Decoder::cardinalDirectionToString(metaf::Direction::Cardinal cardinal)
{
    switch(cardinal) {
    case metaf::Direction::Cardinal::NOT_REPORTED:
        return tr("not reported");

    case metaf::Direction::Cardinal::N:
        return tr("north");

    case metaf::Direction::Cardinal::S:
        return tr("south");

    case metaf::Direction::Cardinal::W:
        return tr("west");

    case metaf::Direction::Cardinal::E:
        return tr("east");

    case metaf::Direction::Cardinal::NW:
        return tr("northwest");

    case metaf::Direction::Cardinal::NE:
        return tr("northeast");

    case metaf::Direction::Cardinal::SW:
        return tr("southwest");

    case metaf::Direction::Cardinal::SE:
        return tr("southeast");

    case metaf::Direction::Cardinal::TRUE_N:
        return tr("true north");

    case metaf::Direction::Cardinal::TRUE_W:
        return tr("true west");

    case metaf::Direction::Cardinal::TRUE_S:
        return tr("true south");

    case metaf::Direction::Cardinal::TRUE_E:
        return tr("true east");

    case metaf::Direction::Cardinal::NDV:
        return tr("no directional variations");

    case metaf::Direction::Cardinal::VRB:
        return "variable";

    case metaf::Direction::Cardinal::OHD:
        return "overhead";

    case metaf::Direction::Cardinal::ALQDS:
        return "all quadrants (in all directions)";

    case metaf::Direction::Cardinal::UNKNOWN:
        return "unknown direction";
    }
}

QString Meteorologist::Decoder::cloudAmountToString(metaf::CloudGroup::Amount amount) {
    switch (amount) {
    case metaf::CloudGroup::Amount::NOT_REPORTED:
        return tr("Cloud amount not reported");

    case metaf::CloudGroup::Amount::NSC:
        return tr("No significant cloud");

    case metaf::CloudGroup::Amount::NCD:
        return tr("No cloud detected");

    case metaf::CloudGroup::Amount::NONE_CLR:
    case metaf::CloudGroup::Amount::NONE_SKC:
        return tr("Clear sky");

    case metaf::CloudGroup::Amount::FEW:
        return tr("Few clouds");

    case metaf::CloudGroup::Amount::SCATTERED:
        return tr("Scattered clouds");

    case metaf::CloudGroup::Amount::BROKEN:
        return tr("Broken clouds");

    case metaf::CloudGroup::Amount::OVERCAST:
        return tr("Overcast clouds");

    case metaf::CloudGroup::Amount::OBSCURED:
        return tr("Sky obscured");

    case metaf::CloudGroup::Amount::VARIABLE_FEW_SCATTERED:
        return tr("Few -- scattered clouds");

    case metaf::CloudGroup::Amount::VARIABLE_SCATTERED_BROKEN:
        return tr("Scattered -- broken clouds");

    case metaf::CloudGroup::Amount::VARIABLE_BROKEN_OVERCAST:
        return tr("Broken -- overcast clouds");
    }
}

QString Meteorologist::Decoder::cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer)
{
    switch(highLayer) {
    case metaf::LowMidHighCloudGroup::HighLayer::NONE:
        return tr("No high-layer clouds");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC:
        return tr("Cirrus fibratus or Cirrus uncinus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CI_CAS_CI_FLO:
        return tr("Cirrus spissatus or Cirrus castellanus or Cirrus floccus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CBGEN:
        return tr("Cirrus spissatus cumulonimbogenitus");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC_SPREADING:
        return tr("Cirrus uncinus or Cirrus fibratus progressively invading the sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_LOW_ABOVE_HORIZON:
        return tr("Cirrus or Cirrostratus progressively invading the sky, but the continuous veil does not reach 45° above the horizon");

    case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_HIGH_ABOVE_HORIZON:
        return tr("Cirrus or Cirrostratus progressively invading the sky, the continuous veil extends more than 45° above the horizon, without the sky being totally covered");

    case metaf::LowMidHighCloudGroup::HighLayer::CS_NEB_CS_FIB_COVERING_ENTIRE_SKY:
        return tr("Cirrostratus nebulosus or Cirrostratus fibratus covering the whole sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CS:
        return tr("Cirrostratus that is not invading the sky and that does not completely cover the whole sky");

    case metaf::LowMidHighCloudGroup::HighLayer::CC:
        return tr("Cirrocumulus alone");

    case metaf::LowMidHighCloudGroup::HighLayer::NOT_OBSERVABLE:
        return tr("Clouds are not observable");
    }
    return QString();
}

QString Meteorologist::Decoder::cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer)
{
    switch(lowLayer) {
    case metaf::LowMidHighCloudGroup::LowLayer::NONE:
        return tr("No low layer clouds");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_HU_CU_FR:
        return tr("Cumulus humilis or Cumulus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_MED_CU_CON:
        return tr("Cumulus clouds with moderate or significant vertical extent");

    case metaf::LowMidHighCloudGroup::LowLayer::CB_CAL:
        return tr("Cumulonimbus calvus");

    case metaf::LowMidHighCloudGroup::LowLayer::SC_CUGEN:
        return tr("Stratocumulus cumulogenitus");

    case metaf::LowMidHighCloudGroup::LowLayer::SC_NON_CUGEN:
        return tr("Stratocumulus non-cumulogenitus");

    case metaf::LowMidHighCloudGroup::LowLayer::ST_NEB_ST_FR:
        return tr("Stratus nebulosus or Stratus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::ST_FR_CU_FR_PANNUS:
        return tr("Stratus fractus or Cumulus fractus");

    case metaf::LowMidHighCloudGroup::LowLayer::CU_SC_NON_CUGEN_DIFFERENT_LEVELS:
        return tr("Cumulus and Stratocumulus with bases at different levels");

    case metaf::LowMidHighCloudGroup::LowLayer::CB_CAP:
        return "Cumulonimbus capillatus or Cumulonimbus capillatus incus)";

    case metaf::LowMidHighCloudGroup::LowLayer::NOT_OBSERVABLE:
        return tr("Clouds are not observable due to fog, blowing dust or sand, or other similar phenomena");
    }
}

QString Meteorologist::Decoder::cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer)
{
    switch(midLayer) {
    case metaf::LowMidHighCloudGroup::MidLayer::NONE:
        return tr("No mid-layer clouds");

    case metaf::LowMidHighCloudGroup::MidLayer::AS_TR:
        return tr("Altostratus translucidus");

    case metaf::LowMidHighCloudGroup::MidLayer::AS_OP_NS:
        return tr("Altostratus opacus or Nimbostratus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR:
        return tr("Altocumulus translucidus at a single level");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_LEN_PATCHES:
        return tr("Patches of Altocumulus translucidus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_AC_OP_SPREADING:
        return tr("Altocumulus translucidus in bands");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_CUGEN_AC_CBGEN:
        return tr("Altocumulus cumulogenitus or Altocumulus cumulonimbogenitus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_DU_AC_OP_AC_WITH_AS_OR_NS:
        return tr("Altocumulus duplicatus, or Altocumulus opacus in a single layer");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_CAS_AC_FLO:
        return tr("Altocumulus castellanus or Altocumulus floccus");

    case metaf::LowMidHighCloudGroup::MidLayer::AC_OF_CHAOTIC_SKY:
        return tr("Broken cloud sheets of ill-defined species or varieties");

    case metaf::LowMidHighCloudGroup::MidLayer::NOT_OBSERVABLE:
        return tr("Clouds are not observable");
    }
    return QString();
}

QString Meteorologist::Decoder::cloudTypeToString(metaf::CloudType::Type type)
{
    switch(type) {
    case metaf::CloudType::Type::NOT_REPORTED:
        return tr("cumulonimbus");

    case metaf::CloudType::Type::CUMULONIMBUS:
        return tr("cumulonimbus");

    case metaf::CloudType::Type::TOWERING_CUMULUS:
        return tr("towering cumulus");

    case metaf::CloudType::Type::CUMULUS:
        return tr("cumulus");

    case metaf::CloudType::Type::CUMULUS_FRACTUS:
        return tr("cumulus fractus");

    case metaf::CloudType::Type::STRATOCUMULUS:
        return tr("stratocumulus");

    case metaf::CloudType::Type::NIMBOSTRATUS:
        return tr("nimbostratus");

    case metaf::CloudType::Type::STRATUS:
        return tr("stratus");

    case metaf::CloudType::Type::STRATUS_FRACTUS:
        return tr("stratus fractus");

    case metaf::CloudType::Type::ALTOSTRATUS:
        return tr("altostratus");

    case metaf::CloudType::Type::ALTOCUMULUS:
        return tr("altocumulus");

    case metaf::CloudType::Type::ALTOCUMULUS_CASTELLANUS:
        return tr("altocumulus castellanus");

    case metaf::CloudType::Type::CIRRUS:
        return tr("cirrus");

    case metaf::CloudType::Type::CIRROSTRATUS:
        return tr("cirrostratus");

    case metaf::CloudType::Type::CIRROCUMULUS:
        return tr("cirrocumulus");

    case metaf::CloudType::Type::BLOWING_SNOW:
        return tr("blowing snow");

    case metaf::CloudType::Type::BLOWING_DUST:
        return tr("blowing dust");

    case metaf::CloudType::Type::BLOWING_SAND:
        return tr("blowing sand");

    case metaf::CloudType::Type::ICE_CRYSTALS:
        return tr("ice crystals");

    case metaf::CloudType::Type::RAIN:
        return tr("rain");

    case metaf::CloudType::Type::DRIZZLE:
        return tr("drizzle");

    case metaf::CloudType::Type::SNOW:
        return tr("snow");

    case metaf::CloudType::Type::ICE_PELLETS:
        return tr("ice pellets");

    case metaf::CloudType::Type::SMOKE:
        return tr("smoke");

    case metaf::CloudType::Type::FOG:
        return tr("fog");

    case metaf::CloudType::Type::MIST:
        return tr("mist");

    case metaf::CloudType::Type::HAZE:
        return tr("haze");

    case metaf::CloudType::Type::VOLCANIC_ASH:
        return tr("volcanic ash");
    }
}

QString Meteorologist::Decoder::convectiveTypeToString(metaf::CloudGroup::ConvectiveType type)
{
    switch (type) {
    case metaf::CloudGroup::ConvectiveType::NONE:
        return QString();

    case metaf::CloudGroup::ConvectiveType::NOT_REPORTED:
        return tr("not reported");

    case metaf::CloudGroup::ConvectiveType::TOWERING_CUMULUS:
        return tr("towering cumulus");

    case metaf::CloudGroup::ConvectiveType::CUMULONIMBUS:
        return tr("cumulonimbus");
    }
}

QString Meteorologist::Decoder::layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type)
{
    switch(type) {
    case metaf::LayerForecastGroup::Type::ICING_TRACE_OR_NONE:
        return tr("Trace icing or no icing");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_MIXED:
        return tr("Light mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_RIME_IN_CLOUD:
        return tr("Light rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_LIGHT_CLEAR_IN_PRECIPITATION:
        return tr("Light clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_MIXED:
        return tr("Moderate mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_RIME_IN_CLOUD:
        return tr("Moderate rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_MODERATE_CLEAR_IN_PRECIPITATION:
        return tr("Moderate clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_MIXED:
        return tr("Severe mixed icing");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_RIME_IN_CLOUD:
        return tr("Severe rime icing in cloud");

    case metaf::LayerForecastGroup::Type::ICING_SEVERE_CLEAR_IN_PRECIPITATION:
        return tr("Severe clear icing in precipitation");

    case metaf::LayerForecastGroup::Type::TURBULENCE_NONE:
        return tr("No turbulence");

    case metaf::LayerForecastGroup::Type::TURBULENCE_LIGHT:
        return tr("Light turbulence");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLEAR_AIR_OCCASIONAL:
        return tr("Occasional moderate turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLEAR_AIR_FREQUENT:
        return tr("Frequent moderate turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLOUD_OCCASIONAL:
        return tr("Occasional moderate turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_MODERATE_IN_CLOUD_FREQUENT:
        return tr("Frequent moderate turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLEAR_AIR_OCCASIONAL:
        return tr("Occasional severe turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLEAR_AIR_FREQUENT:
        return tr("Frequent severe turbulence in clear air");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLOUD_OCCASIONAL:
        return tr("Occasional severe turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_SEVERE_IN_CLOUD_FREQUENT:
        return tr("Frequent severe turbulence in cloud");

    case metaf::LayerForecastGroup::Type::TURBULENCE_EXTREME:
        return tr("Extreme turbulence");
    }
}

QString Meteorologist::Decoder::pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend)
{
    switch(trend) {
    case metaf::PressureTendencyGroup::Trend::NOT_REPORTED:
        return tr("not reported");

    case metaf::PressureTendencyGroup::Trend::HIGHER:
        return tr("higher than");

    case metaf::PressureTendencyGroup::Trend::HIGHER_OR_SAME:
        return tr("higher or the same as");

    case metaf::PressureTendencyGroup::Trend::SAME:
        return tr("same as");

    case metaf::PressureTendencyGroup::Trend::LOWER_OR_SAME:
        return tr("lower or the same as");

    case metaf::PressureTendencyGroup::Trend::LOWER:
        return tr("lower than");
    }
}

QString Meteorologist::Decoder::pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type)
{
    switch(type) {
    case metaf::PressureTendencyGroup::Type::INCREASING_THEN_DECREASING:
        return tr("increasing, then decreasing");

    case metaf::PressureTendencyGroup::Type::INCREASING_MORE_SLOWLY:
        return tr("increasing more slowly");

    case metaf::PressureTendencyGroup::Type::INCREASING:
        return tr("increasing");

    case metaf::PressureTendencyGroup::Type::INCREASING_MORE_RAPIDLY:
        return tr("increasing more rapidly");

    case metaf::PressureTendencyGroup::Type::STEADY:
        return tr("steady");

    case metaf::PressureTendencyGroup::Type::DECREASING_THEN_INCREASING:
        return tr("decreasing, then increasing");

    case metaf::PressureTendencyGroup::Type::DECREASING_MORE_SLOWLY:
        return tr("decreasing more slowly");

    case metaf::PressureTendencyGroup::Type::DECREASING:
        return tr("decreasing");

    case metaf::PressureTendencyGroup::Type::DECREASING_MORE_RAPIDLY:
        return tr("decreasing more rapidly");

    case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
        return tr("not reported");

    case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
        return tr("rising rapidly");

    case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
        return tr("falling rapidly");
    }
}

QString Meteorologist::Decoder::runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits)
{
    switch(deposits) {
    case metaf::RunwayStateGroup::Deposits::NOT_REPORTED:
        return tr("not reported");

    case metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY:
        return tr("clear and dry");

    case metaf::RunwayStateGroup::Deposits::DAMP:
        return tr("damp");

    case metaf::RunwayStateGroup::Deposits::WET_AND_WATER_PATCHES:
        return tr("wet and water patches");

    case metaf::RunwayStateGroup::Deposits::RIME_AND_FROST_COVERED:
        return tr("rime and frost covered");

    case metaf::RunwayStateGroup::Deposits::DRY_SNOW:
        return tr("dry snow");

    case metaf::RunwayStateGroup::Deposits::WET_SNOW:
        return tr("wet snow");

    case metaf::RunwayStateGroup::Deposits::SLUSH:
        return tr("slush");

    case metaf::RunwayStateGroup::Deposits::ICE:
        return tr("ice");

    case metaf::RunwayStateGroup::Deposits::COMPACTED_OR_ROLLED_SNOW:
        return tr("compacted or rolled snow");

    case metaf::RunwayStateGroup::Deposits::FROZEN_RUTS_OR_RIDGES:
        return tr("frozen ruts or ridges");
    }
    return QString();
}

QString Meteorologist::Decoder::runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent)
{
    switch(extent) {
    case metaf::RunwayStateGroup::Extent::NOT_REPORTED:
    case metaf::RunwayStateGroup::Extent::RESERVED_3:
    case metaf::RunwayStateGroup::Extent::RESERVED_4:
    case metaf::RunwayStateGroup::Extent::RESERVED_6:
    case metaf::RunwayStateGroup::Extent::RESERVED_7:
    case metaf::RunwayStateGroup::Extent::RESERVED_8:
        return tr("not reported");

    case metaf::RunwayStateGroup::Extent::NONE:
        return tr("none");

    case metaf::RunwayStateGroup::Extent::LESS_THAN_10_PERCENT:
        return QString("< 10%");

    case metaf::RunwayStateGroup::Extent::FROM_11_TO_25_PERCENT:
        return QString("11% -- 25%");

    case metaf::RunwayStateGroup::Extent::FROM_26_TO_50_PERCENT:
        return QString("26% -- 50%");

    case metaf::RunwayStateGroup::Extent::MORE_THAN_51_PERCENT:
        return QString(">51%");
    }
    return QString();
}

QString Meteorologist::Decoder::specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp)
{
    QStringList results;
    for (const auto &weather : wp.weather()) {

        // PRECIPITATION, undetermined
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy precipitation");
                break;
            default:
                return QString();
            }
            continue;
        }

        // DRIZZLE
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::DRIZZLE) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy drizzle");
                break;
            default:
                return QString();
            }
            continue;
        }

        // DUST, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::DUST) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy blowing dust");
                break;
            default:
                return QString();
            }
            continue;
        }

        // FOG
        if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::NONE && weather == metaf::WeatherPhenomena::Weather::FOG) {
            switch(wp.descriptor()) {
            case metaf::WeatherPhenomena::Descriptor::FREEZING:
                results << tr("freezing fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::PARTIAL:
                results << tr("partial fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::PATCHES:
                results << tr("patches of fog");
                break;
            case metaf::WeatherPhenomena::Descriptor::SHALLOW:
                results << tr("shallow fog");
                break;
            default:
                return QString();
            }
            continue;
        }


        // RAIN
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy rain");
                break;
            default:
                return QString();
            }
            continue;
        }

        // RAIN SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent rain showers");
                break;
            default:
                return QString();
            }
            continue;
        }

        // SNOW
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy snowfall");
                break;
            default:
                return QString();
            }
            continue;
        }

        // SNOW, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy blowing snow");
                break;
            default:
                return QString();
            }
            continue;
        }

        // SNOW, LOW_DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy drifting snow");
                break;
            default:
                return QString();
            }
            continue;
        }

        // SNOW SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy snow showers");
                break;
            default:
                return QString();
            }
            continue;
        }

        return QString();
    }

    return results.join(" • ");
}

QString Meteorologist::Decoder::stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface)
{
    switch(stateOfSurface) {
    case metaf::WaveHeight::StateOfSurface::NOT_REPORTED:
        return tr("not reported");

    case metaf::WaveHeight::StateOfSurface::CALM_GLASSY:
        return tr("calm (glassy), no waves");

    case metaf::WaveHeight::StateOfSurface::CALM_RIPPLED:
        return tr("calm (rippled), wave height <0.1 meters");

    case metaf::WaveHeight::StateOfSurface::SMOOTH:
        return tr("smooth, wave height 0.1 to 0.5 meters");

    case metaf::WaveHeight::StateOfSurface::SLIGHT:
        return tr("slight, wave height 0.5 to 1.25 meters");

    case metaf::WaveHeight::StateOfSurface::MODERATE:
        return tr("moderate, wave height 1.25 to 2.5 meters");

    case metaf::WaveHeight::StateOfSurface::ROUGH:
        return tr("rough, wave height 2.5 to 4 meters");

    case metaf::WaveHeight::StateOfSurface::VERY_ROUGH:
        return tr("very rough, wave height 4 to 6 meters");

    case metaf::WaveHeight::StateOfSurface::HIGH:
        return tr("high, wave height 6 to 9 meters");

    case metaf::WaveHeight::StateOfSurface::VERY_HIGH:
        return tr("very high, wave height 9 to 14 meters");

    case metaf::WaveHeight::StateOfSurface::PHENOMENAL:
        return tr("phenomenal, wave height >14 meters");
    }
    return QString();
}

QString Meteorologist::Decoder::weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor)
{
    switch(descriptor) {
    case metaf::WeatherPhenomena::Descriptor::NONE:
        return QString();

    case metaf::WeatherPhenomena::Descriptor::SHALLOW:
        return tr("shallow");

    case metaf::WeatherPhenomena::Descriptor::PARTIAL:
        return tr("partial");

    case metaf::WeatherPhenomena::Descriptor::PATCHES:
        return tr("patches");

    case metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING:
        return tr("low drifting");

    case metaf::WeatherPhenomena::Descriptor::BLOWING:
        return tr("blowing");

    case metaf::WeatherPhenomena::Descriptor::SHOWERS:
        return tr("showers");

    case metaf::WeatherPhenomena::Descriptor::THUNDERSTORM:
        return tr("thunderstorm");

    case metaf::WeatherPhenomena::Descriptor::FREEZING:
        return tr("freezing");
    }
}

QString Meteorologist::Decoder::weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier)
{
    switch (qualifier) {
    case metaf::WeatherPhenomena::Qualifier::NONE:
        return QString();

    case metaf::WeatherPhenomena::Qualifier::RECENT:
        return QString("recent");

    case metaf::WeatherPhenomena::Qualifier::VICINITY:
        return QString("in vicinity");

    case metaf::WeatherPhenomena::Qualifier::LIGHT:
        return tr("light");

    case metaf::WeatherPhenomena::Qualifier::MODERATE:
        return tr("moderate");

    case metaf::WeatherPhenomena::Qualifier::HEAVY:
        return tr("heavy");
    }
}

QString Meteorologist::Decoder::weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather)
{
    switch (weather) {
    case metaf::WeatherPhenomena::Weather::NOT_REPORTED:
        return QString();

    case metaf::WeatherPhenomena::Weather::DRIZZLE:
        return tr("drizzle");

    case metaf::WeatherPhenomena::Weather::RAIN:
        return tr("rain");

    case metaf::WeatherPhenomena::Weather::SNOW:
        return tr("snow");

    case metaf::WeatherPhenomena::Weather::SNOW_GRAINS:
        return tr("snow grains");

    case metaf::WeatherPhenomena::Weather::ICE_CRYSTALS:
        return tr("ice crystals");

    case metaf::WeatherPhenomena::Weather::ICE_PELLETS:
        return tr("ice pellets");

    case metaf::WeatherPhenomena::Weather::HAIL:
        return tr("hail");

    case metaf::WeatherPhenomena::Weather::SMALL_HAIL:
        return tr("small hail");

    case metaf::WeatherPhenomena::Weather::UNDETERMINED:
        return tr("undetermined precipitation");

    case metaf::WeatherPhenomena::Weather::MIST:
        return tr("mist");

    case metaf::WeatherPhenomena::Weather::FOG:
        return tr("fog");

    case metaf::WeatherPhenomena::Weather::SMOKE:
        return tr("smoke");

    case metaf::WeatherPhenomena::Weather::VOLCANIC_ASH:
        return tr("volcanic ash");

    case metaf::WeatherPhenomena::Weather::DUST:
        return tr("dust");

    case metaf::WeatherPhenomena::Weather::SAND:
        return tr("sand");

    case metaf::WeatherPhenomena::Weather::HAZE:
        return tr("haze");

    case metaf::WeatherPhenomena::Weather::SPRAY:
        return tr("spray");

    case metaf::WeatherPhenomena::Weather::DUST_WHIRLS:
        return tr("dust or sand whirls");

    case metaf::WeatherPhenomena::Weather::SQUALLS:
        return tr("squalls");

    case metaf::WeatherPhenomena::Weather::FUNNEL_CLOUD:
        return tr("funnel cloud");

    case metaf::WeatherPhenomena::Weather::SANDSTORM:
        return tr("sand storm");

    case metaf::WeatherPhenomena::Weather::DUSTSTORM:
        return tr("dust storm");
    }
}


// Visitor methods

QString Meteorologist::Decoder::visitCloudGroup(const CloudGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    const auto rw = group.runway();
    const auto d = group.direction();

    switch (group.type()) {
    case metaf::CloudGroup::Type::NO_CLOUDS:
        return cloudAmountToString(group.amount());

    case metaf::CloudGroup::Type::CLOUD_LAYER:
        if (group.convectiveType() != metaf::CloudGroup::ConvectiveType::NONE)
            return tr("%1 (%2) in %3 AGL")
                    .arg(cloudAmountToString(group.amount()))
                    .arg(convectiveTypeToString(group.convectiveType()))
                    .arg(explainDistance_FT(group.height()));
        return tr("%1 in %2 AGL")
                .arg(cloudAmountToString(group.amount()))
                .arg(explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VERTICAL_VISIBILITY:
        return tr("Sky obscured, vertical visibility is %1")
                .arg(explainDistance_FT(group.verticalVisibility()));

    case metaf::CloudGroup::Type::CEILING:
        if (rw.has_value() && d.has_value())
            return tr("Ceiling height %1 AGL at %2 towards %3")
                    .arg(explainDistance_FT(group.height()))
                    .arg(explainRunway(*rw))
                    .arg(explainDirection(*d));
        if (rw.has_value())
            return tr("Ceiling height %1 AGL at %2")
                    .arg(explainDistance_FT(group.height()))
                    .arg(explainRunway(*rw));
        if (d.has_value())
            return tr("Ceiling height %1 AGL towards %2")
                    .arg(explainDistance_FT(group.height()))
                    .arg(explainDirection(*d));
        return tr("Ceiling height %1")
                .arg(explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VARIABLE_CEILING:
        if (rw.has_value() && d.has_value())
            return tr("Ceiling height %1 -- %2 AGL at %3 towards %4")
                    .arg(explainDistance_FT(group.minHeight()))
                    .arg(explainDistance_FT(group.maxHeight()))
                    .arg(explainRunway(*rw))
                    .arg(explainDirection(*d));
        if (rw.has_value())
            return tr("Ceiling height %1 -- %2 AGL at %3")
                    .arg(explainDistance_FT(group.minHeight()))
                    .arg(explainDistance_FT(group.maxHeight()))
                    .arg(explainRunway(*rw));
        if (d.has_value())
            return tr("Ceiling height %1 -- %2 AGL towards %3")
                    .arg(explainDistance_FT(group.minHeight()))
                    .arg(explainDistance_FT(group.maxHeight()))
                    .arg(explainDirection(*d));
        return tr("Ceiling height %1 -- %2 AGL")
                .arg(explainDistance_FT(group.minHeight()))
                .arg(explainDistance_FT(group.maxHeight()));

    case metaf::CloudGroup::Type::CHINO:
        return tr("Ceiling data not awailable");

    case metaf::CloudGroup::Type::CLD_MISG:
        return tr("Sky condition data (cloud data) is missing");

    case metaf::CloudGroup::Type::OBSCURATION:
        const auto h = group.height().distance();
        const auto ct = group.cloudType();
        if (h.has_value() && !h.value() && ct.has_value())
            return tr("Ground-based obscuration, %1").arg(explainCloudType(ct.value()));
        if (h.has_value() && !h.value())
            return tr("Ground-based obscuration");
        if (h.has_value() && h.value() && ct.has_value())
            return tr("Aloft obscuration, %1").arg(explainCloudType(ct.value()));
        if (h.has_value() && h.value())
            return tr("Aloft obscuration");
        return explainCloudType(ct.value());
    }
    return QString();
}

QString Meteorologist::Decoder::visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    QStringList layers;
    const auto clouds = group.cloudTypes();
    for (auto i = 0u; i < clouds.size(); i++)
        layers << explainCloudType(clouds.at(i));
    return tr("Cloud layers: %1").arg(layers.join(" • "));
}

QString Meteorologist::Decoder::visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    if (!group.baseHeight().isReported() && !group.topHeight().isReported())
        return tr("%1 at all heights")
                .arg(layerForecastGroupTypeToString(group.type()));

    return tr("%1 at heights from %1 to %2.")
            .arg(layerForecastGroupTypeToString(group.type()))
            .arg(explainDistance(group.baseHeight()))
            .arg(explainDistance(group.topHeight()));
}

QString Meteorologist::Decoder::visitLightningGroup(const LightningGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    QStringList result;
    result << tr("Lightning strikes observed.");

    if (group.distance().isReported())
        result << tr("Distance %1.").arg(explainDistance(group.distance()));

    switch(group.frequency()) {
    case metaf::LightningGroup::Frequency::NONE:
        break;

    case metaf::LightningGroup::Frequency::OCCASIONAL:
        result << tr("Less than 1 strike per minute.");
        break;

    case metaf::LightningGroup::Frequency::FREQUENT:
        result << tr("1 -- 6 strikes per minute.");
        break;

    case metaf::LightningGroup::Frequency::CONSTANT:
        result << tr("More than 6 strikes per minute.");
        break;
    }

    if (group.isCloudGround() || group.isInCloud() || group.isCloudCloud() || group.isCloudAir()) {
        QStringList typeList;
        if (group.isCloudGround())
            typeList << tr("cloud-to-ground");
        if (group.isInCloud())
            typeList << tr("in-cloud");
        if (group.isCloudCloud())
            typeList << tr("cloud-to-cloud");
        if (group.isCloudAir())
            typeList << tr("cloud-to-air without strike to ground");
        if (!typeList.isEmpty())
            result << tr("Lightning types: %1.").arg(typeList.join(", "));
    }

    if (group.isUnknownType())
        result << tr("Lightning strike types not recognised by parser.");

    QStringList directionList;
    if (const auto directions = group.directions(); directions.size()) {
        directionList << explainDirectionSector(directions);
    }
    if (!directionList.isEmpty())
        result << tr("Lightning strikes observed in the following directions: %1").arg(directionList.join(", "));

    return result.join(" ");
}

QString Meteorologist::Decoder::visitLocationGroup(const LocationGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    return tr("Report for %1").arg(QString::fromStdString(group.toString()));
}

QString Meteorologist::Decoder::visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    return tr("Low cloud layer: %1 • Mid cloud layer: %2 • High cloud layer: %2")
            .arg(cloudLowLayerToString(group.lowLayer()))
            .arg(cloudMidLayerToString(group.midLayer()))
            .arg(cloudHighLayerToString(group.highLayer()));
}

QString Meteorologist::Decoder::visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    QString result;
    switch(group.type()) {
    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_6_HOURLY:
        return tr("Observed 6-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()))
                .arg(explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_24_HOURLY:
        return tr("Observed 24-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()))
                .arg(explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::FORECAST:
        if (group.minimum().isReported())
            result += tr("Minimum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.minimum()))
                    .arg(explainMetafTime(group.minimumTime().value()));
        if (group.maximum().isReported())
            result += " " + tr("Maximum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.maximum()))
                    .arg(explainMetafTime(group.maximumTime().value()));
        return result;
    }
    return QString();
}

QString Meteorologist::Decoder::visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString)
{
    if (!group.isValid())
        return tr("Invalid data");

    (void)reportPart; (void)rawString;
    std::ostringstream result;
    if (!group.isValid()) result << groupNotValidMessage << "\n";
    switch(group.type()) {
    case metaf::PrecipitationGroup::Type::TOTAL_PRECIPITATION_HOURLY:
        return tr("Total precipitation for the past hour: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_DEPTH_ON_GROUND:
        return tr("Snow depth on ground: %1")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_OR_6_HOURLY:
        return tr("Water equivalent of frozen precipitation for the last 3 or 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_HOURLY:
        return tr("Water equivalent of frozen precipitation for the last 3 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_6_HOURLY:
        return tr("Water equivalent of frozen precipitation for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_24_HOURLY:
        return tr("Water equivalent of frozen precipitation for the last 24 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_6_HOURLY:
        return tr("Snowfall for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::WATER_EQUIV_OF_SNOW_ON_GROUND:
        return tr("Water equivalent of snow on ground: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_HOUR:
        return tr("Ice accretion for the last hour: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_3_HOURS:
        return tr("Ice accretion for the last 3 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_6_HOURS:
        return tr("Ice accretion for the last 6 hours: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::PRECIPITATION_ACCUMULATION_SINCE_LAST_REPORT:
        return tr("Precipitation accumulation since last report: %1.")
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::SNOW_INCREASING_RAPIDLY:
        return tr("Snow increasing rapidly. For the last hour snow increased by %1. Total snowfall: %2.")
                .arg(explainPrecipitation(group.recent()))
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::RAINFALL_9AM_10MIN:
        return tr("Rainfall for the last 10 minutes before report release time: %1. Rainfall since 9:00 local time: %2.")
                .arg(explainPrecipitation(group.recent()))
                .arg(explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::PNO:
        return tr("Tipping bucket rain gauge INOP.");

    case metaf::PrecipitationGroup::Type::FZRANO:
        return tr("Freezing rain sensor INOP.");

    case metaf::PrecipitationGroup::Type::ICG_MISG:
        return tr("Icing data is missing.");

    case metaf::PrecipitationGroup::Type::PCPN_MISG:
        return tr("Precipitation data is missing.");
    }

    return QString();
}

QString Meteorologist::Decoder::visitPressureGroup(const PressureGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    switch(group.type()) {
    case metaf::PressureGroup::Type::OBSERVED_QNH:
        return tr("QNH: %1").arg(explainPressure(group.atmosphericPressure()));

    case metaf::PressureGroup::Type::FORECAST_LOWEST_QNH:
        return tr("Forecast lowest QNH: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::OBSERVED_QFE:
        return tr("QFE: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::SLPNO:
        return tr("QNH is not available");
        break;

    case metaf::PressureGroup::Type::PRES_MISG:
        return tr("Atmospheric pressure data is missing");
    }
    return QString();
}

QString Meteorologist::Decoder::visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    switch (group.type()) {
    case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
        return tr("3-hour pressure tendency is not reported. Absolute pressure change is %1.")
                .arg(explainPressure(group.difference()));

    case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
    case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
        return tr("Atmospheric pressure is %1")
                .arg(pressureTendencyTypeToString(group.type()));

    default:
        //: Note: the string %2 will be replaced by a text such as "less than"
        return tr("During last 3 hours the atmospheric pressure was %1. Now the atmospheric pressure is %2 3h ago. Absolute pressure change is %3")
                .arg(pressureTendencyTypeToString(group.type()))
                .arg(pressureTendencyTrendToString(metaf::PressureTendencyGroup::trend(group.type())))
                .arg(explainPressure(group.difference()));
    }
    return QString();
}

QString Meteorologist::Decoder::visitReportTimeGroup(const ReportTimeGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    return tr("Issued at %1").arg(explainMetafTime(group.time()));
}

QString Meteorologist::Decoder::visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    QString result = tr("State of %1:").arg(explainRunway(group.runway()));

    switch (group.type()) {
    case metaf::RunwayStateGroup::Type::RUNWAY_STATE:
        result += runwayStateDepositsToString(group.deposits());
        if (group.deposits() != metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY)
            result += ", " + tr("%1 of deposits, %2 of runway contaminated")
                    .arg(explainPrecipitation(group.depositDepth()))
                    .arg(runwayStateExtentToString(group.contaminationExtent()));
        result += ", " + explainSurfaceFriction(group.surfaceFriction());
        break;

    case metaf::RunwayStateGroup::Type::RUNWAY_CLRD:
        result += tr("deposits on runway were cleared or ceased to exist");
        result += ", " + explainSurfaceFriction(group.surfaceFriction());
        break;

    case metaf::RunwayStateGroup::Type::RUNWAY_SNOCLO:
        result += tr("runway closed due to snow accumulation");
        break;

    case metaf::RunwayStateGroup::Type::AERODROME_SNOCLO:
        return tr("Aerodrome closed due to snow accumulation");

    case metaf::RunwayStateGroup::Type::RUNWAY_NOT_OPERATIONAL:
        result += tr("runway is not operational");
    }
    return result;
}

QString Meteorologist::Decoder::visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    return tr("Sea surface temperature: %1, %2")
            .arg(explainTemperature(group.surfaceTemperature()))
            .arg(explainWaveHeight(group.waves()));
}

QString Meteorologist::Decoder::visitTemperatureGroup(const TemperatureGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    const auto rh = group.relativeHumidity();

    switch (group.type()) {
    case metaf::TemperatureGroup::Type::TEMPERATURE_AND_DEW_POINT:
        if (rh.has_value())
            return tr("Temperature %1, Dew point %2, Humidity %3%").arg(explainTemperature(group.airTemperature())).arg(explainTemperature(group.dewPoint())).arg(qRound(*rh));
        return tr("Temperature %1, Dew point %2").arg(explainTemperature(group.airTemperature())).arg(explainTemperature(group.dewPoint()));

    case metaf::TemperatureGroup::Type::T_MISG:
        return tr("Temperature data is missing");

    case metaf::TemperatureGroup::Type::TD_MISG:
        return tr("Dew point data is missing");
    }
    return QString();
}

QString Meteorologist::Decoder::visitWeatherGroup(const WeatherGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    // Gather string with list of phenomena
    QStringList phenomenaList;
    for (const auto p : group.weatherPhenomena())
        phenomenaList << Meteorologist::Decoder::explainWeatherPhenomena(p);
    auto phenomenaString = phenomenaList.join(" • ");


    switch (group.type()) {
    case metaf::WeatherGroup::Type::CURRENT:
        return phenomenaString; // tr("Current weather: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::RECENT:
        return tr("Recent weather: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::EVENT:
        return tr("Precipitation beginning/ending time: %1").arg(phenomenaString);

    case metaf::WeatherGroup::Type::NSW:
        return tr("No significant weather");

    case metaf::WeatherGroup::Type::PWINO:
        return tr("Automated weather identifier INOP");

    case metaf::WeatherGroup::Type::TSNO:
        return tr("Lightning detector INOP");

    case metaf::WeatherGroup::Type::WX_MISG:
        return tr("Weather phenomena data is missing");

    case metaf::WeatherGroup::Type::TS_LTNG_TEMPO_UNAVBL:
        return tr("Thunderstorm / lightning data is missing");
    }

    return QString();
}

QString Meteorologist::Decoder::visitWindGroup(const WindGroup & group, ReportPart, const std::string &)
{
    if (!group.isValid())
        return tr("Invalid data");

    switch (group.type()) {
    case metaf::WindGroup::Type::SURFACE_WIND_CALM:
        return tr("No wind");

    case metaf::WindGroup::Type::SURFACE_WIND:
        if (group.gustSpeed().isReported())
            return tr("Wind direction %1, wind speed %2, gusts at %3")
                    .arg(explainDirection(group.direction(), true))
                    .arg(explainSpeed(group.windSpeed()))
                    .arg(explainSpeed(group.gustSpeed()));
        return tr("Wind direction %1, wind speed %2")
                .arg(explainDirection(group.direction(), true))
                .arg(explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::VARIABLE_WIND_SECTOR:
        return tr("Variable wind direction %1 -- %2")
                .arg(explainDirection(group.varSectorBegin()))
                .arg(explainDirection(group.varSectorEnd()));

    case metaf::WindGroup::Type::SURFACE_WIND_WITH_VARIABLE_SECTOR:
        if (group.gustSpeed().isReported())
            return tr("Wind direction %1 (%2 -- %3), wind speed %4, gusts at %5")
                    .arg(explainDirection(group.direction(), true))
                    .arg(explainDirection(group.varSectorBegin()))
                    .arg(explainDirection(group.varSectorEnd()))
                    .arg(explainSpeed(group.windSpeed()))
                    .arg(explainSpeed(group.gustSpeed()));
        return tr("Wind direction %1 (%2 -- %3), wind speed %4")
                .arg(explainDirection(group.direction(), true))
                .arg(explainDirection(group.varSectorBegin()))
                .arg(explainDirection(group.varSectorEnd()))
                .arg(explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR:
        if (group.gustSpeed().isReported())
            return tr("Wind shear at %1 AGL, wind direction %2, wind speed %3, gusts at %4")
                    .arg(explainDistance_FT(group.height()))
                    .arg(explainDirection(group.direction(), true))
                    .arg(explainSpeed(group.windSpeed()))
                    .arg(explainSpeed(group.gustSpeed()));
        return tr("Wind shear at %1 AGL, wind direction %2, wind speed %3")
                .arg(explainDistance_FT(group.height()))
                .arg(explainDirection(group.direction(), true))
                .arg(explainSpeed(group.windSpeed()));


    case metaf::WindGroup::Type::WIND_SHIFT:
        if (group.eventTime())
            return tr("Wind direction changed at %1").arg(explainMetafTime(*group.eventTime()));
        return tr("Wind direction changed recently");

    case metaf::WindGroup::Type::WIND_SHIFT_FROPA:
        if (group.eventTime())
            return tr("Wind direction changed at %1 because of weather front passage").arg(explainMetafTime(*group.eventTime()));
        return tr("Wind directed changed recently because of weather front passage");

    case metaf::WindGroup::Type::PEAK_WIND:
        return tr("Peak wind observed at %1, wind direction %2, wind speed %3")
                .arg(explainMetafTime(group.eventTime().value()))
                .arg(explainDirection(group.direction(), true))
                .arg(explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR_IN_LOWER_LAYERS:
        if (const auto rw = group.runway(); rw.has_value())
            return  tr("Wind shear between runway level and 1.600 ft at runway %1").arg(explainRunway(*rw));
        return tr("Wind shear between runway level and 1.600 ft");

    case metaf::WindGroup::Type::WSCONDS:
        return tr("Potential wind shear");

    case metaf::WindGroup::Type::WND_MISG:
        return tr("Wind data is missing");
    }
    return QString();
}
