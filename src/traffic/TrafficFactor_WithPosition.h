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

#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor_Abstract.h"
#include "traffic/TrafficFactorData.h"


namespace Traffic {


/*! \brief Traffic factor whose precise position is known
 *
 *  Objects of this class represent traffic factors whose precise position is
 *  known. Other properties of the traffic, such as heading and ground speed,
 *  might also be known.  Compared to TrafficFactor_Abstract, instances of this
 *  class hold an important additional property, namely the positionInfo for the
 *  traffic.
 */

class TrafficFactor_WithPosition : public TrafficFactor_Abstract {
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficFactor_WithPosition(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficFactor_WithPosition() override;

    //
    // Methods
    //

    /*! \brief Offer a data record to this object, for the same traffic factor
     *
     *  This method checks whether \a data describes the *same* traffic factor
     *  as *this, by comparing identifiers. The intended use is that a caller
     *  offers a freshly received record to each of its traffic objects in turn
     *  until one accepts it.
     *
     *  - If \a data refers to a *different* factor, it is declined and *this is
     *    left unchanged.
     *  - If \a data refers to the *same* factor, it is accepted. The record is
     *    then adopted only if it is newer than the data already held here:
     *    unless the timestamp of the positionInfo of \a data is strictly newer
     *    than the timestamp of the positionInfo of *this, the record is
     *    considered stale or out-of-order and *this is left unchanged (its
     *    lifetime is not restarted either). A stale record is still *accepted*
     *    — it belongs to this object, there is simply nothing newer to apply.
     *
     *  When the record is adopted, the positionInfo is taken over and the
     *  remaining properties are updated through
     *  TrafficFactor_Abstract::updateFrom(); see there for how the "animate"
     *  property, callsign and type are handled.
     *
     *  @param data Data record offered to *this
     *
     *  @returns True if \a data refers to the same factor and was accepted
     *  here, false if it refers to a different factor and was declined
     */
    [[nodiscard]] bool updateFrom(const TrafficFactorData_WithPosition& data)
    {
        // Decline records that belong to a different factor.
        if (!isSameFactorAs(data.data))
        {
            return false;
        }

        // Same factor: adopt the record if it is newer than what we already
        // hold.
        if (positionInfo().timestamp() < data.positionInfo.timestamp())
        {
            const QScopedPropertyUpdateGroup updateGroup;
            setPositionInfo(data.positionInfo);
            TrafficFactor_Abstract::updateFrom(data.data);
        }
        return true;
    }

    /*! \brief Replace this object by a different traffic factor
     *
     *  This method is for the case where *this is repurposed to represent a
     *  *different* traffic factor, namely the one described by \a data. It
     *  takes over the positionInfo of \a data and replaces the remaining
     *  properties through TrafficFactor_Abstract::replaceBy().
     *
     *  @param data Data record whose contents replace the data of *this
     */
    void replaceBy(const TrafficFactorData_WithPosition& data)
    {
        const QScopedPropertyUpdateGroup updateGroup;
        // Order matters here. The deferred change notifications fire in the order
        // the properties are written, and the GUI gates its position/heading
        // animations on "animate". Update the scalar fields first, because
        // TrafficFactor_Abstract::replaceBy() sets animate=false: that disables
        // the animation gate before the positionInfo change (which alone drives
        // the extrapolated coordinate) is processed. Writing positionInfo first
        // would let QML animate the coordinate jump while "animate" is still
        // stale (true), gliding the icon across the map instead of snapping.
        TrafficFactor_Abstract::replaceBy(data.data);
        setPositionInfo(data.positionInfo);
    }


    //
    // PROPERTIES
    //

    /*! \brief Extrapolated Coordinate
     *
     *  Extrapolated coordinate of the traffic, for use in the GUI. For
     *  performance reasons, instances of this class do not have their own
     *  timer/animation logic to update the property. Instead, the property is
     *  set whenever the positionInfo changes and whenever the slot
     *  updatedExtrapolatedData() is called.  The owner of this class (=
     *  TrafficDataProvider) is responsible to call updatedExtrapolatedData() at
     *  regular intervals.
     */
    Q_PROPERTY(QGeoCoordinate extrapolatedCoordinate READ extrapolatedCoordinate BINDABLE bindableExtrapolatedCoordinate)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property extrapolatedCoordinate
     */
    [[nodiscard]] QGeoCoordinate extrapolatedCoordinate() const {return m_extrapolatedCoordinate.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property extrapolatedCoordinate
     */
    [[nodiscard]] QBindable<QGeoCoordinate> bindableExtrapolatedCoordinate() const {return &m_extrapolatedCoordinate;}

    /*! \brief Extrapolated True Track
     *
     *  Extrapolated true track of the traffic, for use in the GUI. For
     *  performance reasons, instances of this class do not have their own
     *  timer/animation logic to update the property. Instead, the property is
     *  set whenever the positionInfo changes and whenever the slot
     *  updatedExtrapolatedData() is called.  The owner of this class (=
     *  TrafficDataProvider) is responsible to call updatedExtrapolatedData() at
     *  regular intervals.
     */
    Q_PROPERTY(Units::Angle extrapolatedTrueTrack READ extrapolatedTrueTrack BINDABLE bindableExtrapolatedTrueTrack)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property extrapolatedTrueTrack
     */
    [[nodiscard]] Units::Angle extrapolatedTrueTrack() const {return m_extrapolatedTrueTrack.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property extrapolatedTrueTrack
     */
    [[nodiscard]] QBindable<Units::Angle> bindableExtrapolatedTrueTrack() const {return &m_extrapolatedTrueTrack;}

    /*! \brief Suggested icon
     *
     *  Depending on alarm level, type and movement of the traffic opponent,
     *  this property suggests an icon for GUI representation of the traffic.
     */
    Q_PROPERTY(QString icon READ icon BINDABLE bindableIcon)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property icon
     */
    [[nodiscard]] QString icon() const {return m_icon.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property icon
     */
    [[nodiscard]] QBindable<QString> bindableIcon() const {return &m_icon;}

    /*! \brief PositionInfo of the traffic */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo WRITE setPositionInfo NOTIFY positionInfoChanged BINDABLE bindablePositionInfo)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] Positioning::PositionInfo positionInfo() const {return m_positionInfo.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] QBindable<Positioning::PositionInfo> bindablePositionInfo() const {return &m_positionInfo;}

    /*! \brief Setter function for property with the same name
     *
     *  @note Setting a new position info does not update the hDist or vDist
     *  properties.
     *
     *  @param newPositionInfo Property positionInfo
     */
    void setPositionInfo(const Positioning::PositionInfo& newPositionInfo) {m_positionInfo = newPositionInfo;}

    /*! \brief Uncertainty radius
     *
     *  If the last position update is less than 30s ago, this property holds
     *  the value zero. If the last position update is more than 30s ago, this
     *  property holds the radius of the "uncertainty circle" that should be
     *  drawn around the extrapolated position, to notify the user that the
     *  position data is stale. The radius is also zero when the object is
     *  invalid or has no finite groundSpeed/trueTrack.
     */
    Q_PROPERTY(Units::Distance uncertaintyRadius READ uncertaintyRadius BINDABLE bindableUncertaintyRadius)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property uncertaintyRadius
     */
    [[nodiscard]] Units::Distance uncertaintyRadius() const {return m_uncertaintyRadius.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property uncertaintyRadius
     */
    [[nodiscard]] QBindable<Units::Distance> bindableUncertaintyRadius() const {return &m_uncertaintyRadius;}

signals:
    /*! \brief Notifier signal */
    void positionInfoChanged();

public slots:
    /*! \brief Update extrapolated data
     *
     *  This method extrapolates the position of the aircraft and sets the
     *  property extrapolatedCoordinate appropriately.
     */
    void updateExtrapolatedData();

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_WithPosition)

    //
    // Property values
    //
    QProperty<QGeoCoordinate> m_extrapolatedCoordinate;
    QProperty<Units::Angle> m_extrapolatedTrueTrack;
    QProperty<Units::Distance> m_uncertaintyRadius;
    QProperty<QString> m_icon;
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_WithPosition, Positioning::PositionInfo, m_positionInfo, &Traffic::TrafficFactor_WithPosition::positionInfoChanged);
};

} // namespace Traffic
