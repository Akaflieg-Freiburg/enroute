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

/*! \brief Traffic factor whose distance is known, but not its bearing
 *
 *  Objects of this class represent traffic factors for which only the distance
 *  to the traffic is known, not its bearing. This is typically the case for
 *  aircraft seen through a Mode-S transponder, which reports altitude and range
 *  but no position.
 *
 *  Geometrically, such a factor is described by a horizontal **range ring**:
 *  the traffic is somewhere on a circle of radius range(), centered at
 *  coordinate() — the ownship position at the time of report — and offset
 *  vertically by vDist(). Compared to TrafficFactor_Abstract, this class
 *  therefore adds the property coordinate (the center of the ring); the radius
 *  is exposed as range().
 *
 *  Note that a finite range() is required for the factor to be valid (see the
 *  binding of the "valid" property), so a distance-only factor can only be
 *  shown when both the range and the ownship position are known.
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

    /*! \brief Offer a data record to this object, for the same traffic factor
     *
     *  This method checks whether \a data describes the *same* traffic factor
     *  as *this, by comparing identifiers. The intended use is that the caller
     *  offers a freshly received record and falls back to replaceBy() if it is
     *  declined.
     *
     *  - If \a data refers to a *different* factor, it is declined and *this is
     *    left unchanged.
     *  - If \a data refers to the *same* factor, it is accepted: the coordinate
     *    is taken over and the remaining properties are updated through
     *    TrafficFactor_Abstract::updateFrom(), so that the transition is
     *    animated (e.g. the range ring grows or shrinks smoothly as the target
     *    moves).
     *
     *  @param data Data record offered to *this
     *
     *  @returns True if \a data refers to the same factor and was accepted
     *  here, false if it refers to a different factor and was declined
     */
    [[nodiscard]] bool updateFrom(const TrafficFactorData_DistanceOnly& data)
    {
        // Decline records that belong to a different factor.
        if (!isSameFactorAs(data.data))
        {
            return false;
        }

        const QScopedPropertyUpdateGroup updateGroup;
        setCoordinate(data.coordinate);
        TrafficFactor_Abstract::updateFrom(data.data);
        return true;
    }

    /*! \brief Replace this object by a different traffic factor
     *
     *  This method is for the case where *this is repurposed to represent a
     *  *different* position-less factor, namely the one described by \a data —
     *  typically after updateFrom() has declined the record. The coordinate is
     *  taken over and the remaining properties are replaced through
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

    /*! \brief Center coordinate of the range ring
     *
     *  This property contains the center of the range ring, that is, the
     *  ownship position at the time of report. The traffic is located somewhere
     *  on the circle of radius range() around this coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate BINDABLE bindableCoordinate)

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

    /*! \brief Range to the traffic — the radius of the range ring
     *
     *  This property holds the horizontal distance from the ownship to the
     *  traffic, i.e. the radius of the range ring described in the class
     *  documentation. It is the defining datum of a distance-only factor: it is
     *  reported by the traffic receiver and set, together with the other data,
     *  through replaceBy() (which forwards to
     *  TrafficFactor_Abstract::replaceBy() and thus to setHDist()). A finite
     *  range is required for the factor to be valid.
     *
     *  range() is the first-class, well-named view of the distance for this
     *  class. It is backed by the inherited hDist property and always holds the
     *  same value; hDist is retained because the base class
     *  TrafficFactor_Abstract uses it for the "valid" property and for priority
     *  comparisons.
     */
    Q_PROPERTY(Units::Distance range READ range BINDABLE bindableRange)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property range
     */
    [[nodiscard]] Units::Distance range() const {return hDist();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property range
     */
    [[nodiscard]] QBindable<Units::Distance> bindableRange() {return bindableHDist();}

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_DistanceOnly)

    //
    // Property values
    //
    QProperty<QGeoCoordinate> m_coordinate;
};

} // namespace Traffic
