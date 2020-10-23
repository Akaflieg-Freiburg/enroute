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
    static QString visTrendToString(metaf::VisibilityGroup::Trend trend);
    static QString weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor);
    static QString weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier);
    static QString weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather);

    // visitor Methods
    virtual QString visitCloudGroup(const CloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitKeywordGroup(const KeywordGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLightningGroup(const LightningGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLocationGroup(const LocationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitMiscGroup(const MiscGroup & group,  ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureGroup(const PressureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitReportTimeGroup(const ReportTimeGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitTemperatureGroup(const TemperatureGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitUnknownGroup(const UnknownGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitVicinityGroup(const VicinityGroup & group, ReportPart reportPart, const std::string & rawString);
    virtual QString visitVisibilityGroup(const VisibilityGroup & group, ReportPart reportPart, const std::string & rawString);
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

#warning docu
    QString _decodedText;
};

