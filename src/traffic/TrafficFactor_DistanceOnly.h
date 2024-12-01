/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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
    ~TrafficFactor_DistanceOnly() override = default;

    //
    // Methods
    //

    /*! \brief Copy data from other object
     *
     *  This method copies all properties from the other object, with two notable exceptions.
     *
     *  - The property "animate" is not copied, the property "animate" of this class is not touched.
     *  - The lifeTime of this object is not changed.
     *
     *  @param other Instance whose properties are copied
     */
    void copyFrom(const TrafficFactor_DistanceOnly& other)
    {
        setCoordinate(other.coordinate());
        TrafficFactor_Abstract::copyFrom(other); // This will also call updateDescription
    }


    //
    // PROPERTIES
    //


    /*! \brief Center coordinate
     *
     *  This property contains the coordinate of the center of the cylinder
     *  where the traffic is most likely located.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property coordinate
     */
    [[nodiscard]] auto coordinate() const -> QGeoCoordinate
    {
        return m_coordinate;
    }

    /*! \brief Setter function for property with the same name
     *
     *  Setting a new position info does not update the hDist or vDist properties.
     *
     *  @param newCoordinate Property coordinate
     */
    void setCoordinate(const QGeoCoordinate& newCoordinate)
    {
        if (m_coordinate == newCoordinate) {
            return;
        }

        m_coordinate = newCoordinate;
        emit coordinateChanged();
    }


signals:
    /*! \brief Notifier signal */
    void coordinateChanged();


private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_DistanceOnly)

    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    void updateValid() override;

    //
    // Property values
    //
    QGeoCoordinate m_coordinate;
};

} // namespace Traffic
