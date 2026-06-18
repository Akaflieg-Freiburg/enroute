/***************************************************************************
 *   Copyright (C) 2021-2026 by Stefan Kebekus                             *
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

#include <QGeoCoordinate>

#include "traffic/TrafficFactor_Abstract.h"
#include "traffic/TrafficFactorData.h"


namespace Traffic {

/*! \brief Traffic factor where only distance is known
 *
 *  Objects of this class represent traffic factors, where only the horizontal distance to the traffic is known.
 *  This is typically the case for aircraft that report their position only through a Mode-S transponder.
 *  Compared to TrafficFactor_Abstract, instances of this class hold one additional property, namely
 *  the ownship position at the time of report.  The traffic must then be expected within a cylinder
 *  centered in coordinate with radius hDist.
 */

class TrafficFactor_DistanceOnly : public Traffic::TrafficFactor_Abstract {
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficFactor_DistanceOnly(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficFactor_DistanceOnly() override;

    //
    // Methods
    //

    /*! \brief Replace this object by a different traffic factor
     *
     *  This single distance-only slot is reused for whichever position-less factor
     *  was reported most recently. Successive reports generally describe different
     *  factors, so each call is treated as a replacement rather than an update: the
     *  coordinate is taken over and the remaining properties are replaced through
     *  TrafficFactor_Abstract::replaceBy().
     *
     *  @param data Data record whose contents replace the data of *this
     */
    void replaceBy(const TrafficFactorData_DistanceOnly& data)
    {
        const QScopedPropertyUpdateGroup updateGroup;
        setCoordinate(data.coordinate);
        TrafficFactor_Abstract::replaceBy(data.data);
    }


    //
    // PROPERTIES
    //

    /*! \brief Center coordinate
     *
     *  This property contains the coordinate of the center of the cylinder
     *  where the traffic is most likely located.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged BINDABLE bindableCoordinate)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property coordinate
     */
    [[nodiscard]] QGeoCoordinate coordinate() const {return m_coordinate.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property coordinate
     */
    [[nodiscard]] QBindable<QGeoCoordinate> bindableCoordinate() const {return &m_coordinate;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newCoordinate Property coordinate
     */
    void setCoordinate(const QGeoCoordinate& newCoordinate) {m_coordinate = newCoordinate;}
#warning figure out where hDist is actually set!

signals:
    /*! \brief Notifier signal */
    void coordinateChanged();


private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_DistanceOnly)

    //
    // Property values
    //
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_DistanceOnly, QGeoCoordinate, m_coordinate, &Traffic::TrafficFactor_DistanceOnly::coordinateChanged);
};

} // namespace Traffic
