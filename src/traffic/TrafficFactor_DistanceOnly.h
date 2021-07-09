/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#warning documentation
/*! \brief Traffic factor where only distance is known
 *
 *  Objects of this class represent traffic factors, where only the horizontal distance to the traffic is known.
 *  This is often the case for aircraft that report their position only through a Mode-S transponder.
 */

class TrafficFactor_DistanceOnly : public Traffic::TrafficFactor_Abstract {
    Q_OBJECT

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

    // Copy data from other object
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
    QGeoCoordinate coordinate() const
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
    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    virtual void updateValid() override;

    //
    // Property values
    //
    QGeoCoordinate m_coordinate {};
};

}
