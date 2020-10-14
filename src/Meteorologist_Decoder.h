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

#include "Meteorologist.h"

class QXmlStreamReader;

/*! \brief METAR report
 *
 * This class contains the data of a METAR or SPECI report and provided a few
 * methods to access the data. Instances of this class are provided by the
 * Meteorologist class; there is no way to construct valid instances yourself.
 */

class Meteorologist::Decoder : public QObject {
    Q_OBJECT

public:
    explicit Decoder(QObject *parent = nullptr);

    static QString weatherPhenomenaDescriptorToString(metaf::WeatherPhenomena::Descriptor descriptor);
    static QString weatherPhenomenaQualifierToString(metaf::WeatherPhenomena::Qualifier qualifier);
    static QString weatherPhenomenaWeatherToString(metaf::WeatherPhenomena::Weather weather);
    static QString specialWeatherPhenomenaToString(const metaf::WeatherPhenomena & wp);
};
