/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

#include "Clock.h"
#include "GlobalSettings.h"
#include "weather/Decoder.h"


Weather::Decoder::Decoder(QObject *parent)
    : QObject(parent)
{
    // Re-parse the text whenever the date changes
    connect(Clock::globalInstance(), &Clock::dateChanged, this, &Weather::Decoder::parse);

    // Re-parse whenever the preferred unit system changes
    connect(GlobalSettings::globalInstance(), &GlobalSettings::useMetricUnitsChanged, this, &Weather::Decoder::parse);
}


auto Weather::Decoder::messageType() const -> QString
{
    switch(parseResult.reportMetadata.type) {
    case ReportType::METAR:
        if (parseResult.reportMetadata.isSpeci) {
            return "METAR/SPECI";
        }
        return "METAR";
    case ReportType::TAF:
        return "TAF";
    default:
        return "UNKNOWN";
    }
}


void Weather::Decoder::setRawText(const QString& rawText, QDate referenceDate)
{
    if ((_rawText == rawText) && (_referenceDate == referenceDate)) {
        return;
    }

    _referenceDate = referenceDate;
    _rawText = rawText;
    emit rawTextChanged();
    parse();
}


void Weather::Decoder::parse()
{
    auto oldDecodedText = _decodedText;

    parseResult = metaf::Parser::parse(_rawText.toStdString());
    QStringList decodedStrings;
    decodedStrings.reserve(64);
    QString listStart = "<ul style=\"margin-left:-25px;\">";
    QString listEnd = "</ul>";
    for (const auto &groupInfo : parseResult.groups) {
        auto decodedString = visit(groupInfo);
        if (decodedString.contains("<strong>")) {
            decodedStrings << listEnd+"<li>"+decodedString+"</li>"+listStart;
        }
        else {
            decodedStrings << "<li>"+decodedString+"</li>";
        }
    }
    _decodedText = listStart+decodedStrings.join("\n")+listEnd+"<br>";

    if (_decodedText != oldDecodedText) {
        emit decodedTextChanged();
    }
}


// explanation Methods

auto Weather::Decoder::explainCloudType(const metaf::CloudType ct) -> QString {
    const auto h = ct.height();
    if (h.isReported()) {
        return tr("Cloud cover %1/8, %2, base height %3")
                .arg(ct.okta())
                .arg(cloudTypeToString(ct.type()),
                     explainDistance_FT(ct.height()));
    }
    return tr("Cloud cover %1/8, %2")
            .arg(ct.okta())
            .arg(cloudTypeToString(ct.type()));
}

auto Weather::Decoder::explainDirection(metaf::Direction direction, bool trueCardinalDirections) -> QString
{
    switch (direction.type()) {
    case metaf::Direction::Type::NOT_REPORTED:
        return tr("not reported");

    case metaf::Direction::Type::VARIABLE:
        return tr("variable");

    case metaf::Direction::Type::NDV:
        return tr("no directional variation");

    case metaf::Direction::Type::VALUE_DEGREES:
        if (const auto d = direction.degrees(); d.has_value()) {
            return QString("%1°").arg(*d);
        }
        return tr("[unable to produce value in °]");

    case metaf::Direction::Type::VALUE_CARDINAL:
        if (auto c = cardinalDirectionToString(direction.cardinal(trueCardinalDirections)); !c.isEmpty()) {
            if (direction.type() == metaf::Direction::Type::VALUE_DEGREES) {
                return QString("(%1)").arg(c);
            }
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

auto Weather::Decoder::explainDirectionSector(const std::vector<metaf::Direction>& dir) -> QString
{
    std::string result;
    for (auto i=0U; i<dir.size(); i++) {
        if (i != 0U) { result += ", ";
}
        result += cardinalDirectionToString(dir[i].cardinal()).toStdString();
    }
    return QString::fromStdString(result);
}

auto Weather::Decoder::explainDistance(metaf::Distance distance) -> QString {
    if (!distance.isReported()) {
        return tr("not reported");
    }

    QStringList results;
    switch (distance.modifier()) {
    case metaf::Distance::Modifier::NONE:
        break;

    case metaf::Distance::Modifier::LESS_THAN:
        results << "<";
        break;

    case metaf::Distance::Modifier::MORE_THAN:
        results << ">";
        break;

    case metaf::Distance::Modifier::DISTANT:
        if (GlobalSettings::useMetricUnitsStatic()) {
            results << tr("19 to 55 km");
        } else {
            results << tr("10 to 30 NM");
        }
        break;

    case metaf::Distance::Modifier::VICINITY:
        if (GlobalSettings::useMetricUnitsStatic()) {
            results << tr("9 to 19 km");
        } else {
            results << tr("5 to 10 NM");
        }
        break;
    }

    if (!distance.isValue()) {
        return results.join(" ");
    }

    // Give distance in natural units
    if (distance.unit() == metaf::Distance::Unit::STATUTE_MILES) {
        const auto d = distance.miles();
        if (!d.has_value()) {
            return tr("[unable to get distance value in miles]");
        }
        const auto integer = std::get<unsigned int>(d.value());
        const auto fraction = std::get<metaf::Distance::MilesFraction>(d.value());
        if ((integer != 0U) || fraction == metaf::Distance::MilesFraction::NONE) {
            results << QString::number(integer);
        }
        if (fraction != metaf::Distance::MilesFraction::NONE) {
            results << distanceMilesFractionToString(fraction);
        }
        results << distanceUnitToString(distance.unit());
    } else if (distance.unit() == metaf::Distance::Unit::METERS) {
        const auto d = distance.toUnit(metaf::Distance::Unit::METERS);
        if (d.has_value()) {
            if ((*d) < 5000) {
                results << QString("%1 m").arg(qRound(*d));
            } else {
                results << QString("%1 km").arg( QString::number( *d/1000.0, 'f', 1));
            }
        } else {
            results << tr("[unable to convert distance to meters]");
        }
    } else {
        const auto d = distance.toUnit(distance.unit());
        if (!d.has_value()) {
            return tr("[unable to get distance's floating-point value]");
        }
        results << QString::number(qRound(*d));
        results << distanceUnitToString(distance.unit());
    }

    if (GlobalSettings::useMetricUnitsStatic() && (distance.unit() != metaf::Distance::Unit::METERS)) {
        const auto d = distance.toUnit(metaf::Distance::Unit::METERS);
        if (d.has_value()) {
            if ((*d) < 5000) {
                results << QString("(%1 m)").arg(qRound(*d));
            } else {
                results << QString("(%1 km)").arg( QString::number( *d/1000.0, 'f', 1));
            }
        } else {
            results << tr("[unable to convert distance to meters]");
        }
    }
    return results.join(" ");
}

auto Weather::Decoder::explainDistance_FT(metaf::Distance distance) -> QString {

    if (!distance.isReported()) {
        return tr("not reported");
    }

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

    if (!distance.isValue()) {
        return tr("no value");
    }

    const auto d = distance.toUnit(metaf::Distance::Unit::FEET);
    if (d.has_value()) {
        return modifier + QString("%1 ft").arg(qRound(*d));
    }

    return "[unable to convert distance to feet]";
}

auto Weather::Decoder::explainMetafTime(metaf::MetafTime metafTime) -> QString
{
    // QTime for result
    auto metafQTime = QTime(gsl::narrow_cast<int>(metafTime.hour()), gsl::narrow_cast<int>(metafTime.minute()) );

    auto currentQDate = QDate::currentDate().addDays(5);
    auto currentDate = metaf::MetafTime::Date(currentQDate.year(), currentQDate.month(), currentQDate.day());
    if (_referenceDate.isValid()) {
        currentDate = metaf::MetafTime::Date(_referenceDate.year(), _referenceDate.month(), _referenceDate.day());
    }
    auto metafDate = metafTime.dateBeforeRef(currentDate);
    auto metafQDate = QDate(gsl::narrow_cast<int>(metafDate.year), gsl::narrow_cast<int>(metafDate.month), gsl::narrow_cast<int>(metafDate.day) );

    auto metafQDateTime = QDateTime(metafQDate, metafQTime, QTimeZone::utc());
    return Clock::describePointInTime(metafQDateTime);
}

auto Weather::Decoder::explainPrecipitation(metaf::Precipitation precipitation) -> QString
{
    if (!precipitation.isReported()) {
        return "not reported";
    }

    if (const auto p = precipitation.amount(); p.has_value() && (*p == 0.0F)) {
        return tr("trace amount");
    }

    const auto p = precipitation.toUnit(metaf::Precipitation::Unit::MM);
    if (p.has_value()) {
        return QString("%1 mm").arg(QString::number(*p, 'f', 2));
    }
    return tr("[unable to convert precipitation to mm]");
}

auto Weather::Decoder::explainPressure(metaf::Pressure pressure) -> QString {

    if (!pressure.pressure().has_value()) {
        return tr("not reported");
    }

    const auto phpa = pressure.toUnit(metaf::Pressure::Unit::HECTOPASCAL);
    if (phpa.has_value()) {
        return QString("%1 hPa").arg(qRound(*phpa));
    }
    return tr("[unable to convert pressure to hPa]");
}

auto Weather::Decoder::explainRunway(metaf::Runway runway) -> QString {
    if (runway.isAllRunways()) {
        return tr("all runways");
    }
    if (runway.isMessageRepetition()) {
        return tr("same runway (repetition of last message)");
    }

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

auto Weather::Decoder::explainSpeed(metaf::Speed speed) -> QString {

    if (const auto s = speed.speed(); !s.has_value()) {
        return tr("not reported");
    }

    bool useMetric = false;
    auto *globalSettings = GlobalSettings::globalInstance();
    if (globalSettings != nullptr) {
        useMetric = globalSettings->useMetricUnits();
    }

    if (useMetric) {
        const auto s = speed.toUnit(metaf::Speed::Unit::KILOMETERS_PER_HOUR);
        if (s.has_value()) {
            return QString("%1 km/h").arg(qRound(*s));
        }
        return tr("[unable to convert speed to km/h]");
    }

    const auto s = speed.toUnit(metaf::Speed::Unit::KNOTS);
    if (s.has_value()) {
        return QString("%1 kt").arg(qRound(*s));
    }
    return tr("[unable to convert speed to knots]");
}

auto Weather::Decoder::explainSurfaceFriction(metaf::SurfaceFriction surfaceFriction) -> QString
{
    const auto c = surfaceFriction.coefficient();

    switch (surfaceFriction.type()) {
    case metaf::SurfaceFriction::Type::NOT_REPORTED:
        return tr("not reported");

    case metaf::SurfaceFriction::Type::SURFACE_FRICTION_REPORTED:
        if (c.has_value()) {
            return tr("friction coefficient %1").arg(QString::number(*c, 'f', 2));
        }
        return tr("[unable to produce a friction coefficient]");

    case metaf::SurfaceFriction::Type::BRAKING_ACTION_REPORTED:
        return tr("braking action %1").arg(brakingActionToString(surfaceFriction.brakingAction()));

    case metaf::SurfaceFriction::Type::UNRELIABLE:
        return tr("unreliable or unmeasurable");
    }
    return QString();
}

auto Weather::Decoder::explainTemperature(metaf::Temperature temperature) -> QString
{
    if (!temperature.temperature().has_value()) {
        return tr("not reported");
    }

    QString temperatureString = tr("[unable to convert temperature to °C]");
    const auto t = temperature.toUnit(metaf::Temperature::Unit::C);
    if (t.has_value()) {
        temperatureString = QString("%1 °C").arg(qRound(*t));
    }

    if (((*temperature.temperature()) == 0.0F) && !temperature.isPrecise()) {
        if (temperature.isFreezing()) {
            return tr("slightly less than %1").arg(temperatureString);
        }
        if (!temperature.isFreezing()) {
            return tr("slightly more than %1").arg(temperatureString);
        }
    }

    return temperatureString;
}

auto Weather::Decoder::explainWaveHeight(metaf::WaveHeight waveHeight) -> QString
{
    switch (waveHeight.type()) {
    case metaf::WaveHeight::Type::STATE_OF_SURFACE:
        return tr("state of sea surface: %1").arg(stateOfSeaSurfaceToString(waveHeight.stateOfSurface()));

    case metaf::WaveHeight::Type::WAVE_HEIGHT:
        if (waveHeight.isReported()) {
            const auto h = waveHeight.toUnit(metaf::WaveHeight::Unit::METERS);
            if (h.has_value()) {
                return tr("wave height: %1 m").arg(qRound(*h));
            }
            return tr("[unable to convert wave height to meters]");
        }
        return tr("wave height not reported");
    }
    return QString();
}

auto Weather::Decoder::explainWeatherPhenomena(const metaf::WeatherPhenomena & wp) -> QString
{
    /* Handle special cases */
    auto weatherStr = Weather::Decoder::specialWeatherPhenomenaToString(wp);
    if (!weatherStr.isEmpty()) {
        return weatherStr;
    }

    // Obtain strings for qualifier & descriptor
    auto qualifier = Weather::Decoder::weatherPhenomenaQualifierToString(wp.qualifier()); // Qualifier, such as "light" or "moderate"
    auto descriptor = Weather::Decoder::weatherPhenomenaDescriptorToString(wp.descriptor()); // Descriptor, such as "freezing" or "blowing"

    // String that will hold the result
    QString result;

    QStringList weatherPhenomena;
    weatherPhenomena.reserve(8);
    for (const auto w : wp.weather()) {
        // This is a string such as "hail" or "rain"
        auto wpString = Weather::Decoder::weatherPhenomenaWeatherToString(w);
        if (!wpString.isEmpty()) {
            weatherPhenomena << Weather::Decoder::weatherPhenomenaWeatherToString(w);
        }
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
    if (!qualifier.isEmpty()) {
        parenthesisTexts << qualifier;
    }
    if (!descriptor.isEmpty()) {
        parenthesisTexts << descriptor;
    }
    auto parenthesisText = parenthesisTexts.join(", ");
    if (!parenthesisText.isEmpty()) {
        result += QString(" (%1)").arg(parenthesisText);
    }


    const auto time = wp.time();
    switch (wp.event()){
    case metaf::WeatherPhenomena::Event::BEGINNING:
        if (!time.has_value()) {
            break;
        }
        result += " " + tr("began:") + " " + Weather::Decoder::explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::ENDING:
        if (!time.has_value()) {
            break;
        }
        result += " " + tr("ended:") + " " + Weather::Decoder::explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::NONE:
        break;
    }

    if (!parenthesisText.isEmpty()) {
        qWarning() << "Weather phenomena w/o special handling code" << result;
    }

    return result;
}


// …toString Methods

auto Weather::Decoder::brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction) -> QString
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
    return QString();
}

auto Weather::Decoder::cardinalDirectionToString(metaf::Direction::Cardinal cardinal) -> QString
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
    return QString();
}

auto Weather::Decoder::cloudAmountToString(metaf::CloudGroup::Amount amount) -> QString
{
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
    return QString();
}

auto Weather::Decoder::cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer) -> QString
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

auto Weather::Decoder::cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer) -> QString
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
    return QString();
}

auto Weather::Decoder::cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer) -> QString
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

auto Weather::Decoder::cloudTypeToString(metaf::CloudType::Type type) -> QString
{
    switch(type) {
    case metaf::CloudType::Type::NOT_REPORTED:
        return tr("unknown cloud type");

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
    return QString();
}

auto Weather::Decoder::convectiveTypeToString(metaf::CloudGroup::ConvectiveType type) -> QString
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
    return QString();
}

auto Weather::Decoder::distanceMilesFractionToString(metaf::Distance::MilesFraction f) -> QString
{
    switch (f) {
    case metaf::Distance::MilesFraction::NONE:
        return "";

    case metaf::Distance::MilesFraction::F_1_16:
        return "1/16";

    case metaf::Distance::MilesFraction::F_1_8:
        return "1/8";

    case metaf::Distance::MilesFraction::F_3_16:
        return "3/16";

    case metaf::Distance::MilesFraction::F_1_4:
        return "1/4";

    case metaf::Distance::MilesFraction::F_5_16:
        return "5/16";

    case metaf::Distance::MilesFraction::F_3_8:
        return "3/8";

    case metaf::Distance::MilesFraction::F_1_2:
        return "1/2";

    case metaf::Distance::MilesFraction::F_5_8:
        return "5/8";

    case metaf::Distance::MilesFraction::F_3_4:
        return "3/4";

    case metaf::Distance::MilesFraction::F_7_8:
        return "7/8";
    }
    return QString();
}

auto Weather::Decoder::distanceUnitToString(metaf::Distance::Unit unit) -> QString
{
    switch (unit) {
    case metaf::Distance::Unit::METERS:
        return "m";

    case metaf::Distance::Unit::STATUTE_MILES:
        return tr("statute miles");

    case metaf::Distance::Unit::FEET:
        return "ft";
    }
    return QString();
}

auto Weather::Decoder::layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type) -> QString
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
    return QString();
}

auto Weather::Decoder::pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend) -> QString
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
    return QString();
}

auto Weather::Decoder::pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type) -> QString
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
    return QString();
}

auto Weather::Decoder::probabilityToString(metaf::TrendGroup::Probability prob) -> QString
{
    switch (prob) {
    case metaf::TrendGroup::Probability::PROB_30:
        return tr("Probability 30%");

    case metaf::TrendGroup::Probability::PROB_40:
        return tr("Probability 40%");

    case metaf::TrendGroup::Probability::NONE:
        return QString();
    }
    return QString();
}

auto Weather::Decoder::runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits) -> QString
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

auto Weather::Decoder::runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent) -> QString
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

auto Weather::Decoder::specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp) -> QString
{
    QStringList results;
    for (const auto &weather : wp.weather()) {
        // DRIZZLE
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::DRIZZLE) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("drizzle in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy drizzle");
                break;
            }
            continue;
        }

        // DRIZZLE, FREEZING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::DRIZZLE) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("freezing drizzle in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy freezing drizzle");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent freezing drizzle");
                break;
            }
            continue;
        }

        // DUST, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::DUST) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("blowing dust");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("blowing dust in the vicinity");
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
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent blowing dust");
                break;
            }
            continue;
        }

        // FOG
        if (wp.qualifier() == metaf::WeatherPhenomena::Qualifier::NONE && weather == metaf::WeatherPhenomena::Weather::FOG) {
            switch(wp.descriptor()) {
            case metaf::WeatherPhenomena::Descriptor::NONE:
                results << tr("fog");
                break;
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

        // HAIL, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::HAIL) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("hail showers in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy hail showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent hail showers");
                break;
            }
            continue;
        }

        // HAIL, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::HAIL) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("thunderstorm with hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy thunderstorm with hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent thunderstorm with hail");
                break;
            }
            continue;
        }

        // ICE PELLETS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::ICE_PELLETS) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("ice pellet precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy ice pellet precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent ice pellet precipitation");
                break;
            }
            continue;
        }

        // PRECIPITATION
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
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("precipitation in the vicinity");
                break;
            }
            continue;
        }

        // PRECIPITATION, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("showers in the vicinity with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate showers with undetermined precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy showers with undetermined precipitation");
                break;
            }
            continue;
        }

        // PRECIPITATION, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::UNDETERMINED) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("thunderstorm with precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate thunderstorm with precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy thunderstorm with precipitation");
                break;
            }
            continue;
        }

        // RAIN
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("rain in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent rain");
                break;
            }
            continue;
        }

        // RAIN, FREEZING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent freezing rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("freezing rain in the vicinity");
                break;
            }
            continue;
        }

        // RAIN, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("rain showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("rain showers in the vicinity");
                break;
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
            }
            continue;
        }

        // RAIN, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::RAIN) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("thunderstorm with rain in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy thunderstorm with rain");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent thunderstorm with rain");
                break;
            }
            continue;
        }

        // SAND, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::SAND) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("blowing sand in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy blowing sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent blowing sand");
                break;
            }
            continue;
        }

        // SAND, LOW DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING && weather == metaf::WeatherPhenomena::Weather::SAND) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("low drifting sand in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy low drifting sand");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent low drifting sand");
                break;
            }
            continue;
        }

        // SNOW
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("snowfall in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy snowfall");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent snowfall");
                break;
            }
            continue;
        }

        // SNOW, BLOWING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::BLOWING && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("blowing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("blowing snow in the vicinity");
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
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent blowing snow");
                break;
            }
            continue;
        }

        // SNOW GRAINS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::NONE && weather == metaf::WeatherPhenomena::Weather::SNOW_GRAINS) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("snow grain precipitation in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy snow grain precipitation");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent snow grain precipitation");
                break;
            }
            continue;
        }

        // SNOW, LOW_DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::LOW_DRIFTING && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("low drifting snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy low drifting snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent low drifting snow");
                break;
            }
            continue;
        }

        // SNOW, LOW_DRIFTING
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::FREEZING && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("freezing snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy freezing snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent freezing snow");
                break;
            }
            continue;
        }

        // SNOW SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("snow showers in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy snow showers");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent snow showers");
                break;
            }
            continue;
        }

        // SNOW, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::SNOW) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("thunderstorm with snow in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy thunderstorm with snow");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent thunderstorm with snow");
                break;
            }
            continue;
        }

        // SMALL HAIL, SHOWERS
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::SHOWERS && weather == metaf::WeatherPhenomena::Weather::SMALL_HAIL) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("shower with small hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy shower with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent shower with small hail");
                break;
            }
            continue;
        }

        // SMALL HAIL, THUNDERSTORM
        if (wp.descriptor() == metaf::WeatherPhenomena::Descriptor::THUNDERSTORM && weather == metaf::WeatherPhenomena::Weather::SMALL_HAIL) {
            switch(wp.qualifier()) {
            case metaf::WeatherPhenomena::Qualifier::NONE:
                results << tr("thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::VICINITY:
                results << tr("thunderstorm with small hail in the vicinity");
                break;
            case metaf::WeatherPhenomena::Qualifier::LIGHT:
                results << tr("light thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::MODERATE:
                results << tr("moderate thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::HEAVY:
                results << tr("heavy thunderstorm with small hail");
                break;
            case metaf::WeatherPhenomena::Qualifier::RECENT:
                results << tr("recent thunderstorm with small hail");
                break;
            }
            continue;
        }

        return QString();
    }

    return results.join(" • ");
}

auto Weather::Decoder::stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface) -> QString
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

auto Weather::Decoder::visTrendToString(metaf::VisibilityGroup::Trend trend) -> QString
{
    switch(trend) {
    case metaf::VisibilityGroup::Trend::NONE:
        return QString();

    case metaf::VisibilityGroup::Trend::NOT_REPORTED:
        return tr("not reported");

    case metaf::VisibilityGroup::Trend::UPWARD:
        //: visibility trend
        return tr("upward");

    case metaf::VisibilityGroup::Trend::NEUTRAL:
        //: visibility trend
        return tr("neutral");

    case metaf::VisibilityGroup::Trend::DOWNWARD:
        //: visibility trend
        return tr("downward");
    }
    return QString();
}

auto Weather::Decoder::weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor) -> QString
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
    return QString();
}

auto Weather::Decoder::weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier) -> QString
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
    return QString();
}

auto Weather::Decoder::weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather) -> QString
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
    return QString();
}


// Visitor methods

auto Weather::Decoder::visitCloudGroup(const CloudGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    const auto rw = group.runway();
    const auto d = group.direction();

    switch (group.type()) {
    case metaf::CloudGroup::Type::NO_CLOUDS:
        return cloudAmountToString(group.amount());

    case metaf::CloudGroup::Type::CLOUD_LAYER:
        if (group.convectiveType() != metaf::CloudGroup::ConvectiveType::NONE) {
            return tr("%1 (%2) in %3 AGL")
                    .arg(cloudAmountToString(group.amount()),
                         convectiveTypeToString(group.convectiveType()),
                         explainDistance_FT(group.height()));
        }
        return tr("%1 in %2 AGL")
                .arg(cloudAmountToString(group.amount()),
                     explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VERTICAL_VISIBILITY:
        return tr("Vertical visibility %1")
                .arg(explainDistance_FT(group.verticalVisibility()));

    case metaf::CloudGroup::Type::CEILING:
        if (rw.has_value() && d.has_value()) {
            return tr("Ceiling height %1 AGL at %2 towards %3")
                    .arg(explainDistance_FT(group.height()),
                         explainRunway(*rw),
                         explainDirection(*d));
        }
        if (rw.has_value()) {
            return tr("Ceiling height %1 AGL at %2")
                    .arg(explainDistance_FT(group.height()),
                         explainRunway(*rw));
        }
        if (d.has_value()) {
            return tr("Ceiling height %1 AGL towards %2")
                    .arg(explainDistance_FT(group.height()),
                         explainDirection(*d));
        }
        return tr("Ceiling height %1")
                .arg(explainDistance_FT(group.height()));

    case metaf::CloudGroup::Type::VARIABLE_CEILING:
        if (rw.has_value() && d.has_value()) {
            return tr("Ceiling height %1 -- %2 AGL at %3 towards %4")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainRunway(*rw),
                         explainDirection(*d));
        }
        if (rw.has_value()) {
            return tr("Ceiling height %1 -- %2 AGL at %3")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainRunway(*rw));
        }
        if (d.has_value()) {
            return tr("Ceiling height %1 -- %2 AGL towards %3")
                    .arg(explainDistance_FT(group.minHeight()),
                         explainDistance_FT(group.maxHeight()),
                         explainDirection(*d));
        }
        return tr("Ceiling height %1 -- %2 AGL")
                .arg(explainDistance_FT(group.minHeight()),
                     explainDistance_FT(group.maxHeight()));

    case metaf::CloudGroup::Type::CHINO:
        return tr("Ceiling data not awailable");

    case metaf::CloudGroup::Type::CLD_MISG:
        return tr("Sky condition data (cloud data) is missing");

    case metaf::CloudGroup::Type::OBSCURATION:
        const auto h = group.height().distance();
        const auto ct = group.cloudType();
        if (h.has_value() && (h.value() == 0.0F) && ct.has_value()) {
            return tr("Ground-based obscuration, %1").arg(explainCloudType(ct.value()));
        }
        if (h.has_value() && (h.value() == 0.0F)) {
            return tr("Ground-based obscuration");
        }
        if (h.has_value() && (h.value() != 0.0F) && ct.has_value()) {
            return tr("Aloft obscuration, %1").arg(explainCloudType(ct.value()));
        }
        if (h.has_value() && (h.value() != 0.0F)) {
            return tr("Aloft obscuration");
        }
        return explainCloudType(ct.value());
    }
    return QString();
}

auto Weather::Decoder::visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    QStringList layers;
    layers.reserve(5);
    const auto clouds = group.cloudTypes();
    for (const auto & cloud : clouds) {
        layers << explainCloudType(cloud);
    }
    return tr("Cloud layers: %1").arg(layers.join(" • "));
}

auto Weather::Decoder::visitKeywordGroup(const KeywordGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    switch (group.type()) {
    case metaf::KeywordGroup::Type::METAR:
        return tr("Report type: METAR");

    case metaf::KeywordGroup::Type::SPECI:
        return tr("Report type: unscheduled METAR");

    case metaf::KeywordGroup::Type::TAF:
        return tr("Report type: TAF");

    case metaf::KeywordGroup::Type::AMD:
        return tr("Amended report");

    case metaf::KeywordGroup::Type::NIL:
        return tr("Missing report");

    case metaf::KeywordGroup::Type::CNL:
        return tr("Cancelled report");

    case metaf::KeywordGroup::Type::COR:
        return tr("Correctional report");

    case metaf::KeywordGroup::Type::AUTO:
        return tr("Automated report");

    case metaf::KeywordGroup::Type::CAVOK:
        return tr("CAVOK");

    case metaf::KeywordGroup::Type::RMK:
        return tr("<strong>Remarks</strong>");

    case metaf::KeywordGroup::Type::MAINTENANCE_INDICATOR:
        return tr("Automated station requires maintenance");

    case metaf::KeywordGroup::Type::AO1:
        return tr("Automated station w/o precipitation discriminator");

    case metaf::KeywordGroup::Type::AO2:
        return tr("Automated station with precipitation discriminator");

    case metaf::KeywordGroup::Type::AO1A:
        return tr("Automated station w/o precipitation discriminator, report augmented by a human observer");

    case metaf::KeywordGroup::Type::AO2A:
        return tr("Automated station with precipitation discriminator, report augmented by a human observer");

    case metaf::KeywordGroup::Type::NOSPECI:
        return tr("Manual station, does not issue SPECI reports");
    }
    return QString();
}

auto Weather::Decoder::visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    if (!group.baseHeight().isReported() && !group.topHeight().isReported()) {
        return tr("%1 at all heights")
                .arg(layerForecastGroupTypeToString(group.type()));
    }

    return tr("%1 at heights from %2 to %3.")
            .arg(layerForecastGroupTypeToString(group.type()),
                 explainDistance(group.baseHeight()),
                 explainDistance(group.topHeight()));
}

auto Weather::Decoder::visitLightningGroup(const LightningGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    QStringList result;
    result << tr("Lightning strikes observed.");

    if (group.distance().isReported()) {
        result << tr("Distance %1.").arg(explainDistance(group.distance()));
    }

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
        if (group.isCloudGround()) {
            typeList << tr("cloud-to-ground");
        }
        if (group.isInCloud()) {
            typeList << tr("in-cloud");
        }
        if (group.isCloudCloud()) {
            typeList << tr("cloud-to-cloud");
        }
        if (group.isCloudAir()) {
            typeList << tr("cloud-to-air without strike to ground");
        }
        if (!typeList.isEmpty()) {
            result << tr("Lightning types: %1.").arg(typeList.join(", "));
        }
    }

    if (group.isUnknownType()) {
        result << tr("Lightning strike types not recognised by parser.");
    }

    QStringList directionList;
    if (const auto directions = group.directions(); !directions.empty()) {
        directionList << explainDirectionSector(directions);
    }
    if (!directionList.isEmpty()) {
        result << tr("Lightning strikes observed in the following directions: %1").arg(directionList.join(", "));
    }

    return result.join(" ");
}

auto Weather::Decoder::visitLocationGroup(const LocationGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    return tr("Report for %1").arg(QString::fromStdString(group.toString()));
}

auto Weather::Decoder::visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    return tr("Low cloud layer: %1 • Mid cloud layer: %2 • High cloud layer: %3")
            .arg(cloudLowLayerToString(group.lowLayer()),
                 cloudMidLayerToString(group.midLayer()),
                 cloudHighLayerToString(group.highLayer()));
}

auto Weather::Decoder::visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    QString result;
    switch(group.type()) {
    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_6_HOURLY:
        return tr("Observed 6-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()),
                     explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::OBSERVED_24_HOURLY:
        return tr("Observed 24-hourly minimum/maximum temperature: %1/%2")
                .arg(explainTemperature(group.minimum()),
                     explainTemperature(group.maximum()));

    case metaf::MinMaxTemperatureGroup::Type::FORECAST:
        if (group.minimum().isReported()) {
            result += tr("Minimum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.minimum()),
                         explainMetafTime(group.minimumTime().value()));
        }
        if (group.maximum().isReported()) {
            result += " " + tr("Maximum forecast temperature: %1, expected at %2.")
                    .arg(explainTemperature(group.maximum()),
                         explainMetafTime(group.maximumTime().value()));
        }
        return result;
    }
    return QString();
}

auto Weather::Decoder::visitMiscGroup(const MiscGroup & group,  ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    static const QString colourCodeBlack = tr("Colour code BLACK: aerodrome closed due to snow accumulation or non-weather reasons");
    QString result;

    switch (group.type()) {
    case metaf::MiscGroup::Type::SUNSHINE_DURATION_MINUTES:
        if (const auto duration = group.data(); *duration) {
            return tr("Duration of sunshine that occurred the previous calendar day is %1 minutes.").arg(qRound(*duration));
        }
        return tr("No sunshine occurred the previous calendar day");

    case metaf::MiscGroup::Type::CORRECTED_WEATHER_OBSERVATION:
        return tr("This report is the corrected weather observation, correction number is %1").arg(static_cast<int>(*group.data()));

    case metaf::MiscGroup::Type::DENSITY_ALTITUDE:
        if (!group.data().has_value()) {
            return QString();
        }
        return tr("Density altitude is %1 feet").arg(qRound(*group.data()));

    case metaf::MiscGroup::Type::HAILSTONE_SIZE:
        return tr("Largest hailstone size is %1 inches").arg(*group.data());

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKBLUE:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_BLUE:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code BLUE: visibility >8000 m and lowest cloud base height >2500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKBLUE_PLUS:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_BLUE_PLUS:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code BLUE+: visibility >8000 m or lowest cloud base height >2000 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code YELLOW: visibility 1600-3700 m or lowest cloud base height 300-700 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKWHITE:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_WHITE:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code WHITE: visibility >5000 m and lowest cloud base height >1500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKGREEN:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_GREEN:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code GREEN: visibility >3700 m and lowest cloud base height >700 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW1:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW1:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code YELLOW 1: visibility >2500 m and lowest cloud base height >500 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW2:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW2:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code YELLOW 2: visibility >1600 m and lowest cloud base height >300 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKAMBER:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_AMBER:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code AMBER: visibility >800 m and lowest cloud base height >200 ft");
        return result;

    case metaf::MiscGroup::Type::COLOUR_CODE_BLACKRED:
        result = colourCodeBlack;
    case metaf::MiscGroup::Type::COLOUR_CODE_RED:
        if (!result.isEmpty()) {
            result += " ";
        }
        result += tr("Colour code RED: visibility <800 m or lowest cloud base height <200 ft");
        return result;

    case metaf::MiscGroup::Type::FROIN:
        return tr("Frost on the instrument (e.g. due to freezing fog depositing rime).");

    case metaf::MiscGroup::Type::ISSUER_ID_FN:
        return tr("Report issuer identifier is %1. This forecast is issued at The Fleet Weather Center Norfolk, VA.").arg(static_cast<int>(*group.data()));

    case metaf::MiscGroup::Type::ISSUER_ID_FS:
        return tr("Report issuer identifier is %1. This forecast is issued at The Fleet Weather Center San Diego, CA (FS).").arg(static_cast<int>(*group.data()));
    }
    return QString();
}

auto Weather::Decoder::visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

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
                .arg(explainPrecipitation(group.recent()),
                     explainPrecipitation(group.total()));

    case metaf::PrecipitationGroup::Type::RAINFALL_9AM_10MIN:
        return tr("Rainfall for the last 10 minutes before report release time: %1. Rainfall since 9:00 local time: %2.")
                .arg(explainPrecipitation(group.recent()),
                     explainPrecipitation(group.total()));

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

auto Weather::Decoder::visitPressureGroup(const PressureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    switch(group.type()) {
    case metaf::PressureGroup::Type::OBSERVED_QNH:
        return tr("QNH: %1").arg(explainPressure(group.atmosphericPressure()));

    case metaf::PressureGroup::Type::FORECAST_LOWEST_QNH:
        return tr("Forecast lowest QNH: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::OBSERVED_QFE:
        return tr("QFE: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::OBSERVED_SLP:
        return tr("Standard sea level pressure: %1").arg(explainPressure(group.atmosphericPressure()));
        break;

    case metaf::PressureGroup::Type::SLPNO:
        return tr("QNH is not available");
        break;

    case metaf::PressureGroup::Type::PRES_MISG:
        return tr("Atmospheric pressure data is missing");
    }
    return QString();
}

auto Weather::Decoder::visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

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
                .arg(pressureTendencyTypeToString(group.type()),
                     pressureTendencyTrendToString(metaf::PressureTendencyGroup::trend(group.type())),explainPressure(group.difference()));
    }
    return QString();
}

auto Weather::Decoder::visitReportTimeGroup(const ReportTimeGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    return tr("Issued at %1").arg(explainMetafTime(group.time()));
}

auto Weather::Decoder::visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    QString result = tr("State of %1:").arg(explainRunway(group.runway()));

    switch (group.type()) {
    case metaf::RunwayStateGroup::Type::RUNWAY_STATE:
        result += runwayStateDepositsToString(group.deposits());
        if (group.deposits() != metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY) {
            result += ", " + tr("%1 of deposits, %2 of runway contaminated")
                    .arg(explainPrecipitation(group.depositDepth()),runwayStateExtentToString(group.contaminationExtent()));
        }
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

auto Weather::Decoder::visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    return tr("Sea surface temperature: %1, %2")
            .arg(explainTemperature(group.surfaceTemperature()),
                 explainWaveHeight(group.waves()));
}

auto Weather::Decoder::visitTemperatureGroup(const TemperatureGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    const auto rh = group.relativeHumidity();

    switch (group.type()) {
    case metaf::TemperatureGroup::Type::TEMPERATURE_AND_DEW_POINT:
        if (rh.has_value()) {
            return tr("Temperature %1, Dew point %2, Humidity %3%")
                    .arg(explainTemperature(group.airTemperature()),
                         explainTemperature(group.dewPoint())).arg(qRound(*rh));
        }
        return tr("Temperature %1, Dew point %2")
                .arg(explainTemperature(group.airTemperature()),
                     explainTemperature(group.dewPoint()));

    case metaf::TemperatureGroup::Type::T_MISG:
        return tr("Temperature data is missing");

    case metaf::TemperatureGroup::Type::TD_MISG:
        return tr("Dew point data is missing");
    }
    return QString();
}

auto Weather::Decoder::visitTrendGroup(const TrendGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    const auto timeFrom = group.timeFrom();
    const auto timeUntil = group.timeUntil();
    const auto timeAt = group.timeAt();

    QString result;
    switch (group.type()) {
    case metaf::TrendGroup::Type::NOSIG:
        return tr("No significant weather changes expected");

    case metaf::TrendGroup::Type::BECMG:
        result += tr("Gradually changing");
        if (timeFrom) {
            result += " " + tr("from %1").arg(explainMetafTime(*timeFrom));
        }
        if (timeUntil) {
            result += " " + tr("until %1").arg(explainMetafTime(*timeUntil));
        }
        if (timeAt) {
            result += " " + tr("at %1").arg(explainMetafTime(*timeAt));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::TEMPO:
    case metaf::TrendGroup::Type::INTER:
        result += tr("Temporarily");
        if (timeFrom) {
            result += " " + tr("from %1").arg(explainMetafTime(*timeFrom));
        }
        if (timeUntil) {
            result += " " + tr("until %1").arg(explainMetafTime(*timeUntil));
        }
        if (timeAt) {
            result += " " + tr("at %1").arg(explainMetafTime(*timeAt));
        }
        if (group.probability() != metaf::TrendGroup::Probability::NONE) {
            result += QString(" (%1)").arg(probabilityToString(group.probability()));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::FROM:
        result = tr("Forecast: rapid weather change at %1")
                .arg(explainMetafTime(*group.timeFrom()));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::UNTIL:
        result = tr("Forecast until %1")
                .arg(explainMetafTime(*group.timeUntil()));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::AT:
        result = tr("Forecast for %1")
                .arg(explainMetafTime(*group.timeAt()));
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::TIME_SPAN:
        if (group.probability() != metaf::TrendGroup::Probability::NONE) {
            result = tr("Forecast from %1 to %2 (%3)")
                    .arg(explainMetafTime(*group.timeFrom()),
                         explainMetafTime(*group.timeUntil()),
                         probabilityToString(group.probability()));
        } else {
            result = tr("Forecast from %1 to %2")
                    .arg(explainMetafTime(*group.timeFrom()),
                         explainMetafTime(*group.timeUntil()));
        }
        return "<strong>" + result + "</strong>";

    case metaf::TrendGroup::Type::PROB:
        return tr("Forecast %1")
                .arg(probabilityToString(group.probability()));
    }
    return QString();
}

auto Weather::Decoder::visitUnknownGroup(const UnknownGroup & group, ReportPart /*reportPart*/, const std::string & rawString) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    return tr("Not recognised by parser: %1").arg(QString::fromStdString(rawString));
}

auto Weather::Decoder::visitVicinityGroup(const VicinityGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }


    QString type;
    switch (group.type()) {
    case metaf::VicinityGroup::Type::THUNDERSTORM:
        type = tr("Thunderstorm");
        break;

    case metaf::VicinityGroup::Type::CUMULONIMBUS:
        type = tr("Cumulonimbus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::CUMULONIMBUS_MAMMATUS:
        type = tr("Cumulonimbus cloud(s) with mammatus");
        break;

    case metaf::VicinityGroup::Type::TOWERING_CUMULUS:
        type = tr("Towering cumulus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ALTOCUMULUS_CASTELLANUS:
        type = tr("Altocumulus cloud(s)");
        break;

    case metaf::VicinityGroup::Type::STRATOCUMULUS_STANDING_LENTICULAR:
        type = tr("Stratocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ALTOCUMULUS_STANDING_LENTICULAR:
        type = tr("Altocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::CIRROCUMULUS_STANDING_LENTICULAR:
        type = tr("Cirrocumulus standing lenticular cloud(s)");
        break;

    case metaf::VicinityGroup::Type::ROTOR_CLOUD:
        type = tr("Rotor cloud(s)");
        break;

    case metaf::VicinityGroup::Type::VIRGA:
        type = tr("Virga");
        break;

    case metaf::VicinityGroup::Type::PRECIPITATION_IN_VICINITY:
        type = tr("Precipitation");
        break;

    case metaf::VicinityGroup::Type::FOG:
        type = tr("Fog");
        break;

    case metaf::VicinityGroup::Type::FOG_SHALLOW:
        type = tr("Shallow fog");
        break;

    case metaf::VicinityGroup::Type::FOG_PATCHES:
        type = tr("Patches of fog");
        break;

    case metaf::VicinityGroup::Type::HAZE:
        type = tr("Haze");
        break;

    case metaf::VicinityGroup::Type::SMOKE:
        type = tr("Smoke");
        break;

    case metaf::VicinityGroup::Type::BLOWING_SNOW:
        type = tr("Blowing snow");
        break;

    case metaf::VicinityGroup::Type::BLOWING_SAND:
        type = tr("Blowing sand");
        break;

    case metaf::VicinityGroup::Type::BLOWING_DUST:
        type = tr("Blowing dust");
        break;
    }

    //: %1 is string like 'Smoke'
    QStringList results;
    results << tr("%1 observed.");

    if (group.distance().isReported()) {
        results << tr("Distance %1.").arg(explainDistance(group.distance()));
    }
    if (const auto directions = group.directions(); !directions.empty()) {
        results << tr("Directions: %1").arg(explainDirectionSector(directions));
    }
    if (group.movingDirection().isReported()) {
        //: %1 is string like 'west'
        results << tr("Moving towards %1.").arg(cardinalDirectionToString(group.movingDirection().cardinal()));
    }

    return results.join(" ");
}

auto Weather::Decoder::visitVisibilityGroup(const VisibilityGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    switch (group.type()) {
    case metaf::VisibilityGroup::Type::PREVAILING:
        return tr("Visibility is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::PREVAILING_NDV:
        return tr("Visibility is %1. Station cannot differentiate the directional variation of visibility")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::DIRECTIONAL:
        return tr("Visibility toward %1 is %2")
                .arg(explainDirection(group.direction().value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::RUNWAY:
        return tr("Visibility for %1 is %2")
                .arg(explainRunway(group.runway().value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::RVR:
        if (!group.runway().has_value()) {
            return QString();
        }
        if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
            return tr("Runway visual range for %1 is %2 and the trend is %3")
                    .arg(explainRunway(group.runway().value()),
                         explainDistance(group.visibility()),
                         visTrendToString(group.trend()));
        }
        return tr("Runway visual range for %1 is %2")
                .arg(explainRunway(group.runway().value()),
                     explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::SURFACE:
        return tr("Visibility at surface level is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::TOWER:
        return tr("Visibility from air traffic control tower is %1")
                .arg(explainDistance(group.visibility()));

    case metaf::VisibilityGroup::Type::SECTOR:
        return tr("Sector visibility is %1 in the following directions %2")
                .arg(explainDistance(group.visibility()),
                     explainDirectionSector(group.sectorDirections()));

    case metaf::VisibilityGroup::Type::VARIABLE_PREVAILING:
        return tr("Visibility is variable from %1 to %2")
                .arg(explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_DIRECTIONAL:
        return tr("Directional visibility toward %1 is variable from %2 to %3")
                .arg(explainDirection(group.direction().value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_RUNWAY:
        return tr("Visibility for %1 is variable from %2 to %3")
                .arg(explainRunway(group.runway().value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_RVR:
        if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
            return tr("Runway visual range for %1 is variable from %2 to %3 and the trend is %4")
                    .arg(explainRunway(group.runway().value()),
                         explainDistance(group.minVisibility()),
                         explainDistance(group.maxVisibility()),
                         visTrendToString(group.trend()));
        }
        return tr("Runway visual range for %1 is variable from %2 to %3")
                .arg(explainRunway(group.runway().value()),
                     explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()));

    case metaf::VisibilityGroup::Type::VARIABLE_SECTOR:
        return tr("Sector visibility is variable from %1 to %2 in the following directions: %3")
                .arg(explainDistance(group.minVisibility()),
                     explainDistance(group.maxVisibility()),
                     explainDirectionSector(group.sectorDirections()));

    case metaf::VisibilityGroup::Type::VIS_MISG:
        return tr("Visibility data missing");

    case metaf::VisibilityGroup::Type::RVR_MISG:
        return tr("Runway visual range data is missing");

    case metaf::VisibilityGroup::Type::RVRNO:
        return tr("Runway visual range should be reported but is missing");

    case metaf::VisibilityGroup::Type::VISNO:
        const auto r = group.runway();
        const auto d = group.direction();
        if (r.has_value() && d.has_value()) {
            return tr("Visibility data not available for %1 in the direction of %2")
                    .arg(explainRunway(*r),
                         explainDirection(*d));
        }
        if (r.has_value()) {
            return tr("Visibility data not available for %1")
                    .arg(explainRunway(*r));
        }
        if (d.has_value()) {
            return tr("Visibility data not available in the direction of %1")
                    .arg(explainDirection(*d));
        }
        return tr("Visibility data not awailable");
    }
    return QString();
}

auto Weather::Decoder::visitWeatherGroup(const WeatherGroup & group, ReportPart part, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    // Gather string with list of phenomena
    QStringList phenomenaList;
    phenomenaList.reserve(8);
    for (const auto p : group.weatherPhenomena()) {
        phenomenaList << Weather::Decoder::explainWeatherPhenomena(p);
    }
    auto phenomenaString = phenomenaList.join(" • ");

    // If this is a METAR, then save the current weather
    if (part == ReportPart::METAR) {
        _currentWeather = phenomenaString;
    }

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

auto Weather::Decoder::visitWindGroup(const WindGroup & group, ReportPart /*reportPart*/, const std::string & /*rawString*/) -> QString
{
    if (!group.isValid()) {
        return tr("Invalid data");
    }

    switch (group.type()) {
    case metaf::WindGroup::Type::SURFACE_WIND_CALM:
        return tr("No wind");

    case metaf::WindGroup::Type::SURFACE_WIND:
        if (group.gustSpeed().isReported()) {
            return tr("Wind direction %1, wind speed %2, gusts at %3")
                    .arg(explainDirection(group.direction(), true),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return tr("Wind direction %1, wind speed %2")
                .arg(explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::VARIABLE_WIND_SECTOR:
        return tr("Variable wind direction %1 -- %2")
                .arg(explainDirection(group.varSectorBegin()),
                     explainDirection(group.varSectorEnd()));

    case metaf::WindGroup::Type::SURFACE_WIND_WITH_VARIABLE_SECTOR:
        if (group.gustSpeed().isReported()) {
            return tr("Wind direction %1 (%2 -- %3), wind speed %4, gusts at %5")
                    .arg(explainDirection(group.direction(), true),
                         explainDirection(group.varSectorBegin()),
                         explainDirection(group.varSectorEnd()),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return tr("Wind direction %1 (%2 -- %3), wind speed %4")
                .arg(explainDirection(group.direction(), true),
                     explainDirection(group.varSectorBegin()),
                     explainDirection(group.varSectorEnd()),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR:
        if (group.gustSpeed().isReported()) {
            return tr("Wind shear at %1 AGL, wind direction %2, wind speed %3, gusts at %4")
                    .arg(explainDistance_FT(group.height()),
                         explainDirection(group.direction(), true),
                         explainSpeed(group.windSpeed()),
                         explainSpeed(group.gustSpeed()));
        }
        return tr("Wind shear at %1 AGL, wind direction %2, wind speed %3")
                .arg(explainDistance_FT(group.height()),
                     explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));


    case metaf::WindGroup::Type::WIND_SHIFT:
        if (group.eventTime()) {
            return tr("Wind direction changed at %1").arg(explainMetafTime(*group.eventTime()));
        }
        return tr("Wind direction changed recently");

    case metaf::WindGroup::Type::WIND_SHIFT_FROPA:
        if (group.eventTime()) {
            return tr("Wind direction changed at %1 because of weather front passage").arg(explainMetafTime(*group.eventTime()));
        }
        return tr("Wind directed changed recently because of weather front passage");

    case metaf::WindGroup::Type::PEAK_WIND:
        return tr("Peak wind observed at %1, wind direction %2, wind speed %3")
                .arg(explainMetafTime(group.eventTime().value()),
                     explainDirection(group.direction(), true),
                     explainSpeed(group.windSpeed()));

    case metaf::WindGroup::Type::WIND_SHEAR_IN_LOWER_LAYERS:
        if (const auto rw = group.runway(); rw.has_value()) {
            return  tr("Wind shear between runway level and 1.600 ft at runway %1").arg(explainRunway(*rw));
        }
        return tr("Wind shear between runway level and 1.600 ft");

    case metaf::WindGroup::Type::WSCONDS:
        return tr("Potential wind shear");

    case metaf::WindGroup::Type::WND_MISG:
        return tr("Wind data is missing");
    }
    return QString();
}
