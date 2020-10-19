/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#pragma once

#include <QDateTime>
#include <QMultiMap>
#include <QGeoCoordinate>

#include "../3rdParty/metaf/include/metaf.hpp"

#include "GlobalSettings.h"
#include "Meteorologist.h"

using namespace metaf;


static const inline std::string groupNotValidMessage = "Data in this group may be errorneous, incomplete or inconsistent";


#warning docu

class Meteorologist::Decoder : public QObject, public Visitor<QString> {
    Q_OBJECT

public:
    explicit Decoder(QString rawText, QObject *parent = nullptr);

    QString decodedText() const
    {
        return _decodedText;
    }

    // Explanation functions
    static QString explainDirection(const metaf::Direction & direction, bool trueCardinalDirections=true);
    static QString explainDistance_FT(const metaf::Distance & distance);
    static QString explainMetafTime(const metaf::MetafTime & metafTime);
    static QString explainPressure(const metaf::Pressure & pressure);
    static QString explainRunway(const metaf::Runway & runway);
    static QString explainSpeed(const metaf::Speed & speed);
    static QString explainTemperature(const metaf::Temperature & temperature);
    static QString explainWeatherPhenomena(const metaf::WeatherPhenomena & wp);

    // … toString Methods
    static QString cardinalDirectionToString(metaf::Direction::Cardinal cardinal);
    static QString specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp);
    static QString weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor);
    static QString weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier);
    static QString weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather);

    // visitor Methods
    virtual QString visitPressureGroup(const PressureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitTemperatureGroup(const TemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitWindGroup(const WindGroup & group, ReportPart reportPart, const std::string & rawString);


    // ==================================================

    std::string roundTo(float number, size_t digitsAfterDecimalPoint)
    {
        static const char decimalPoint = '.';
        std::string numStr = std::to_string(number);
        const auto decimalPointPos = numStr.find(decimalPoint);
        if (decimalPointPos == std::string::npos) return numStr;
        const auto decimalsAfterPoint = numStr.length() - decimalPointPos;
        if (decimalsAfterPoint > digitsAfterDecimalPoint) {
            return numStr.substr(0, decimalPointPos + digitsAfterDecimalPoint + 1);
        }
        return numStr;
    }

    std::string_view speedUnitToString(metaf::Speed::Unit unit) {
        switch (unit) {
        case metaf::Speed::Unit::KNOTS:
            return "knots";

        case metaf::Speed::Unit::METERS_PER_SECOND:
            return "m/s";

        case metaf::Speed::Unit::KILOMETERS_PER_HOUR:
            return "km/h";

        case metaf::Speed::Unit::MILES_PER_HOUR:
            return "mph";
        }
    }

    QString explainDirectionSector(const std::vector<metaf::Direction> dir)
    {
        std::string result;
        for (auto i=0u; i<dir.size(); i++) {
            if (i) result += ", ";
            result += cardinalDirectionToString(dir[i].cardinal()).toStdString();
        }
        return QString::fromStdString(result);
    }

    std::string_view visTrendToString(metaf::VisibilityGroup::Trend trend)
    {
        switch(trend) {
        case metaf::VisibilityGroup::Trend::NONE:
            return std::string_view();

        case metaf::VisibilityGroup::Trend::NOT_REPORTED:
            return "not reported";

        case metaf::VisibilityGroup::Trend::UPWARD:
            return "upward";

        case metaf::VisibilityGroup::Trend::NEUTRAL:
            return "neutral";

        case metaf::VisibilityGroup::Trend::DOWNWARD:
            return "downward";
        }
    }

    virtual QString visitKeywordGroup(const KeywordGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        switch (group.type()) {
        case metaf::KeywordGroup::Type::METAR:
            result << "Report type: METAR (weather observation report)";
            break;

        case metaf::KeywordGroup::Type::SPECI:
            result << "Report type: unscheduled METAR ";
            result << "(weather observation report)";
            result << "\nUnscheduled report is issued due to sudden changes in ";
            result << "weather conditions: wind shift, visibility decrease, ";
            result << "severe weather, clouds formed or dissipated, etc.";
            break;

        case metaf::KeywordGroup::Type::TAF:
            result << "Report type: TAF (terminal aerodrome forecast)";
            break;

        case metaf::KeywordGroup::Type::AMD:
            result << "Amended report";
            break;

        case metaf::KeywordGroup::Type::NIL:
            result << "Missing report";
            break;

        case metaf::KeywordGroup::Type::CNL:
            result << "Cancelled report";
            break;

        case metaf::KeywordGroup::Type::COR:
            result << "Correctional report";
            break;

        case metaf::KeywordGroup::Type::AUTO:
            result << "Fully automated report with no human intervention ";
            result << "or oversight";
            break;

        case metaf::KeywordGroup::Type::CAVOK:
            result << "Ceiling and visibility OK";
            result << "\nVisibility 10 km or more in all directions, ";
            result << "no cloud below 5000 feet (1500 meters), ";
            result << "no cumulonimbus or towering cumulus clouds, ";
            result << "no significant weather phenomena";
            break;

        case metaf::KeywordGroup::Type::RMK:
            result << "The remarks are as follows";
            break;

        case metaf::KeywordGroup::Type::MAINTENANCE_INDICATOR:
            result << "Automated station requires maintenance";
            break;

        case metaf::KeywordGroup::Type::AO1:
            result << "This automated station is not equipped with a ";
            result << "precipitation discriminator";
            break;

        case metaf::KeywordGroup::Type::AO2:
            result << "This automated station is equipped with a ";
            result << "precipitation discriminator";
            break;

        case metaf::KeywordGroup::Type::AO1A:
            result << "This automated station is not equipped with a ";
            result << "precipitation discriminator";
            result << "\nAutomated observation is augmented by a human observer";
            break;

        case metaf::KeywordGroup::Type::AO2A:
            result << "This automated station is equipped with a ";
            result << "precipitation discriminator";
            result << "\nAutomated observation is augmented by a human observer";
            break;

        case metaf::KeywordGroup::Type::NOSPECI:
            result << "This manual station does not issue SPECI (unscheduled) ";
            result << "reports";
            break;
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitLocationGroup(const LocationGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        result << "ICAO code of station: " << group.toString();
        return QString::fromStdString(result.str());
    }

    virtual QString visitReportTimeGroup(const ReportTimeGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        QString result;

        if (!group.isValid())
            result += QString::fromStdString(groupNotValidMessage);
        result += "Day and time of report release: ";
        result += explainMetafTime(group.time());
        return result;
    }

    std::string_view probabilityToString(metaf::TrendGroup::Probability prob)
    {
        switch (prob) {
        case metaf::TrendGroup::Probability::PROB_30:
            return "Trend probability is 30 percent";

        case metaf::TrendGroup::Probability::PROB_40:
            return "Trend probability is 40 percent";

        case metaf::TrendGroup::Probability::NONE:
            return "";
        }
    }

    virtual QString visitTrendGroup(const TrendGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch (group.type()) {
        case metaf::TrendGroup::Type::NOSIG:
            result << "No significant weather changes expected";
            break;

        case metaf::TrendGroup::Type::BECMG:
            result << "Weather conditions are expected to gradually change ";
            result << "as follows";
            if (const auto timeFrom = group.timeFrom(); timeFrom) {
                result << "\nFrom ";
                result << explainMetafTime(*timeFrom).toStdString();
            }
            if (const auto timeUntil = group.timeUntil(); timeUntil) {
                result << "\nUntil ";
                result << explainMetafTime(*timeUntil).toStdString();
            }
            if (const auto timeAt = group.timeAt(); timeAt) {
                result << "\nAt ";
                result << explainMetafTime(*timeAt).toStdString();
            }
            break;

        case metaf::TrendGroup::Type::TEMPO:
            result << "The following temporary weather conditions may arise ";
            result << " for less than 60 minutes";
            if (const auto timeFrom = group.timeFrom(); timeFrom) {
                result << "\nFrom ";
                result << explainMetafTime(*timeFrom).toStdString();
            }
            if (const auto timeUntil = group.timeUntil(); timeUntil) {
                result << "\nUntil ";
                result << explainMetafTime(*timeUntil).toStdString();
            }
            if (const auto timeAt = group.timeAt(); timeAt) {
                result << "\nAt ";
                result << explainMetafTime(*timeAt).toStdString();
            }
            if (group.probability() != metaf::TrendGroup::Probability::NONE) {
                result << "\n";
                result << probabilityToString(group.probability());
            }
            break;

        case metaf::TrendGroup::Type::INTER:
            result << "The following temporary weather conditions may arise ";
            result << "for less than 30 minutes";
            if (const auto timeFrom = group.timeFrom(); timeFrom) {
                result << "\nFrom ";
                result << explainMetafTime(*timeFrom).toStdString();
            }
            if (const auto timeUntil = group.timeUntil(); timeUntil) {
                result << "\nUntil ";
                result << explainMetafTime(*timeUntil).toStdString();
            }
            if (const auto timeAt = group.timeAt(); timeAt) {
                result << "\nAt ";
                result << explainMetafTime(*timeAt).toStdString();
            }
            if (group.probability() != metaf::TrendGroup::Probability::NONE) {
                result << "\n";
                result << probabilityToString(group.probability());
            }
            break;

        case metaf::TrendGroup::Type::FROM:
            result << "Weather conditions are expected to change rapidly from ";
            result << explainMetafTime(*group.timeFrom()).toStdString();
            result << "\nAll previous weather conditions are superseded by ";
            result << "the following conditions";
            break;

        case metaf::TrendGroup::Type::UNTIL:
            result << "The following weather conditions expected to last until ";
            result << explainMetafTime(*group.timeUntil()).toStdString();
            break;

        case metaf::TrendGroup::Type::AT:
            result << "The following weather conditions expected to occur at ";
            result << explainMetafTime(*group.timeAt()).toStdString();
            break;

        case metaf::TrendGroup::Type::TIME_SPAN:
            result << "The following weather condition are expected within ";
            result << "time span from ";
            result << explainMetafTime(*group.timeFrom()).toStdString();
            result << " until ";
            result << explainMetafTime(*group.timeUntil()).toStdString();
            if (group.probability() != metaf::TrendGroup::Probability::NONE) {
                result << "\n";
                result << probabilityToString(group.probability());
            }
            break;

        case metaf::TrendGroup::Type::PROB:
            result << "The weather conditions probability is as follows\n";
            result << probabilityToString(group.probability());
            break;
        }
        return QString::fromStdString(result.str());
    }

    std::string_view distanceUnitToString(metaf::Distance::Unit unit)
    {
        switch (unit) {
        case metaf::Distance::Unit::METERS:
            return "meters";

        case metaf::Distance::Unit::STATUTE_MILES:
            return "statute miles";

        case metaf::Distance::Unit::FEET:
            return "feet";
        }
    }

    std::string_view distanceMilesFractionToString(metaf::Distance::MilesFraction f)
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
    }

    std::string explainDistance(const metaf::Distance & distance) {
        if (!distance.isReported()) return "not reported";
        std::ostringstream result;
        switch (distance.modifier()) {
        case metaf::Distance::Modifier::NONE:
            break;

        case metaf::Distance::Modifier::LESS_THAN:
            result << "&lt;";
            break;

        case metaf::Distance::Modifier::MORE_THAN:
            result << "&gt;";
            break;

        case metaf::Distance::Modifier::DISTANT:
            result << "10 to 30 nautical miles ";
            result << "(19 to 55 km, 12 to 35 statue miles)";
            break;

        case metaf::Distance::Modifier::VICINITY:
            result << "5 to 10 nautical miles ";
            result << "(9 to 19 km, 6 to 12 statue miles)";
            break;
        }

        if (!distance.isValue()) return result.str();

        if (distance.unit() == metaf::Distance::Unit::STATUTE_MILES) {
            const auto d = distance.miles();
            if (!d.has_value()) return "[unable to get distance value in miles]";
            const auto integer = std::get<unsigned int>(d.value());
            const auto fraction =
                    std::get<metaf::Distance::MilesFraction>(d.value());
            if (integer || fraction == metaf::Distance::MilesFraction::NONE)
                result << integer;
            if (integer && fraction != metaf::Distance::MilesFraction::NONE)
                result << " ";
            if (fraction != metaf::Distance::MilesFraction::NONE)
                result << distanceMilesFractionToString(fraction);
        } else {
            const auto d = distance.toUnit(distance.unit());
            if (!d.has_value())
                return "[unable to get distance's floating-point value]";
            result << static_cast<int>(*d);
        }
        result << " " << distanceUnitToString(distance.unit());
        result << " (";
        if (distance.unit() != metaf::Distance::Unit::METERS) {
            if (const auto d = distance.toUnit(metaf::Distance::Unit::METERS);
                    d.has_value())
            {
                result << static_cast<int>(*d);
                result << " ";
                result << distanceUnitToString(metaf::Distance::Unit::METERS);
            } else {
                result << "[unable to convert distance to meters]";
            }
            result << " / ";
        }
        if (distance.unit() != metaf::Distance::Unit::STATUTE_MILES) {
            if (const auto d = distance.miles(); d.has_value()) {
                if (!d.has_value())
                    return "[unable to get distance value in miles]";
                const auto integer = std::get<unsigned int>(d.value());
                const auto fraction =
                        std::get<metaf::Distance::MilesFraction>(d.value());
                if (integer || fraction == metaf::Distance::MilesFraction::NONE)
                    result << integer;
                if (integer && fraction != metaf::Distance::MilesFraction::NONE)
                    result << " ";
                if (fraction != metaf::Distance::MilesFraction::NONE)
                    result << distanceMilesFractionToString(fraction);
                result << " ";
                result << distanceUnitToString(metaf::Distance::Unit::STATUTE_MILES);
            } else {
                result << "[unable to convert distance to statute miles]";
            }
            if (distance.unit() != metaf::Distance::Unit::FEET) result << " / ";
        }
        if (distance.unit() != metaf::Distance::Unit::FEET) {
            if (const auto d = distance.toUnit(metaf::Distance::Unit::FEET);
                    d.has_value())
            {
                result << static_cast<int>(*d);
                result << " " << distanceUnitToString(metaf::Distance::Unit::FEET);
            } else {
                result << "[unable to convert distance to feet]";
            }
        }
        result << ")";
        return result.str();
    }

    virtual QString visitVisibilityGroup(const VisibilityGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;

        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch (group.type()) {
        case metaf::VisibilityGroup::Type::PREVAILING:
            result << "Prevailing visibility is ";
            result << explainDistance(group.visibility());
            break;

        case metaf::VisibilityGroup::Type::PREVAILING_NDV:
            result << "Prevailing visibility is ";
            result << explainDistance(group.visibility());
            result << "\nThis station cannot differentiate the directional ";
            result << "variation of visibility";
            break;

        case metaf::VisibilityGroup::Type::DIRECTIONAL:
            result << "Directional visibility toward ";
            result << explainDirection(group.direction().value()).toStdString();
            result << " is " << explainDistance(group.visibility());
            break;

        case metaf::VisibilityGroup::Type::RUNWAY:
            result << "Visibility for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is " << explainDistance(group.visibility());
            break;

        case metaf::VisibilityGroup::Type::RVR:
            if (!group.runway().has_value()) {
                result << "Runway visual range not reported ";
                break;
            }
            result << "Runway visual range for ";
            result << explainRunway(group.runway().value()).toStdString() << " is ";
            result << explainDistance(group.visibility());
            if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
                result << ", and the trend is " << visTrendToString(group.trend());
            }
            break;

        case metaf::VisibilityGroup::Type::SURFACE:
            result << "Visibility at surface level is ";
            result << explainDistance(group.visibility());
            break;

        case metaf::VisibilityGroup::Type::TOWER:
            result << "Visibility from air traffic control tower is ";
            result << explainDistance(group.visibility());
            break;

        case metaf::VisibilityGroup::Type::SECTOR:
            result << "Sector visibility is ";
            result << explainDistance(group.visibility());
            result << "\nIn the following directions: ";
            result << explainDirectionSector(group.sectorDirections()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_PREVAILING:
            result << "Prevailing visibility is variable from ";
            result << explainDistance(group.minVisibility());
            result << " to ";
            result << explainDistance(group.maxVisibility());
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_DIRECTIONAL:
            result << "Directional visibility toward ";
            result << explainDirection(group.direction().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility());
            result << " to ";
            result << explainDistance(group.maxVisibility());
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_RUNWAY:
            result << "Visibility for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility());
            result << " to ";
            result << explainDistance(group.maxVisibility());
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_RVR:
            result << "Runway visual range for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility());
            result << " to ";
            result << explainDistance(group.maxVisibility());
            if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
                result << ", and the trend is " << visTrendToString(group.trend());
            }
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_SECTOR:
            result << "Sector visibility is variable from ";
            result << explainDistance(group.minVisibility());
            result << " to ";
            result << explainDistance(group.maxVisibility());
            result << "\nIn the following directions: ";
            result << explainDirectionSector(group.sectorDirections()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VIS_MISG:
            result << "Visibility data missing";
            break;

        case metaf::VisibilityGroup::Type::RVR_MISG:
            result << "Runway visual range data is missing";
            break;

        case metaf::VisibilityGroup::Type::RVRNO:
            result << "Runway visual range should be reported but is missing";
            break;

        case metaf::VisibilityGroup::Type::VISNO:
            result << "Visibility data not awailable";
            if (const auto r = group.runway(); r.has_value()) {
                result << " for " << explainRunway(*r).toStdString();
            }
            if (const auto d = group.direction(); d.has_value()) {
                result << " in the direction of " << explainDirection(*d).toStdString();
            }
            break;
        }
        return QString::fromStdString(result.str());
    }

    std::string_view cloudTypeToString(metaf::CloudType::Type type)
    {
        switch(type) {
        case metaf::CloudType::Type::NOT_REPORTED:
            return "cumulonimbus";

        case metaf::CloudType::Type::CUMULONIMBUS:
            return "cumulonimbus";

        case metaf::CloudType::Type::TOWERING_CUMULUS:
            return "towering cumulus";

        case metaf::CloudType::Type::CUMULUS:
            return "cumulus";

        case metaf::CloudType::Type::CUMULUS_FRACTUS:
            return "cumulus fractus";

        case metaf::CloudType::Type::STRATOCUMULUS:
            return "stratocumulus";

        case metaf::CloudType::Type::NIMBOSTRATUS:
            return "nimbostratus";

        case metaf::CloudType::Type::STRATUS:
            return "stratus";

        case metaf::CloudType::Type::STRATUS_FRACTUS:
            return "stratus fractus";

        case metaf::CloudType::Type::ALTOSTRATUS:
            return "altostratus";

        case metaf::CloudType::Type::ALTOCUMULUS:
            return "altocumulus";

        case metaf::CloudType::Type::ALTOCUMULUS_CASTELLANUS:
            return "altocumulus castellanus";

        case metaf::CloudType::Type::CIRRUS:
            return "cirrus";

        case metaf::CloudType::Type::CIRROSTRATUS:
            return "cirrostratus";

        case metaf::CloudType::Type::CIRROCUMULUS:
            return "cirrocumulus";

        case metaf::CloudType::Type::BLOWING_SNOW:
            return "blowing snow";

        case metaf::CloudType::Type::BLOWING_DUST:
            return "blowing dust";

        case metaf::CloudType::Type::BLOWING_SAND:
            return "blowing sand";

        case metaf::CloudType::Type::ICE_CRYSTALS:
            return "ice crystals";

        case metaf::CloudType::Type::RAIN:
            return "rain";

        case metaf::CloudType::Type::DRIZZLE:
            return "drizzle";

        case metaf::CloudType::Type::SNOW:
            return "snow";

        case metaf::CloudType::Type::ICE_PELLETS:
            return "ice pellets";

        case metaf::CloudType::Type::SMOKE:
            return "smoke";

        case metaf::CloudType::Type::FOG:
            return "fog";

        case metaf::CloudType::Type::MIST:
            return "mist";

        case metaf::CloudType::Type::HAZE:
            return "haze";

        case metaf::CloudType::Type::VOLCANIC_ASH:
            return "volcanic ash";
        }
    }

    std::string explainCloudType(const metaf::CloudType ct) {
        std::ostringstream result;
        result << cloudTypeToString(ct.type());
        result << " covering ";
        result << ct.okta();
        result << "/8 of the sky";
        if (const auto h = ct.height(); h.isReported()) {
            result << ", base height ";
            result << explainDistance(ct.height());
        }
        return result.str();
    }

    std::string_view convectiveTypeToString(metaf::CloudGroup::ConvectiveType type)
    {
        switch (type) {
        case metaf::CloudGroup::ConvectiveType::NONE:
            return std::string_view();

        case metaf::CloudGroup::ConvectiveType::NOT_REPORTED:
            return "not reported";

        case metaf::CloudGroup::ConvectiveType::TOWERING_CUMULUS:
            return "towering cumulus";

        case metaf::CloudGroup::ConvectiveType::CUMULONIMBUS:
            return "cumulonimbus";
        }
    }

    std::string_view cloudAmountToString(metaf::CloudGroup::Amount amount) {
        switch (amount) {
        case metaf::CloudGroup::Amount::NOT_REPORTED:
            return "Cloud amount not reported";

        case metaf::CloudGroup::Amount::NSC:
            return "No significant cloud: "
                   "no cloud below 5000 feet (1500 meters), no cumulonimbus or towering "
                   "cumulus clouds, no vertical visibility restriction";

        case metaf::CloudGroup::Amount::NCD:
            return "No cloud detected: automated weather station did not detect any clouds; "
                   "this can happen due to either no clouds present or sensor error";

        case metaf::CloudGroup::Amount::NONE_CLR:
            return "Clear sky: "
                   "no cloud layers are detected at or below 12000 feet (3700 meters) (US) "
                   "or 25000 feet (7600 meters) (Canada); "
                   "indicates that station is at least partly automated";

        case metaf::CloudGroup::Amount::NONE_SKC:
            return "Clear sky: "
                   "In North America indicates report producted by human rather than "
                   "automatic weather station";

        case metaf::CloudGroup::Amount::FEW:
            return "Few clouds (1/8 to 2/8 sky covered)";

        case metaf::CloudGroup::Amount::SCATTERED:
            return "Scattered clouds (3/8 to 4/8 sky covered)";

        case metaf::CloudGroup::Amount::BROKEN:
            return "Broken clouds (5/8 to 7/8 sky covered)";

        case metaf::CloudGroup::Amount::OVERCAST:
            return "Overcast (8/8 sky covered)";

        case metaf::CloudGroup::Amount::OBSCURED:
            return "Sky obscured";

        case metaf::CloudGroup::Amount::VARIABLE_FEW_SCATTERED:
            return "Sky cover variable between few and scattered clouds (sky coverage variable between 1/8 and 4/8)";

        case metaf::CloudGroup::Amount::VARIABLE_SCATTERED_BROKEN:
            return "Sky cover variable between scattered and broken clouds (sky coverage variable between 3/8 and 7/8)";

        case metaf::CloudGroup::Amount::VARIABLE_BROKEN_OVERCAST:
            return "Sky cover variable between broken clouds and overcast (sky coverage variable between 5/8 and 8/8)";
        }
    }

    virtual QString visitCloudGroup(const CloudGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        switch (group.type()) {
        case metaf::CloudGroup::Type::NO_CLOUDS:
            result << cloudAmountToString(group.amount());
            break;

        case metaf::CloudGroup::Type::CLOUD_LAYER:
            result << "Cloud layer\n";
            result << cloudAmountToString(group.amount());
            result << "\nBase height ";
            result << explainDistance(group.height());
            if (group.convectiveType() != metaf::CloudGroup::ConvectiveType::NONE) {
                result << "\nConvective type: ";
                result << convectiveTypeToString(group.convectiveType());
            }
            break;

        case metaf::CloudGroup::Type::VERTICAL_VISIBILITY:
            result << "Sky is obscured, vertical visibility is ";
            result << explainDistance(group.verticalVisibility());
            break;

        case metaf::CloudGroup::Type::CEILING:
            result << "Ceiling height " << explainDistance(group.height());
            if (const auto rw = group.runway(); rw.has_value()) {
                result << " at " << explainRunway(*rw).toStdString();
            }
            if (const auto d = group.direction(); d.has_value()) {
                result << " towards " << explainDirection(*d).toStdString();
            }
            break;

        case metaf::CloudGroup::Type::VARIABLE_CEILING:
            result << "Ceiling height is variable between ";
            result << explainDistance(group.minHeight());
            result << " and ";
            result << explainDistance(group.maxHeight());
            if (const auto rw = group.runway(); rw.has_value()) {
                result << " at " << explainRunway(*rw).toStdString();
            }
            if (const auto d = group.direction(); d.has_value()) {
                result << " towards " << explainDirection(*d).toStdString();
            }
            break;

        case metaf::CloudGroup::Type::CHINO:
            result << "Ceiling data not awailable";
            if (const auto rw = group.runway(); rw.has_value()) {
                result << " at " << explainRunway(*rw).toStdString();
            }
            if (const auto d = group.direction(); d.has_value()) {
                result << " towards " << explainDirection(*d).toStdString();
            }
            break;

        case metaf::CloudGroup::Type::CLD_MISG:
            result << "Sky condition data (cloud data) is missing";
            break;

        case metaf::CloudGroup::Type::OBSCURATION:
            if (const auto h = group.height().distance(); h.has_value()) {
                if (!h.value()) {
                    result << "Ground-based obscuration\n";
                } else {
                    result << "Aloft obscuration\n";
                }
            }
            if (const auto ct = group.cloudType(); ct.has_value()) {
                result << explainCloudType(ct.value());
            }
            break;
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitWeatherGroup(const WeatherGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;

        if (!group.isValid())
            result << "Data in this group may be errorneous, incomplete or inconsistent" << "\n";

        // Gather string with list of phenomena
        QStringList phenomenaList;
        for (const auto p : group.weatherPhenomena())
            phenomenaList << Meteorologist::Decoder::explainWeatherPhenomena(p);
        auto phenomenaString = phenomenaList.join(" • ");


        switch (group.type()) {
        case metaf::WeatherGroup::Type::CURRENT:
            return "Current weather: " + phenomenaString;

        case metaf::WeatherGroup::Type::RECENT:
            return "Recent weather: " + phenomenaString;

        case metaf::WeatherGroup::Type::EVENT:
            return "Precipitation beginning/ending time: " + phenomenaString;

        case metaf::WeatherGroup::Type::NSW:
            return "no significant weather";

        case metaf::WeatherGroup::Type::PWINO:
            return "automated weather identifier is INOP";

        case metaf::WeatherGroup::Type::TSNO:
            return "lightning detector is INOP";

        case metaf::WeatherGroup::Type::WX_MISG:
            return "Weather phenomena data is missing";

        case metaf::WeatherGroup::Type::TS_LTNG_TEMPO_UNAVBL:
            return "thunderstorm / lightning data is missing";
        }
        return "";
    }

    std::string_view runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits)
    {
        switch(deposits) {
        case metaf::RunwayStateGroup::Deposits::NOT_REPORTED:
            return "not reported";

        case metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY:
            return "clear and dry";

        case metaf::RunwayStateGroup::Deposits::DAMP:
            return "damp";

        case metaf::RunwayStateGroup::Deposits::WET_AND_WATER_PATCHES:
            return "wet and water patches";

        case metaf::RunwayStateGroup::Deposits::RIME_AND_FROST_COVERED:
            return "rime and frost covered";

        case metaf::RunwayStateGroup::Deposits::DRY_SNOW:
            return "dry snow";

        case metaf::RunwayStateGroup::Deposits::WET_SNOW:
            return "wet snow";

        case metaf::RunwayStateGroup::Deposits::SLUSH:
            return "slush";

        case metaf::RunwayStateGroup::Deposits::ICE:
            return "ice";

        case metaf::RunwayStateGroup::Deposits::COMPACTED_OR_ROLLED_SNOW:
            return "compacted or rolled snow";

        case metaf::RunwayStateGroup::Deposits::FROZEN_RUTS_OR_RIDGES:
            return "frozen ruts or ridges";
        }
    }

    std::string explainPrecipitation(const metaf::Precipitation & precipitation)
    {
        std::ostringstream result;
        if (!precipitation.isReported()) return "not reported";
        if (const auto p = precipitation.amount(); p.has_value() && !*p)
            return "trace amount";
        if (const auto p = precipitation.toUnit(metaf::Precipitation::Unit::MM);
                p.has_value())
        {
            result << roundTo(*p, 1) << " mm";
        } else {
            result << "[unable to convert precipitation to mm]";
        }
        result << " / ";
        if (const auto p = precipitation.toUnit(metaf::Precipitation::Unit::INCHES);
                p.has_value())
        {
            result << roundTo(*p, 2) << " inches";
        } else {
            result << "[unable to convert precipitation to inches]";
        }
        return result.str();
    }

    std::string_view runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent)
    {
        switch(extent) {
        case metaf::RunwayStateGroup::Extent::NOT_REPORTED:
            return "not reported";

        case metaf::RunwayStateGroup::Extent::NONE:
            return "none";

        case metaf::RunwayStateGroup::Extent::LESS_THAN_10_PERCENT:
            return "&lt;10 percent";

        case metaf::RunwayStateGroup::Extent::FROM_11_TO_25_PERCENT:
            return "11 to 25 percent";

        case metaf::RunwayStateGroup::Extent::FROM_26_TO_50_PERCENT:
            return "26 to 50 percent";

        case metaf::RunwayStateGroup::Extent::MORE_THAN_51_PERCENT:
            return "&gt;51 percent";

        case metaf::RunwayStateGroup::Extent::RESERVED_3:
            return "[reserved_extent_value 3]";

        case metaf::RunwayStateGroup::Extent::RESERVED_4:
            return "[reserved_extent_value 4]";

        case metaf::RunwayStateGroup::Extent::RESERVED_6:
            return "[reserved_extent_value 6]";

        case metaf::RunwayStateGroup::Extent::RESERVED_7:
            return "[reserved_extent_value 7]";

        case metaf::RunwayStateGroup::Extent::RESERVED_8:
            return "[reserved_extent_value 8]";
        }
    }

    std::string explainSurfaceFriction(const metaf::SurfaceFriction & surfaceFriction)
    {
        switch (surfaceFriction.type()) {
        case metaf::SurfaceFriction::Type::NOT_REPORTED:
            return "not reported";

        case metaf::SurfaceFriction::Type::SURFACE_FRICTION_REPORTED:
            if (const auto c = surfaceFriction.coefficient(); c.has_value()) {
                return ("friction coefficient " + roundTo(*c, 2));
            }
            return "[unable to produce a friction coefficient]";

        case metaf::SurfaceFriction::Type::BRAKING_ACTION_REPORTED:
            return ("braking action " +
                    std::string(brakingActionToString(surfaceFriction.brakingAction())));

        case metaf::SurfaceFriction::Type::UNRELIABLE:
            return "unreliable or unmeasurable";
        }
    }

    std::string_view brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction)
    {
        switch(brakingAction) {
        case metaf::SurfaceFriction::BrakingAction::NONE:
            return "not reported";

        case metaf::SurfaceFriction::BrakingAction::POOR:
            return "poor (friction coefficient 0.0 to 0.25)";

        case metaf::SurfaceFriction::BrakingAction::MEDIUM_POOR:
            return "medium/poor (friction coefficient 0.26 to 0.29)";

        case metaf::SurfaceFriction::BrakingAction::MEDIUM:
            return "medium (friction coefficient 0.30 to 0.35)";

        case metaf::SurfaceFriction::BrakingAction::MEDIUM_GOOD:
            return "medium/good (friction coefficient 0.36 to 0.40)";

        case metaf::SurfaceFriction::BrakingAction::GOOD:
            return "good (friction coefficient 0.40 to 1.00)";
        }
    }

    virtual QString visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        result << "State of " << explainRunway(group.runway()).toStdString() << ":\n";

        switch (group.type()) {
        case metaf::RunwayStateGroup::Type::RUNWAY_STATE:
            result << runwayStateDepositsToString(group.deposits());
            if (group.deposits() !=
                    metaf::RunwayStateGroup::Deposits::CLEAR_AND_DRY)
            {
                result << "\nDepth of deposits on runway: ";
                result << explainPrecipitation(group.depositDepth());
                result << "\nRunway contamination extent: ";
                result << runwayStateExtentToString(group.contaminationExtent());
            }
            result << "\nSurface friction: ";
            result << explainSurfaceFriction(group.surfaceFriction());
            break;

        case metaf::RunwayStateGroup::Type::RUNWAY_CLRD:
            result << "Deposits on runway were cleared or ceased to exist";
            result << "\nSurface friction: ";
            result << explainSurfaceFriction(group.surfaceFriction());
            break;

        case metaf::RunwayStateGroup::Type::RUNWAY_SNOCLO:
            result << "Runway closed due to snow accumulation";
            break;

        case metaf::RunwayStateGroup::Type::AERODROME_SNOCLO:
            result << "Aerodrome closed due to snow accumulation";
            break;

        case metaf::RunwayStateGroup::Type::RUNWAY_NOT_OPERATIONAL:
            result << "Runway is not operational";
            break;
        }
        return QString::fromStdString(result.str());
    }

    std::string_view stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface)
    {
        switch(stateOfSurface) {
        case metaf::WaveHeight::StateOfSurface::NOT_REPORTED:
            return "not reported";

        case metaf::WaveHeight::StateOfSurface::CALM_GLASSY:
            return "calm (glassy), no waves";

        case metaf::WaveHeight::StateOfSurface::CALM_RIPPLED:
            return "calm (rippled), wave height &lt;0.1 meters / &lt;1/3 feet";

        case metaf::WaveHeight::StateOfSurface::SMOOTH:
            return "smooth, wave height 0.1 to 0.5 meters / 1/3 to 1 1/2 feet";

        case metaf::WaveHeight::StateOfSurface::SLIGHT:
            return "slight, wave height 0.5 to 1.25 meters / 1 1/2 to 4 feet";

        case metaf::WaveHeight::StateOfSurface::MODERATE:
            return "moderate, wave height 1.25 to 2.5 meters / 4 to 8 feet";

        case metaf::WaveHeight::StateOfSurface::ROUGH:
            return "rough, wave height 2.5 to 4 meters / 8 to 13 feet";

        case metaf::WaveHeight::StateOfSurface::VERY_ROUGH:
            return "very rough, wave height 4 to 6 meters / 13 to 20 feet";

        case metaf::WaveHeight::StateOfSurface::HIGH:
            return "high, wave height 6 to 9 meters / 20 to 30 feet";

        case metaf::WaveHeight::StateOfSurface::VERY_HIGH:
            return "very high, wave height 9 to 14 meters / 30 to 46 feet";

        case metaf::WaveHeight::StateOfSurface::PHENOMENAL:
            return "phenomenal, wave height >14 meters / &gt;46 feet";
        }
    }

    std::string explainWaveHeight(const metaf::WaveHeight & waveHeight)
    {
        switch (waveHeight.type()) {
        case metaf::WaveHeight::Type::STATE_OF_SURFACE:
            return ("state of sea surface: " +
                    std::string(stateOfSeaSurfaceToString(waveHeight.stateOfSurface())));

        case metaf::WaveHeight::Type::WAVE_HEIGHT:
            if (waveHeight.isReported()) {
                std::ostringstream result;
                result << "wave height: ";
                if (const auto h =
                        waveHeight.toUnit(metaf::WaveHeight::Unit::METERS);
                        h.has_value())
                {
                    result << roundTo(*h, 1) << " meters";
                } else {
                    result << "[unable to convert wave height to meters]";
                }
                result << " / ";
                if (const auto h =
                        waveHeight.toUnit(metaf::WaveHeight::Unit::FEET);
                        h.has_value())
                {
                    result << roundTo(*h, 1) << " feet";
                } else {
                    result << "[unable to convert wave height to feet]";
                }
                return result.str();
            }
            return "wave height not reported";
        }
    }

    virtual QString visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        result << "Sea surface temperature ";
        result << explainTemperature(group.surfaceTemperature()).toStdString() << ", ";
        result << explainWaveHeight(group.waves());
        return QString::fromStdString(result.str());
    }

    virtual QString visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch(group.type()) {
        case metaf::MinMaxTemperatureGroup::Type::OBSERVED_6_HOURLY:
            result << "Observed 6-hourly minimum/maximum temperature: ";
            result << "\nMinimum ambient air temperature: ";
            result << explainTemperature(group.minimum()).toStdString();
            result << "\nMaximum ambient air temperature: ";
            result << explainTemperature(group.maximum()).toStdString();
            break;

        case metaf::MinMaxTemperatureGroup::Type::OBSERVED_24_HOURLY:
            result << "Observed 24-hourly minimum/maximum temperature: ";
            result << "\nMinimum ambient air temperature: ";
            result << explainTemperature(group.minimum()).toStdString();
            result << "\nMaximum ambient air temperature: ";
            result << explainTemperature(group.maximum()).toStdString();
            break;

        case metaf::MinMaxTemperatureGroup::Type::FORECAST:
            result << "Forecast minimum/maximum temperature";
            if (group.minimum().isReported()) {
                result << "\nMinimum ambient air temperature: ";
                result << explainTemperature(group.minimum()).toStdString();
                result << ", expected at ";
                result << explainMetafTime(group.minimumTime().value()).toStdString();
            }
            if (group.maximum().isReported()) {
                result << "\nMaximum ambient air temperature: ";
                result << explainTemperature(group.maximum()).toStdString();
                result << ", expected at ";
                result << explainMetafTime(group.maximumTime().value()).toStdString();
            }
            break;
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch(group.type()) {
        case metaf::PrecipitationGroup::Type::TOTAL_PRECIPITATION_HOURLY:
            result << "Total precipitation for the past hour: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::SNOW_DEPTH_ON_GROUND:
            result << "Snow depth on ground: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_OR_6_HOURLY:
            result << "Water equivalent of frozen precipitation ";
            result << "for the last 3 or 6 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_3_HOURLY:
            result << "Water equivalent of frozen precipitation";
            result << " for the last 3 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_6_HOURLY:
            result << "Water equivalent of frozen precipitation";
            result << "for the last 6 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::FROZEN_PRECIP_24_HOURLY:
            result << "Water equivalent of frozen precipitation ";
            result << "for the last 24 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::SNOW_6_HOURLY:
            result << "Snowfall for the last 6 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::WATER_EQUIV_OF_SNOW_ON_GROUND:
            result << "Water equivalent of snow on ground: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_HOUR:
            result << "Ice accretion for the last hour: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_3_HOURS:
            result << "Ice accretion for the last 3 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::ICE_ACCRETION_FOR_LAST_6_HOURS:
            result << "Ice accretion for the last 6 hours: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::
        PRECIPITATION_ACCUMULATION_SINCE_LAST_REPORT:
            result << "Precipitation accumulation since last report: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::SNOW_INCREASING_RAPIDLY:
            result << "Snow increasing rapidly";
            result << "\nFor the last hour snow increased by ";
            result << explainPrecipitation(group.recent());
            result << "\nTotal snowfall: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::RAINFALL_9AM_10MIN:
            result << "Rainfall for the last 10 minutes before ";
            result << "report release time: ";
            result << explainPrecipitation(group.recent());
            result << "\nRainfall since 9AM (9:00) local time: ";
            result << explainPrecipitation(group.total());
            break;

        case metaf::PrecipitationGroup::Type::PNO:
            result << "This automated station is equipped with ";
            result << "tipping bucket rain gauge ";
            result << "and this sensor is not operating";
            break;

        case metaf::PrecipitationGroup::Type::FZRANO:
            result << "This automated station is equipped with ";
            result << "freezing rain sensor ";
            result << "and this sensor is not operating";
            break;

        case metaf::PrecipitationGroup::Type::ICG_MISG:
            result << "Icing data is missing";
            break;

        case metaf::PrecipitationGroup::Type::PCPN_MISG:
            result << "Precipitation data is missing";
            break;
        }
        return QString::fromStdString(result.str());
    }

    std::string_view layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type)
    {
        switch(type) {
        case metaf::LayerForecastGroup::Type::ICING_TRACE_OR_NONE:
            return "Trace icing or no icing";

        case metaf::LayerForecastGroup::Type::ICING_LIGHT_MIXED:
            return "Light mixed icing";

        case metaf::LayerForecastGroup::Type::ICING_LIGHT_RIME_IN_CLOUD:
            return "Light rime icing in cloud";

        case metaf::LayerForecastGroup::Type::
        ICING_LIGHT_CLEAR_IN_PRECIPITATION:
            return "Light clear icing in precipitation";

        case metaf::LayerForecastGroup::Type::ICING_MODERATE_MIXED:
            return "Moderate mixed icing";

        case metaf::LayerForecastGroup::Type::ICING_MODERATE_RIME_IN_CLOUD:
            return "Moderate rime icing in cloud";

        case metaf::LayerForecastGroup::Type::
        ICING_MODERATE_CLEAR_IN_PRECIPITATION:
            return "Moderate clear icing in precipitation";

        case metaf::LayerForecastGroup::Type::ICING_SEVERE_MIXED:
            return "Severe mixed icing";

        case metaf::LayerForecastGroup::Type::ICING_SEVERE_RIME_IN_CLOUD:
            return "Severe rime icing in cloud";

        case metaf::LayerForecastGroup::Type::
        ICING_SEVERE_CLEAR_IN_PRECIPITATION:
            return "Severe clear icing in precipitation";

        case metaf::LayerForecastGroup::Type::TURBULENCE_NONE:
            return "No turbulence";

        case metaf::LayerForecastGroup::Type::TURBULENCE_LIGHT:
            return "Light turbulence";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_MODERATE_IN_CLEAR_AIR_OCCASIONAL:
            return "Occasional moderate turbulence in clear air";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_MODERATE_IN_CLEAR_AIR_FREQUENT:
            return "Frequent moderate turbulence in clear air";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_MODERATE_IN_CLOUD_OCCASIONAL:
            return "Occasional moderate turbulence in cloud";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_MODERATE_IN_CLOUD_FREQUENT:
            return "Frequent moderate turbulence in cloud";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_SEVERE_IN_CLEAR_AIR_OCCASIONAL:
            return "Occasional severe turbulence in clear air";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_SEVERE_IN_CLEAR_AIR_FREQUENT:
            return "Frequent severe turbulence in clear air";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_SEVERE_IN_CLOUD_OCCASIONAL:
            return "Occasional severe turbulence in cloud";

        case metaf::LayerForecastGroup::Type::
        TURBULENCE_SEVERE_IN_CLOUD_FREQUENT:
            return "Frequent severe turbulence in cloud";

        case metaf::LayerForecastGroup::Type::TURBULENCE_EXTREME:
            return "Extreme turbulence";
        }
    }

    virtual QString visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        result << layerForecastGroupTypeToString(group.type());
        result << " at ";
        if (!group.baseHeight().isReported() && !group.topHeight().isReported()) {
            result << "all heights";
        } else {
            result << "heights from ";
            result << explainDistance(group.baseHeight());
            result << " to ";
            result << explainDistance(group.topHeight());
        }
        return QString::fromStdString(result.str());
    }

    std::string_view pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type)
    {
        switch(type) {
        case metaf::PressureTendencyGroup::Type::INCREASING_THEN_DECREASING:
            return "increasing, then decreasing";

        case metaf::PressureTendencyGroup::Type::INCREASING_MORE_SLOWLY:
            return "increasing, then steady, "
                   "or increasing then increasing more slowly";

        case metaf::PressureTendencyGroup::Type::INCREASING:
            return "increasing steadily or unsteadily";

        case metaf::PressureTendencyGroup::Type::INCREASING_MORE_RAPIDLY:
            return "decreasing or steady, then increasing; "
                   "or increasing then increasing more rapidly";

        case metaf::PressureTendencyGroup::Type::STEADY:
            return "steady";

        case metaf::PressureTendencyGroup::Type::DECREASING_THEN_INCREASING:
            return "decreasing, then increasing";

        case metaf::PressureTendencyGroup::Type::DECREASING_MORE_SLOWLY:
            return "decreasing then steady; "
                   "or decreasing then decreasing more slowly";

        case metaf::PressureTendencyGroup::Type::DECREASING:
            return "decreasing steadily or unsteadily";

        case metaf::PressureTendencyGroup::Type::DECREASING_MORE_RAPIDLY:
            return "steady or increasing, then decreasing; "
                   "or decreasing then decreasing more rapidly";

        case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
            return "not reported";

        case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
            return "rising rapidly at a rate of "
                   "at least 0.06 inch of mercury (2.03 hectopascal) per hour "
                   "and the pressure change totals "
                   "0.02 inch of mercury (0.68 hectopascal) or more "
                   "at the time of the observation";

        case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
            return "falling rapidly at a rate of "
                   "at least 0.06 inch of mercury (2.03 hectopascal) per hour "
                   "and the pressure change totals "
                   "0.02 inch of mercury (0.68 hectopascal) or more "
                   "at the time of the observation";
        }
    }

    std::string_view pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend)
    {
        switch(trend) {
        case metaf::PressureTendencyGroup::Trend::NOT_REPORTED:
            return "not reported";

        case metaf::PressureTendencyGroup::Trend::HIGHER:
            return "higher than";

        case metaf::PressureTendencyGroup::Trend::HIGHER_OR_SAME:
            return "higher or the same as";

        case metaf::PressureTendencyGroup::Trend::SAME:
            return "same as";

        case metaf::PressureTendencyGroup::Trend::LOWER_OR_SAME:
            return "lower or the same as";

        case metaf::PressureTendencyGroup::Trend::LOWER:
            return "lower than";
        }
    }

    virtual QString visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;

        if (!group.isValid()) result << groupNotValidMessage << "\n";

        switch (group.type()) {
        case metaf::PressureTendencyGroup::Type::NOT_REPORTED:
            result << "3-hourly pressure tendency is not reported";
            result << "\nAbsolute pressure change is ";
            result << explainPressure(group.difference()).toStdString();
            break;

        case metaf::PressureTendencyGroup::Type::RISING_RAPIDLY:
        case metaf::PressureTendencyGroup::Type::FALLING_RAPIDLY:
            result << "Atmospheric pressure is ";
            result << pressureTendencyTypeToString (group.type());
            break;

        default:
            result << "During last 3 hours the atmospheric pressure was ";
            result << pressureTendencyTypeToString (group.type());
            result << "\nNow the atmospheric pressure is ";
            result << pressureTendencyTrendToString(
                          metaf::PressureTendencyGroup::trend(group.type()));
            result << " 3 hours ago";
            result << "\nAbsolute pressure change is ";
            result << explainPressure(group.difference()).toStdString();
            break;
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        const auto clouds = group.cloudTypes();
        result << "Obscuration / cloud layers: ";
        for (auto i = 0u; i < clouds.size(); i++) {
            result << "\n";
            result << explainCloudType(clouds.at(i));
        }
        return QString::fromStdString(result.str());
    }

    std::string_view cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer)
    {
        switch(lowLayer) {
        case metaf::LowMidHighCloudGroup::LowLayer::NONE:
            return "No low layer clouds";

        case metaf::LowMidHighCloudGroup::LowLayer::CU_HU_CU_FR:
            return "Cumulus clouds showing little vertical extent "
                   "(Cumulus humilis or Cumulus fractus of dry weather or both)";

        case metaf::LowMidHighCloudGroup::LowLayer::CU_MED_CU_CON:
            return "Cumulus clouds showing moderate or significant vertical extent "
                   "(Cumulus mediocris or Cumulus congestus, with or without Cumulus "
                   "humilis or Cumulus fractus or stratocumulus, "
                   "all having their bases on the same level)";

        case metaf::LowMidHighCloudGroup::LowLayer::CB_CAL:
            return "Cumulonimbus clouds without fibrous or striated parts at "
                   "summit (Cumulonimbus calvus with or without Cumulus, "
                   "Stratocumulus or Stratus)";

        case metaf::LowMidHighCloudGroup::LowLayer::SC_CUGEN:
            return "Stratocumulus clouds resulting from "
                   "the spreading out of Cumulus (Stratocumulus cumulogenitus; "
                   "Cumulus may also be present)";

        case metaf::LowMidHighCloudGroup::LowLayer::SC_NON_CUGEN:
            return "Stratocumulus clouds not resulting from "
                   "the spreading out of Cumulus (Stratocumulus non-cumulogenitus)";

        case metaf::LowMidHighCloudGroup::LowLayer::ST_NEB_ST_FR:
            return "Stratus clouds which consist of a continuous "
                   "single sheet or layer with a fairly uniform base, or "
                   "transitory stage during the formation or "
                   "the dissipation of such clouds "
                   "(Stratus nebulosus or Stratus fractus of dry weather, or both)";

        case metaf::LowMidHighCloudGroup::LowLayer::ST_FR_CU_FR_PANNUS:
            return "Ragged grey clouds which form below precipitating clouds "
                   "(Stratus fractus or Cumulus fractus of wet weather, "
                   "or both (pannus) )";

        case metaf::LowMidHighCloudGroup::LowLayer::
        CU_SC_NON_CUGEN_DIFFERENT_LEVELS:
            return "Cumulus and Stratocumulus not formed by spreading of Cumulus "
                   "with bases at different levels";

        case metaf::LowMidHighCloudGroup::LowLayer::CB_CAP:
            return "Cumulonimbus clouds with fibrous or striated summits, "
                   "often with an anvil (Cumulonimbus capillatus or "
                   "Cumulonimbus capillatus incus)";

        case metaf::LowMidHighCloudGroup::LowLayer::NOT_OBSERVABLE:
            return "Clouds are not observable due to fog, blowing dust or sand, "
                   "or other similar phenomena";
        }
    }

    std::string_view cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer)
    {
        switch(midLayer) {
        case metaf::LowMidHighCloudGroup::MidLayer::NONE:
            return "No mid-layer clouds";

        case metaf::LowMidHighCloudGroup::MidLayer::AS_TR:
            return "A veil of greyish or bluish colour translucent enough "
                   "to reveal the position of the Sun or Moon "
                   "(Altostratus translucidus)";

        case metaf::LowMidHighCloudGroup::MidLayer::AS_OP_NS:
            return "A veil of a darker grey or a darker bluish grey dense enough "
                   "to completely mask the Sun or Moon "
                   "(Altostratus opacus or Nimbostratus)";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_TR:
            return "Altocumulus (mackerel sky) clouds in patches or sheets at "
                   "the same level or in a single layer "
                   "(Altocumulus translucidus at a single level)";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_LEN_PATCHES:
            return "Patches, often lenticular (lens or almond-shaped), "
                   "of Altocumulus translucidus, continually changing and "
                   "occurring at one or more levels";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_TR_AC_OP_SPREADING:
            return "Altocumulus translucidus in bands, or one or more layers of "
                   "Altocumulus translucidus or Altocumulus opacus, "
                   "progressively invading the sky";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_CUGEN_AC_CBGEN:
            return "Altocumulus resulting generally from the spreading "
                   "out of the summits of Cumulus; or Alcocumulus clouds "
                   "acompanying Cumulonimbus (Altocumulus cumulogenitus or "
                   "Altocumulus cumulonimbogenitus)";

        case metaf::LowMidHighCloudGroup::MidLayer::
        AC_DU_AC_OP_AC_WITH_AS_OR_NS:
            return "Altocumulus duplicatus, or Altocumulus opacus in a single "
                   "layer, not progressively invading the sky, or Altocumulus "
                   "with Altostratus or Nimbostratus.";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_CAS_AC_FLO:
            return "Turrets at appear to be arranged in lines with a "
                   "common horizontal base or scattered tufts with rounded and "
                   "slightly bulging upper parts (Altocumulus castellanus or "
                   "Altocumulus floccus)";

        case metaf::LowMidHighCloudGroup::MidLayer::AC_OF_CHAOTIC_SKY:
            return "Sky of chaotic, heavy and stagnant appearance, "
                   "which consists of superposed, more or less broken cloud sheets "
                   "of ill-defined species or varieties";

        case metaf::LowMidHighCloudGroup::MidLayer::NOT_OBSERVABLE:
            return "Clouds are not observable due to fog, blowing dust or sand, "
                   "or other similar phenomena "
                   "or because of a continuous layer of lower clouds";
        }
    }

    std::string_view cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer)
    {
        switch(highLayer) {
        case metaf::LowMidHighCloudGroup::HighLayer::NONE:
            return "No high-layer clouds";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC:
            return "Nearly straight or more or less curved filaments; more rarely, "
                   "they are shaped like commas topped with either a hook or a tuft "
                   "that is not rounded "
                   "(Cirrus fibratus and sometimes Cirrus uncinus, not "
                   "progressively invading the sky)";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CI_CAS_CI_FLO:
            return "Cirrus spissatus, in patches or entangled sheaves, "
                   "that usually do not increase and sometimes appear to be "
                   "the remains of the upper part of a Cumulonimbus; or "
                   "Cirrus clouds with small fibrous turrets rising from common base; "
                   "or more or less isolated tufts, often with trails "
                   "(Cirrus spissatus or Cirrus castellanus or Cirrus floccus)";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_SPI_CBGEN:
            return "Cirrus clouds originated from a Cumulonimbus cloud(s) "
                   "(Cirrus spissatus cumulonimbogenitus)";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_FIB_CI_UNC_SPREADING:
            return "Cirrus uncinus, Cirrus fibratus or both, "
                   "progressively invading the sky; they generally thicken as a whole";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_LOW_ABOVE_HORIZON:
            return "Cirrus (often in bands) and Cirrostratus, "
                   "or Cirrostratus alone, progressively invading the sky; "
                   "they generally thicken as a whole, "
                   "but the continuous veil does not reach 45&deg; above the horizon";

        case metaf::LowMidHighCloudGroup::HighLayer::CI_CS_HIGH_ABOVE_HORIZON:
            return "Cirrus (often in bands) and Cirrostratus, "
                   "or Cirrostratus alone, progressively invading the sky; "
                   "they generally thicken as a whole; "
                   "the continuous veil extends more than 45&deg; above the horizon, "
                   "without the sky being totally covered";

        case metaf::LowMidHighCloudGroup::HighLayer::
        CS_NEB_CS_FIB_COVERING_ENTIRE_SKY:
            return "Light, uniform and nebulous veil showing no distinct details"
                   "or a white and fibrous veil with more "
                   "or less clear-cut striations, covering the whole sky"
                   "(Cirrostratus nebulosus or "
                   "Cirrostratus fibratus covering the whole sky)";

        case metaf::LowMidHighCloudGroup::HighLayer::CS:
            return "A veil of Cirrostratus that is not (or no longer) "
                   "invading the sky progressively "
                   "and that does not completely cover the whole sky";

        case metaf::LowMidHighCloudGroup::HighLayer::CC:
            return "Cirrocumulus alone, or "
                   "predominant among the high-layer clouds; "
                   "when alone, its elements are frequently grouped into more or less "
                   "extensive patches with very characteristic small wavelets";

        case metaf::LowMidHighCloudGroup::HighLayer::NOT_OBSERVABLE:
            return "Clouds are not observable due to fog, blowing dust or sand, "
                   "or other similar phenomena "
                   "or because of a continuous layer of lower clouds";
        }
    }

    virtual QString visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        result << "Low cloud layer:\n";
        result << cloudLowLayerToString(group.lowLayer());
        result << "\nMid cloud layer:\n";
        result << cloudMidLayerToString(group.midLayer());
        result << "\nHigh cloud layer:\n";
        result << cloudHighLayerToString(group.highLayer());
        return QString::fromStdString(result.str());
    }

    virtual QString visitLightningGroup(const LightningGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        result << "Lightning strikes observed";
        if (group.distance().isReported())
        {
            result << " at distance ";
            result << explainDistance(group.distance());
        }

        switch(group.frequency()) {
        case metaf::LightningGroup::Frequency::NONE:
            break;

        case metaf::LightningGroup::Frequency::OCCASIONAL:
            result << "\nLess than 1 strike per minute";
            break;

        case metaf::LightningGroup::Frequency::FREQUENT:
            result << "\n1 to 6 strikes per minute";
            break;

        case metaf::LightningGroup::Frequency::CONSTANT:
            result << "\nMore than 6 strikes per minute";
            break;
        }

        if (group.isCloudGround() ||
                group.isInCloud() ||
                group.isCloudCloud() ||
                group.isCloudAir())
        {
            result << "\nThe following lightning types are observed: ";
            if (group.isCloudGround()) result << "\ncloud-to-ground";
            if (group.isInCloud()) result << "\nin-cloud";
            if (group.isCloudCloud()) result << "\ncloud-to-cloud";
            if (group.isCloudAir())
                result << "\ncloud-to-air without strike to ground";
        }
        if (group.isUnknownType())
        {
            result << "\nSome lightning strike types specified in this group ";
            result << "were not recognised by parser";
        }
        if (const auto directions = group.directions(); directions.size()) {
            result << "\nLightning strikes observed in the following directions: ";
            result << explainDirectionSector(directions).toStdString();
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitVicinityGroup(const VicinityGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch (group.type()) {
        case metaf::VicinityGroup::Type::THUNDERSTORM:
            result << "Thunderstorm";
            break;

        case metaf::VicinityGroup::Type::CUMULONIMBUS:
            result << "Cumulonimbus cloud(s)";
            break;

        case metaf::VicinityGroup::Type::CUMULONIMBUS_MAMMATUS:
            result << "Cumulonimbus cloud(s) with mammatus";
            break;

        case metaf::VicinityGroup::Type::TOWERING_CUMULUS:
            result << "Towering cumulus cloud(s)";
            break;

        case metaf::VicinityGroup::Type::ALTOCUMULUS_CASTELLANUS:
            result << "Altocumulus cloud(s)";
            break;

        case metaf::VicinityGroup::Type::STRATOCUMULUS_STANDING_LENTICULAR:
            result << "Stratocumulus standing lenticular cloud(s)";
            break;

        case metaf::VicinityGroup::Type::ALTOCUMULUS_STANDING_LENTICULAR:
            result << "Altocumulus standing lenticular cloud(s)";
            break;

        case metaf::VicinityGroup::Type::CIRROCUMULUS_STANDING_LENTICULAR:
            result << "Cirrocumulus standing lenticular cloud(s)";
            break;

        case metaf::VicinityGroup::Type::ROTOR_CLOUD:
            result << "Rotor cloud(s)";
            break;

        case metaf::VicinityGroup::Type::VIRGA:
            result << "Virga";
            break;

        case metaf::VicinityGroup::Type::PRECIPITATION_IN_VICINITY:
            result << "Precipitation";
            break;

        case metaf::VicinityGroup::Type::FOG:
            result << "Fog";
            break;

        case metaf::VicinityGroup::Type::FOG_SHALLOW:
            result << "Shallow fog";
            break;

        case metaf::VicinityGroup::Type::FOG_PATCHES:
            result << "Patches of fog";
            break;

        case metaf::VicinityGroup::Type::HAZE:
            result << "Haze";
            break;

        case metaf::VicinityGroup::Type::SMOKE:
            result << "Smoke";
            break;

        case metaf::VicinityGroup::Type::BLOWING_SNOW:
            result << "Blowing snow";
            break;

        case metaf::VicinityGroup::Type::BLOWING_SAND:
            result << "Blowing sand";
            break;

        case metaf::VicinityGroup::Type::BLOWING_DUST:
            result << "Blowing dust";
            break;
        }
        result << " observed";
        if (group.distance().isReported())
        {
            result << " at distance ";
            result << explainDistance(group.distance());
        }
        if (const auto directions = group.directions(); directions.size()) {
            result << "\nObserved in the following directions: ";
            result << explainDirectionSector(directions).toStdString();
        }
        if (group.movingDirection().isReported()) {
            result << "\nMoving towards ";
            result << cardinalDirectionToString(group.movingDirection().cardinal()).toStdString();
        }

        return QString::fromStdString(result.str());
    }

    virtual QString visitMiscGroup(const MiscGroup & group,  ReportPart reportPart, const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";

        static const std::string colourCodeBlack = "Colour code BLACK: "
                                                   "aerodrome closed due to snow accumulation or non-weather reasons";

        switch (group.type()) {
        case metaf::MiscGroup::Type::SUNSHINE_DURATION_MINUTES:
            if (const auto duration = group.data(); *duration) {
                result << "Duration of sunshine ";
                result << "that occurred the previous calendar day is ";
                result << *duration;
                result << " minutes";
            } else {
                result << "No sunshine occurred the previous calendar day";
            }
            break;

        case metaf::MiscGroup::Type::CORRECTED_WEATHER_OBSERVATION:
            result << "This report is the corrected weather observation, ";
            result << "correction number is ";
            result << static_cast<int>(*group.data());
            break;

        case metaf::MiscGroup::Type::DENSITY_ALTITUDE:
            result << "Density altitude ";
            if (!group.data().has_value()) {
                result << "data missing";
            } else {
                result << " is ";
                result << *group.data() << " feet";
            }
            break;

        case metaf::MiscGroup::Type::HAILSTONE_SIZE:
            result << "Largest hailstone size is ";
            result << *group.data();
            result << " inches";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKBLUE:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_BLUE:
            result << "Colour code BLUE: ";
            result << "visibility &gt;8000 m and ";
            result << "lowest cloud base height &gt;2500 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKWHITE:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_WHITE:
            result << "Colour code WHITE: ";
            result << "visibility &gt;5000 m and ";
            result << "lowest cloud base height &gt;1500 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKGREEN:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_GREEN:
            result << "Colour code GREEN: ";
            result << "visibility &gt;3700 m and ";
            result << "lowest cloud base height &gt;700 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW1:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW1:
            result << "Colour code YELLOW 1: ";
            result << "visibility &gt;2500 m and ";
            result << "lowest cloud base height &gt;500 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKYELLOW2:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_YELLOW2:
            result << "Colour code YELLOW 2: ";
            result << "visibility &gt;1600 m and ";
            result << "lowest cloud base height &gt;300 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKAMBER:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_AMBER:
            result << "Colour code AMBER: ";
            result << "visibility &gt;800 m and ";
            result << "lowest cloud base height &gt;200 ft";
            break;

        case metaf::MiscGroup::Type::COLOUR_CODE_BLACKRED:
            result << colourCodeBlack << "\n";
        case metaf::MiscGroup::Type::COLOUR_CODE_RED:
            result << "Colour code RED: ";
            result << "either visibility &lt;800 m or ";
            result << "lowest cloud base height &lt;200 ft ";
            result << "or both";
            break;

        case metaf::MiscGroup::Type::FROIN:
            result << "Frost on the instrument ";
            result << "(e.g. due to freezing fog depositing rime).";
            break;
        }
        return QString::fromStdString(result.str());
    }

    virtual QString visitUnknownGroup(const UnknownGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        (void)group;(void)reportPart;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        result << "Not recognised by parser: " << rawString;
        return QString::fromStdString(result.str());
    }

#warning docu
    QString _decodedText;
};

