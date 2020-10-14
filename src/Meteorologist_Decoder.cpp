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
#include "Meteorologist_Decoder.h"
#include "Meteorologist_WeatherStation.h"


Meteorologist::Decoder::Decoder(QObject *parent)
    : QObject(parent)
{

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
        return tr("not reported");

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


QString Meteorologist::Decoder::specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp)
{
    using WeatherVector = std::vector<metaf::WeatherPhenomena::Weather>;
    using SpecialWeatherPhenomena = std::tuple<
    metaf::WeatherPhenomena::Qualifier,
    metaf::WeatherPhenomena::Descriptor,
    WeatherVector,
    QString
    >;

    static const std::vector <SpecialWeatherPhenomena>
            specialWeatherPhenomena = {
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::VICINITY,
        metaf::WeatherPhenomena::Descriptor::SHOWERS,
        {},
        tr("precipitation in vicinity")),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::NONE,
        { metaf::WeatherPhenomena::Weather::ICE_CRYSTALS },
        tr("ice crystals")),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::NONE,
        { metaf::WeatherPhenomena::Weather::DUST },
        tr("widespread dust")),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::NONE,
        { metaf::WeatherPhenomena::Weather::UNDETERMINED },
        tr("undetermined precipitation")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::SHALLOW,
        { metaf::WeatherPhenomena::Weather::FOG },
        tr("shallow fog")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::PARTIAL,
        { metaf::WeatherPhenomena::Weather::FOG },
        tr("partial fog covering")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::PATCHES,
        { metaf::WeatherPhenomena::Weather::FOG },
        tr("patches of fog")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::NONE,
        metaf::WeatherPhenomena::Descriptor::FREEZING,
        { metaf::WeatherPhenomena::Weather::FOG },
        tr("freezing fog")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::HEAVY,
        metaf::WeatherPhenomena::Descriptor::NONE,
        { metaf::WeatherPhenomena::Weather::FUNNEL_CLOUD },
        tr("tornado or waterspout")
        ),
        SpecialWeatherPhenomena(
        metaf::WeatherPhenomena::Qualifier::VICINITY,
        metaf::WeatherPhenomena::Descriptor::NONE,
        { metaf::WeatherPhenomena::Weather::FUNNEL_CLOUD },
        tr("tornado in vicinity")
        )
    };

    for (const auto & w : specialWeatherPhenomena) {
        if (wp.qualifier() ==
                std::get<metaf::WeatherPhenomena::Qualifier>(w) &&
                wp.descriptor() ==
                std::get<metaf::WeatherPhenomena::Descriptor>(w) &&
                wp.weather() == std::get<WeatherVector>(w))
        {
            return std::get<QString>(w);
        }
    }
    return QString();
}
