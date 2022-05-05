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

/* This is a heavily altered version of a file from the metaf library package
 * Copyright (C) 2018-2020 Nick Naumenko (https://gitlab.com/nnaumenko)
 * Distributed under the terms of the MIT license.
 */

#pragma once

#include <QDate>
#include <QObject>

#include "../3rdParty/metaf/include/metaf.hpp"
using namespace metaf;

namespace Weather {


/*! \brief METAR/TAF decoder
 *
 * This class takes METAR or TAF messages in raw form and converts them to human-readable, translated text.
 * This class is not meant to be used directly. Instead, use the classes Weather::METAR or Weather::TAF.
 */

class Decoder : public QObject, private metaf::Visitor<QString> {
    Q_OBJECT

public:
    /*! \brief Description of the current weather
     *
     * For METAR messages, this property holds a description of the current weather
     * in translated, human-readable form, such as "low drifting snow" or "light rain".
     * The property can contain an empty string if there is nothing to report.
     */
    Q_PROPERTY(QString currentWeather READ currentWeather NOTIFY rawTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property currentWeather
     */
    [[nodiscard]] auto currentWeather() const -> QString
    {
        return _currentWeather;
    }

    /*! \brief Decoded text of the METAR/TAF message
     *
     * This property holds the decoded text of the message, as a human-readable,
     * rich text string.  The text might change in responde to changes in
     * user settings, and might also change by midnight (the text uses words such
     * as 'tomorrow' whose meaning changes at the end of the day).
     */
    Q_PROPERTY(QString decodedText READ decodedText NOTIFY decodedTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property decodedText
     */
    [[nodiscard]] auto decodedText() const -> QString
    {
        return _decodedText;
    }

    /*! \brief Message Type
     *
     * This is a string of the form "METAR", "TAF" or "METAR/SPECI".
     */
    Q_PROPERTY(QString messageType READ messageType NOTIFY rawTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property currentWeather
     */
    [[nodiscard]] auto messageType() const -> QString;

    /*! \brief Raw text of the METAR/TAF message */
    Q_PROPERTY(QString rawText READ rawText NOTIFY rawTextChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property rawText
     */
    [[nodiscard]] auto rawText() const -> QString
    {
        return _rawText;
    }

signals:
    /*! \brief  Notifier signal */
    void decodedTextChanged();

    /*! \brief  Notifier signal */
    void rawTextChanged();

protected:
    // This constructor creates a Decoder instance.  You need to set the raw text before this class can be useful.
    explicit Decoder(QObject *parent = nullptr);

    // Sets the raw METAR/TAF message and starts processing. Since METAR/TAF messages specify points in time only by "day of month" and "time",
    // the decoder needs to know the month and year. Set this reference date to any date between in the interval [issue date, issue date + 28 days]
    void setRawText(const QString& rawText, QDate referenceDate);

    // Indicates if the parser was able to read the text without error. If an error occurs, the decoded will
    // still be available, but is probably incomplete
    [[nodiscard]] auto hasParseError() const -> bool
    {
        return (parseResult.reportMetadata.error != metaf::ReportError::NONE);
    }

private slots:
    // This slot does the actual parsing
    void parse();

private:
    // Explanation functions
    static auto explainCloudType(const metaf::CloudType &ct) -> QString;
    static auto explainDirection(metaf::Direction direction, bool trueCardinalDirections=true) -> QString;
    static auto explainDirectionSector(const std::vector<metaf::Direction>& dir) -> QString;
    static auto explainDistance(metaf::Distance distance) -> QString;
    static auto explainDistance_FT(metaf::Distance distance) -> QString;
    auto explainMetafTime(metaf::MetafTime metafTime) -> QString;
    static auto explainPrecipitation(metaf::Precipitation precipitation) -> QString;
    static auto explainPressure(metaf::Pressure pressure) -> QString;
    static auto explainRunway(metaf::Runway runway) -> QString;
    static auto explainSpeed(metaf::Speed speed) -> QString;
    static auto explainSurfaceFriction(metaf::SurfaceFriction surfaceFriction) -> QString;
    static auto explainTemperature(metaf::Temperature temperature) -> QString;
    static auto explainWaveHeight(metaf::WaveHeight waveHeight) -> QString;
    auto explainWeatherPhenomena(const metaf::WeatherPhenomena & wp) -> QString;

    // … toString Methods
    static auto brakingActionToString(metaf::SurfaceFriction::BrakingAction brakingAction) -> QString;
    static auto cardinalDirectionToString(metaf::Direction::Cardinal cardinal) -> QString;
    static auto cloudAmountToString(metaf::CloudGroup::Amount amount) -> QString;
    static auto cloudHighLayerToString(metaf::LowMidHighCloudGroup::HighLayer highLayer) -> QString;
    static auto cloudLowLayerToString(metaf::LowMidHighCloudGroup::LowLayer lowLayer) -> QString;
    static auto cloudMidLayerToString(metaf::LowMidHighCloudGroup::MidLayer midLayer) -> QString;
    static auto cloudTypeToString(metaf::CloudType::Type type) -> QString;
    static auto convectiveTypeToString(metaf::CloudGroup::ConvectiveType type) -> QString;
    static auto distanceMilesFractionToString(metaf::Distance::MilesFraction f) -> QString;
    static auto distanceUnitToString(metaf::Distance::Unit unit) -> QString;
    static auto layerForecastGroupTypeToString(metaf::LayerForecastGroup::Type type) -> QString;
    static auto pressureTendencyTrendToString(metaf::PressureTendencyGroup::Trend trend) -> QString;
    static auto pressureTendencyTypeToString(metaf::PressureTendencyGroup::Type type) -> QString;
    static auto probabilityToString(metaf::TrendGroup::Probability prob) -> QString;
    static auto runwayStateDepositsToString(metaf::RunwayStateGroup::Deposits deposits) -> QString;
    static auto runwayStateExtentToString(metaf::RunwayStateGroup::Extent extent) -> QString;
    static auto specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp) -> QString;
    static auto stateOfSeaSurfaceToString(metaf::WaveHeight::StateOfSurface stateOfSurface) -> QString;
    static auto visTrendToString(metaf::VisibilityGroup::Trend trend) -> QString;
    static auto weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor) -> QString;
    static auto weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier) -> QString;
    static auto weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather) -> QString;

    // visitor Methods
    auto visitCloudGroup(const CloudGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitCloudTypesGroup(const CloudTypesGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitKeywordGroup(const KeywordGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitLayerForecastGroup(const LayerForecastGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitLightningGroup(const LightningGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitLocationGroup(const LocationGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitLowMidHighCloudGroup(const LowMidHighCloudGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitMinMaxTemperatureGroup(const MinMaxTemperatureGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitMiscGroup(const MiscGroup & group,  ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitPressureGroup(const PressureGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitPressureTendencyGroup(const PressureTendencyGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitReportTimeGroup(const ReportTimeGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitPrecipitationGroup(const PrecipitationGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitRunwayStateGroup(const RunwayStateGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitSeaSurfaceGroup(const SeaSurfaceGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitTemperatureGroup(const TemperatureGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitTrendGroup(const TrendGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitUnknownGroup(const UnknownGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitVicinityGroup(const VicinityGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitVisibilityGroup(const VisibilityGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitWeatherGroup(const WeatherGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;
    auto visitWindGroup(const WindGroup & group, ReportPart reportPart, const std::string & rawString) -> QString override;


    // Cached data

    // Decoded text generated by last run of parser
    QString _decodedText;

    // Raw text, as set with setRawText(…)
    QString _rawText;

    // Current weather, as read from METAR
    QString _currentWeather;

    // Reference date, as set with setRawText(…)
    QDate _referenceDate;

    // Result of the parser
    ParseResult parseResult;
};

}
