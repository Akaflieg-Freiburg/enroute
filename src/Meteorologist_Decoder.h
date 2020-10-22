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
    static QString explainCloudType(const metaf::CloudType ct);
    static QString explainDirection(const metaf::Direction & direction, bool trueCardinalDirections=true);
    static QString explainDistance_FT(const metaf::Distance & distance);
    static QString explainMetafTime(const metaf::MetafTime & metafTime);
    static QString explainPrecipitation(const metaf::Precipitation & precipitation);
    static QString explainPressure(const metaf::Pressure & pressure);
    static QString explainRunway(const metaf::Runway & runway);
    static QString explainSpeed(const metaf::Speed & speed);
    static QString explainSurfaceFriction(const metaf::SurfaceFriction & surfaceFriction);
    static QString explainTemperature(const metaf::Temperature & temperature);
    static QString explainWaveHeight(const metaf::WaveHeight & waveHeight);
    static QString explainWeatherPhenomena(const metaf::WeatherPhenomena & wp);

    // … toString Methods
    static QString brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction);
    static QString cardinalDirectionToString(metaf::Direction::Cardinal cardinal);
    static QString cloudAmountToString(metaf::CloudGroup::Amount amount);
    static QString cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer);
    static QString cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer);
    static QString cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer);
    static QString cloudTypeToString(metaf::CloudType::Type type);
    static QString convectiveTypeToString(metaf::CloudGroup::ConvectiveType type);
    static QString layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type);
    static QString pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend);
    static QString pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type);
    static QString runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits);
    static QString runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent);
    static QString specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp);
    static QString stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface);
    static QString weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor);
    static QString weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier);
    static QString weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather);

    // visitor Methods
    virtual QString visitCloudGroup(const CloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLightningGroup(const LightningGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLocationGroup(const LocationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureGroup(const PressureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitReportTimeGroup(const ReportTimeGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitTemperatureGroup(const TemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitWeatherGroup(const WeatherGroup & group, ReportPart reportPart, const std::string & rawString);
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
        if (!group.isValid())
            return tr("Invalid data");

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
        if (!group.isValid())
            return tr("Invalid data");

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

    QString explainDistance(const metaf::Distance & distance) {
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

        if (!distance.isValue())
            return QString::fromStdString(result.str());

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
        return QString::fromStdString(result.str());
    }

    virtual QString visitVisibilityGroup(const VisibilityGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        if (!group.isValid())
            return tr("Invalid data");

        (void)reportPart; (void)rawString;
        std::ostringstream result;

        if (!group.isValid()) result << groupNotValidMessage << "\n";
        switch (group.type()) {
        case metaf::VisibilityGroup::Type::PREVAILING:
            result << "Prevailing visibility is ";
            result << explainDistance(group.visibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::PREVAILING_NDV:
            result << "Prevailing visibility is ";
            result << explainDistance(group.visibility()).toStdString();
            result << "\nThis station cannot differentiate the directional ";
            result << "variation of visibility";
            break;

        case metaf::VisibilityGroup::Type::DIRECTIONAL:
            result << "Directional visibility toward ";
            result << explainDirection(group.direction().value()).toStdString();
            result << " is " << explainDistance(group.visibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::RUNWAY:
            result << "Visibility for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is " << explainDistance(group.visibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::RVR:
            if (!group.runway().has_value()) {
                result << "Runway visual range not reported ";
                break;
            }
            result << "Runway visual range for ";
            result << explainRunway(group.runway().value()).toStdString() << " is ";
            result << explainDistance(group.visibility()).toStdString();
            if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
                result << ", and the trend is " << visTrendToString(group.trend());
            }
            break;

        case metaf::VisibilityGroup::Type::SURFACE:
            result << "Visibility at surface level is ";
            result << explainDistance(group.visibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::TOWER:
            result << "Visibility from air traffic control tower is ";
            result << explainDistance(group.visibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::SECTOR:
            result << "Sector visibility is ";
            result << explainDistance(group.visibility()).toStdString();
            result << "\nIn the following directions: ";
            result << explainDirectionSector(group.sectorDirections()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_PREVAILING:
            result << "Prevailing visibility is variable from ";
            result << explainDistance(group.minVisibility()).toStdString();
            result << " to ";
            result << explainDistance(group.maxVisibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_DIRECTIONAL:
            result << "Directional visibility toward ";
            result << explainDirection(group.direction().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility()).toStdString();
            result << " to ";
            result << explainDistance(group.maxVisibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_RUNWAY:
            result << "Visibility for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility()).toStdString();
            result << " to ";
            result << explainDistance(group.maxVisibility()).toStdString();
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_RVR:
            result << "Runway visual range for ";
            result << explainRunway(group.runway().value()).toStdString();
            result << " is variable from ";
            result << explainDistance(group.minVisibility()).toStdString();
            result << " to ";
            result << explainDistance(group.maxVisibility()).toStdString();
            if (group.trend() != metaf::VisibilityGroup::Trend::NONE) {
                result << ", and the trend is " << visTrendToString(group.trend());
            }
            break;

        case metaf::VisibilityGroup::Type::VARIABLE_SECTOR:
            result << "Sector visibility is variable from ";
            result << explainDistance(group.minVisibility()).toStdString();
            result << " to ";
            result << explainDistance(group.maxVisibility()).toStdString();
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

    virtual QString visitVicinityGroup(const VicinityGroup & group, ReportPart reportPart, const std::string & rawString)
    {
        if (!group.isValid())
            return tr("Invalid data");

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
            result << explainDistance(group.distance()).toStdString();
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
        if (!group.isValid())
            return tr("Invalid data");

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
        if (!group.isValid())
            return tr("Invalid data");

        (void)group;(void)reportPart;
        std::ostringstream result;
        if (!group.isValid()) result << groupNotValidMessage << "\n";
        result << "Not recognised by parser: " << rawString;
        return QString::fromStdString(result.str());
    }

#warning docu
    QString _decodedText;
};

