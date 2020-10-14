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
#include <QXmlStreamAttribute>

#include <QDebug>
#include "../3rdParty/metaf/include/metaf.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <cmath>

#include "Clock.h"
#include "Meteorologist_METAR.h"
#include "Meteorologist_WeatherStation.h"


Meteorologist::METAR::METAR(QObject *parent)
    : QObject(parent)
{

}


Meteorologist::METAR::METAR(QXmlStreamReader &xml, Clock *clock, QObject *parent)
    : QObject(parent),
      _clock(clock)
{

    // Lambda to read sky condition
    auto readSky = [&] {
        QXmlStreamAttributes atrs = xml.attributes();
        QString sky;
        sky += atrs.value("sky_cover").toString();
        if (atrs.value("cloud_base_ft_agl").toString() != "")
            sky += "," + atrs.value("cloud_base_ft_agl").toString();
        if (atrs.value("cloud_type").toString() != "")
            sky += "," + atrs.value("cloud_type").toString();
        return sky;
    };

    // Read METAR/TAF and store in map
    QList<QString> accepted; // fields to decode without special treatment
    accepted = {"station_id", "observation_time", "temp_c",
                "dewpoint_c", "wind_dir_degrees", "wind_speed_kt",
                "wind_gust_kt", "visibility_statute_mi",
                "flight_category", "wx_string"};

    while (true) {
        xml.readNextStartElement();
        QString name = xml.name().toString();

        // Read Station_ID
        if (xml.isStartElement() && name == "station_id") {
            _ICAOCode = xml.readElementText();
            continue;
        }

        // Read location
        if (xml.isStartElement() && name == "latitude") {
            _location.setLatitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "longitude") {
            _location.setLongitude(xml.readElementText().toDouble());
            continue;
        }
        if (xml.isStartElement() && name == "elevation_m") {
            _location.setAltitude(xml.readElementText().toDouble());
            continue;
        }

        // Read raw text
        if (xml.isStartElement() && name == "raw_text") {
            _raw_text = xml.readElementText();
            data.insert("raw_text", _raw_text);
            continue;
        }

        // QNH
        if (xml.isStartElement() && name == "altim_in_hg") {
            auto content = xml.readElementText();
            data.insert("altim_in_hg", content);
            _qnh = qRound(content.toDouble() * 33.86);
            if ((_qnh < 800) || (_qnh > 1200))
                _qnh = 0;
            continue;
        }

        // Observation Time
        if (xml.isStartElement() && name == "observation_time") {
            auto content = xml.readElementText();
            data.insert("observation_time", content);
            _observationTime = QDateTime::fromString(content, Qt::ISODate);
            continue;
        }

        // Flight category
        if (xml.isStartElement() && name == "flight_category") {
            auto content = xml.readElementText();
            if (content == "VFR")
                _flightCategory = VFR;
            if (content == "MVFR")
                _flightCategory = MVFR;
            if (content == "IFR")
                _flightCategory = IFR;
            if (content == "LIFR")
                _flightCategory = LIFR;
            continue;
        }

        // Other data fields
        if (xml.isStartElement() && accepted.contains(name) ) {
            data.insert(name, xml.readElementText());
            continue;
        }

        if (xml.isStartElement() && name == "sky_condition") {
            data.insert("sky_condition", readSky());
            xml.skipCurrentElement();
            continue;
        }

        if (xml.isEndElement() && name == "METAR")
            break;

        xml.skipCurrentElement();
    }

    //
    // Generate DATA
    //
    if (data.contains("raw_text"))
        dataStrings.push_back("RAW " + data.value("raw_text").toString());
    if (data.contains("observation_time"))
        dataStrings.push_back("TIME" + Meteorologist::WeatherStation::decodeTime(data.value("observation_time")));
    if (data.contains("wind_dir_degrees") && data.contains("wind_speed_kt")) {
        if (data.contains("wind_gust_kt"))
            dataStrings.push_back("WIND" + Meteorologist::WeatherStation::decodeWind(data.value("wind_dir_degrees"), data.value("wind_speed_kt"), data.value("wind_gust_kt")));
        else
            dataStrings.push_back("WIND" + Meteorologist::WeatherStation::decodeWind(data.value("wind_dir_degrees"), data.value("wind_speed_kt")));
    }
    if (data.contains("visibility_statute_mi"))
        dataStrings.push_back("VIS " + Meteorologist::WeatherStation::decodeVis(data.value("visibility_statute_mi")));
    if (data.contains("wx_string"))
        dataStrings.push_back("WX  " + Meteorologist::WeatherStation::decodeWx(data.value("wx_string")));
    if (data.contains("sky_condition"))
        dataStrings.push_back("CLDS" + Meteorologist::WeatherStation::decodeClouds(data.values("sky_condition")));
    if (data.contains("temp_c"))
        dataStrings.push_back("TEMP" + Meteorologist::WeatherStation::decodeTemp(data.value("temp_c")));
    if (data.contains("dewpoint_c"))
        dataStrings.push_back("DEWP" + Meteorologist::WeatherStation::decodeTemp(data.value("dewpoint_c")));
    if (data.contains("altim_in_hg"))
        dataStrings.push_back("QNH " + Meteorologist::WeatherStation::decodeQnh(data.value("altim_in_hg")));

    //
    // Set up self-destruction timer
    //

    if (!_observationTime.isValid())
        deleteLater();
    else {
        // Time out is 1.5 hours after observation time, unless raw text contains "NOSIG", then it is 3 hours
        auto timeOut = QDateTime::currentDateTimeUtc().msecsTo(_observationTime.addSecs(1.5*60*60));
        if (_raw_text.contains("NOSIG"))
            timeOut = QDateTime::currentDateTimeUtc().msecsTo(_observationTime.addSecs(3*60*60));
        if (timeOut < 1000)
            deleteLater();
        else
            QTimer::singleShot(timeOut, this, SLOT(deleteLater()));

    }

    if (isValid() && !_clock.isNull()) {
        connect(_clock, &Clock::timeChanged, this, &Meteorologist::METAR::summaryChanged);
        connect(_clock, &Clock::timeChanged, this, &Meteorologist::METAR::relativeObservationTimeChanged);
    }

#warning experimental
    process();
}


Meteorologist::METAR::METAR(QDataStream &inputStream, Clock *clock, QObject *parent)
    : QObject(parent),
      _clock(clock)
{
    inputStream >> _flightCategory;
    inputStream >> _ICAOCode;
    inputStream >> _location;
    inputStream >> _observationTime;
    inputStream >> _qnh;
    inputStream >> _raw_text;

#warning experimental
    process();
}


QDateTime Meteorologist::METAR::expiration() const
{
    if (_raw_text.contains("NOSIG"))
        return _observationTime.addSecs(3*60*60);
    return _observationTime.addSecs(1.5*60*60);
}


bool Meteorologist::METAR::isExpired() const
{
    auto exp = expiration();
    if (!exp.isValid())
        return false;
    return QDateTime::currentDateTime() > exp;
}


QString Meteorologist::METAR::decodedText() const
{
    return _decoded;
}


QString Meteorologist::METAR::flightCategoryColor() const
{
    if (_flightCategory == VFR)
        return "green";
    if (_flightCategory == MVFR)
        return "yellow";
    if ((_flightCategory == IFR) || (_flightCategory == LIFR))
        return "red";
    return "transparent";
}


QString Meteorologist::METAR::summary() const {

    QStringList resultList;

    // Get flight category
    QString sky;
    if (data.contains("sky_condition")) {
        auto val = data.value("sky_condition").toString();
        if (val.contains("CAVOK"))
            sky = "CAVOK";
    }

    switch (_flightCategory) {
    case VFR:
        resultList << tr("VMC");
        if (!sky.isEmpty())
            resultList << tr("CAVOK");
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


    int windSpeed = 0;
    int gustSpeed = 0;
    if (data.contains("wind_speed_kt"))
        windSpeed = data.value("wind_speed_kt").toString().toInt();
    if (data.contains("wind_gust_kt"))
        gustSpeed = data.value("wind_gust_kt").toString().toInt();

#warning Look at units!
    if (gustSpeed > 15)
        resultList << tr("gusts of %1 kt").arg(gustSpeed);
    else if (windSpeed > 10)
        resultList << tr("wind at %1 kt").arg(windSpeed);

    if (data.contains("wx_string"))
        resultList << _weather;

    if (resultList.isEmpty())
        return QString();

    return tr("METAR %1: %2").arg(Clock::describeTimeDifference(_observationTime), resultList.join(" • "));
}


QString Meteorologist::METAR::relativeObservationTime() const
{
    if (_clock.isNull())
        return QString();
    if (!_observationTime.isValid())
        return QString();

    return Clock::describeTimeDifference(_observationTime);
}


bool Meteorologist::METAR::isValid() const
{
    if (!_location.isValid())
        return false;
    if (!_observationTime.isValid())
        return false;
    if (_ICAOCode.isEmpty())
        return false;

#warning more thorough checks needed
    return true;
}


QString Meteorologist::METAR::messageType() const
{
#warning WRONG
    return "METAR";
}


QDataStream &operator<<(QDataStream &out, const Meteorologist::METAR &metar)
{
    out << metar._flightCategory;
    out << metar._ICAOCode;
    out << metar._location;
    out << metar._observationTime;
    out << metar._qnh;
    out << metar._raw_text;

    return out;
}


using namespace metaf;

static const inline std::string groupNotValidMessage =
        "Data in this group may be errorneous, incomplete or inconsistent";


QString explainMetafTime(const metaf::MetafTime & metafTime)
{
#warning Many things need doing here!

    QString result;

    const auto day = metafTime.day();
    if (day.has_value()) {
        result += QString("day %1, ").arg(*day);
    }

    return result += QString("%1:%2").arg(metafTime.hour()).arg(metafTime.minute());
}

QString explainWeatherPhenomena(const metaf::WeatherPhenomena & wp)
{
    /* Handle special cases */
    const auto weatherStr = Meteorologist::Decoder::specialWeatherPhenomenaToString(wp);
    if (!weatherStr.isEmpty())
        return weatherStr;


    // String that will hold the result
    QString result;

    QStringList weatherPhenomena;
    for (const auto w : wp.weather())
        // This is a string such as "hail" or "rain"
        weatherPhenomena << Meteorologist::Decoder::weatherPhenomenaWeatherToString(w);
    result += weatherPhenomena.join(", ");

    QStringList parenthesisTexts;

    // Qualifier, such as "light" or "moderate"
    const auto qualifier = Meteorologist::Decoder::weatherPhenomenaQualifierToString(wp.qualifier());
    if (!qualifier.isEmpty())
        parenthesisTexts << qualifier;
    // Descriptor, such as "freezing" or "blowing"
    const auto descriptor = Meteorologist::Decoder::weatherPhenomenaDescriptorToString(wp.descriptor());
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
        result += " began: " + explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::ENDING:
        if (!time.has_value())
            break;
        result += " ended: " + explainMetafTime(*wp.time());
        break;

    case metaf::WeatherPhenomena::Event::NONE:
        break;
    }

    return result;
}


class MyVisitor : public Visitor<std::string> {
    virtual std::string visitKeywordGroup(
            const KeywordGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Keyword: " + rawString);
    }

    virtual std::string visitLocationGroup(
            const LocationGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("ICAO location: " + rawString);
    }

    virtual std::string visitReportTimeGroup(
            const ReportTimeGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Report Release Time: " + rawString);
    }

    virtual std::string visitTrendGroup(
            const TrendGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Trend Header: " + rawString);
    }

    virtual std::string visitWindGroup(
            const WindGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Wind: " + rawString);
    }

    virtual std::string visitVisibilityGroup(
            const VisibilityGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Visibility: " + rawString);
    }

    virtual std::string visitCloudGroup(
            const CloudGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Data: " + rawString);
    }

    virtual std::string visitWeatherGroup(
            const WeatherGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)reportPart; (void)rawString;
        std::ostringstream result;

        if (!group.isValid())
            result << groupNotValidMessage << "\n";

        // Gather string with list of phenomena
        QStringList phenomenaList;
        for (const auto p : group.weatherPhenomena())
            phenomenaList << explainWeatherPhenomena(p);
        auto phenomenaString = phenomenaList.join(" • ");


        switch (group.type()) {
        case metaf::WeatherGroup::Type::CURRENT:
            return phenomenaString.toStdString();

        case metaf::WeatherGroup::Type::RECENT:
            return "Recent weather: " + phenomenaString.toStdString();

        case metaf::WeatherGroup::Type::EVENT:
            return "Precipitation beginning/ending time: " + phenomenaString.toStdString();

        case metaf::WeatherGroup::Type::NSW:
            return "End of significant weather phenomena";

        case metaf::WeatherGroup::Type::PWINO:
            return "This automated station is equipped with present weather identifier and this sensor is not operating";

        case metaf::WeatherGroup::Type::TSNO:
            return "This automated station is equipped with lightning detector and this sensor is not operating";

        case metaf::WeatherGroup::Type::WX_MISG:
            return "Weather phenomena data is missing";

        case metaf::WeatherGroup::Type::TS_LTNG_TEMPO_UNAVBL:
            return "Thunderstorm / lightning data is missing";
        }
        return "";
    }

    virtual std::string visitTemperatureGroup(
            const TemperatureGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Temperature and Dew Point: " + rawString);
    }

    virtual std::string visitPressureGroup(
            const PressureGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure: " + rawString);
    }

    virtual std::string visitRunwayStateGroup(
            const RunwayStateGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("State of Runway:" + rawString);
    }

    virtual std::string visitSeaSurfaceGroup(
            const SeaSurfaceGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Sea Surface: " + rawString);
    }

    virtual std::string visitMinMaxTemperatureGroup(
            const MinMaxTemperatureGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Minimum/Maximum Temperature: " + rawString);
    }

    virtual std::string visitPrecipitationGroup(
            const PrecipitationGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Precipitation: " + rawString);
    }

    virtual std::string visitLayerForecastGroup(
            const LayerForecastGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Atmospheric Layer Forecast: " + rawString);
    }

    virtual std::string visitPressureTendencyGroup(
            const PressureTendencyGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure Tendency: " + rawString);
    }

    virtual std::string visitCloudTypesGroup(
            const CloudTypesGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Types: " + rawString);
    }

    virtual std::string visitLowMidHighCloudGroup(
            const LowMidHighCloudGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Low, middle, and high cloud layers: " + rawString);
    }

    virtual std::string visitLightningGroup(
            const LightningGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Lightning data: " + rawString);
    }

    virtual std::string visitVicinityGroup(
            const VicinityGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Events in vicinity: " + rawString);
    }

    virtual std::string visitMiscGroup(
            const MiscGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Miscellaneous Data: " + rawString);
    }

    virtual std::string visitUnknownGroup(
            const UnknownGroup & group,
            ReportPart reportPart,
            const std::string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Not recognised by the parser: " + rawString);
    }
};



void Meteorologist::METAR::process()
{
    auto report = rawText().toStdString();
    const auto result = metaf::Parser::parse(report);

    // Error handling
    /*
 *     if (result.reportMetadata.error == metaf::ReportError::NONE)
        qDebug() << "parsed successfully";
*(
*/
    QStringList decodedStrings;
    MyVisitor visitor;
    for (const auto &groupInfo : result.groups) {
        auto description = QString::fromStdString(visitor.visit(groupInfo));
        decodedStrings.append(description);
        if (std::holds_alternative<metaf::WeatherGroup>(groupInfo.group)) {
            _weather = description;
            qWarning() << description;
        }
    }

    _decoded = decodedStrings.join("<br>");
}
