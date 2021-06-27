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

#include "traffic/TrafficFactor_Abstract.h"


namespace Traffic {

#warning
/*! \brief Traffic factors
 *
 *  Objects of this class represent traffic factors, as detected by FLARM and
 *  similar devices.  This is an abstract base class.
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

    /*! \brief Copy data from other object */
    virtual void copy(const TrafficFactor_DistanceOnly& other)
    {
        copyAbstract(other);
        setHDist(other.hDist());
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property description
     */
    virtual QString description();


    //
    // PROPERTIES
    //

    /*! \brief Horizontal distance from own position to the traffic, at the time
     *  of report
     *
     *  If known, this property holds the horizontal distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  NaN.
     */
    Q_PROPERTY(AviationUnits::Distance hDist READ hDist WRITE setHDist NOTIFY hDistChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property hDist
     */
    AviationUnits::Distance hDist() const
    {
        return m_hDist;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newHDist Property hDist
     */
    void setHDist(AviationUnits::Distance newHDist) {
        if (m_hDist == newHDist) {
            return;
        }
        m_hDist = newHDist;
        emit hDistChanged();
    }

signals:
    /*! \brief Notifier signal */
    void hDistChanged();

private slots:
    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    virtual void setValid() override;

private:
    //
    // Property values
    //
    AviationUnits::Distance m_hDist;
};

}
