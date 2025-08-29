/***************************************************************************
 *   Copyright (C) 2025 by Markus Marks and Stefan Kebekus                 *
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

#include <QGeoPositionInfo>
#include <QPropertyNotifier>
#include <QTime>

#include "units/Distance.h"


namespace Navigation
{

/*! \brief Cache relating geometric and barometric altitude information
 *
 * This class collects geometric and barometric altitude information of the own aircraft. If sufficient data is available, it uses this data to
 * estimate geometric altitudes from barometric altitudes and vice versa. This functionality is used, for instance, to estimate the geometric
 * altitude of airspace boundaries (which are defined as geometric altitudes).
 *
 * For clarity: We use the term "barometric altitude" to refer to the baromatric altitude over the standard level. This is the altitude shown
 * by an aircraft altimeter when set to 1013.2 hPa.
 */
class BaroCache : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    BaroCache(QObject* parent = nullptr);

    // Default destructor
    ~BaroCache() override = default;



    //
    // Methods
    //

    /*! \brief Estimate pressure altitude for a given geometric altitude
     *
     *  This method queries the cache, in order to estimate the pressure altitude for a given geometric altitude.
     *  It will return an invalid Distance unless there exists a cache entry whose geometric
     *  altitude is within +/- 500ft from the geometricAltitude parameter.
     *
     *  Remember: pressure altitude is the barometric altitude above the 1013.2 hPa pressure surface
     *
     *  @param geometricAltitude Geometric altitude whose associated pressure altitude is to be estimated
     *
     *  @returns Estimated pressure altitude
     */
    [[nodiscard]] Q_INVOKABLE Units::Distance estimatedPressureAltitude(Units::Distance geometricAltitude);

private:
    Q_DISABLE_COPY_MOVE(BaroCache)

    void addIncomingBaroCacheData();

    Units::Distance m_incomingGeometricAltitude;
    QDateTime m_incomingGeometricAltitudeTimestamp;

    Units::Distance m_incomingPressureAltitude;
    QDateTime m_incomingPressureAltitudeTimestamp;

    struct altitudeElement
    {
        QDateTime timestamp;
        Units::Distance pressureAltitude;
        Units::Distance geometricAltitude;
    };

    QMap<int, altitudeElement> m_altitudeElementsByFlightLevel;

    std::vector<QPropertyNotifier> notifiers;
};

} // namespace Navigation
