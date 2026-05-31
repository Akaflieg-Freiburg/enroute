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


namespace Traffic {


/*! \brief Traffic factor whose precise position is known
 *
 *  Objects of this class represent traffic factors whose precise position is known.
 *  Other properties of the traffic, such as heading and ground speed, might also
 *  be known.  Compared to TrafficFactor_Abstract, instances of this class hold
 *  important additional property, namely the positionInfo for the traffic.
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

    /*! \brief Copy data from other object
     *
     *  This method copies all properties from the other object, with two notable exceptions.
     *
     *  - The property "animate" is not copied, the property "animate" of this class is not touched.
     *  - The lifeTime of this object is not changed.
     *
     *  @param other Instance whose properties are copied
     */
    void copyFrom(const TrafficFactor_WithPosition& other)
    {
        if (other.positionInfo().timestamp() < m_positionInfo.value().timestamp())
        {
            return;
        }
        setPositionInfo(other.positionInfo());
        TrafficFactor_Abstract::copyFrom(other);
    }


    //
    // PROPERTIES
    //

    /*! \brief Extrapolated Coordinate
     *
     *  Extrapolated coordinate of the traffic, for use in the GUI. For performance reasons, instances
     *  of this class do not have their own timer/animation logic to update the property. Instead, the
     *  property is set whenever the positionInfo changes and whenever the slot
     *  updatedExtrapolatedData() is called.  The owner of this class (= TrafficDataProvider) is responsible
     *  to call updatedExtrapolatedData() at regular intervals.
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
     *  Extrapolated true track of the traffic, for use in the GUI. For performance reasons, instances
     *  of this class do not have their own timer/animation logic to update the property. Instead, the
     *  property is set whenever the positionInfo changes and whenever the slot
     *  updatedExtrapolatedData() is called.  The owner of this class (= TrafficDataProvider) is responsible
     *  to call updatedExtrapolatedData() at regular intervals.
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
     *  Depending on alarm level, type and movement of the traffic opponent, this
     *  property suggests an icon for GUI representation of the traffic.
     */
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

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
     *  @note Setting a new position info does not update the hDist or vDist properties.
     *
     *  @param newPositionInfo Property positionInfo
     */
    void setPositionInfo(const Positioning::PositionInfo& newPositionInfo) {m_positionInfo = newPositionInfo;}

    /*! \brief Uncertainty radius
     *
     *  If the last position update is less than 30s ago, this property holds the value zero.
     *  If the last position update is more than 30s ago, this property holds the radius of the "uncertainty circle" that should be drawn around the extrapolated position,
     *  to notify the user that the position data is stale.
     */
    Q_PROPERTY(Units::Distance uncertaintyRadius READ uncertaintyRadius BINDABLE bindableUncertaintyRadius)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property uncertaintyRadius
     */
    [[nodiscard]] Units::Distance uncertaintyRadius() const {return m_uncertainityRadius.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property uncertaintyRadius
     */
    [[nodiscard]] QBindable<Units::Distance> bindableUncertaintyRadius() const {return &m_uncertainityRadius;}

signals:
    /*! \brief Notifier signal */
    void iconChanged();

    /*! \brief Notifier signal */
    void positionInfoChanged();

public slots:
    /*! \brief Update extrapolated data
     *
     *  This method extrapolates the position of the aircraft and sets the property extrapolatedCoordinate appropriately.
     */
    void updateExtrapolatedData();

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_WithPosition)

    //
    // Property values
    //
    QProperty<QGeoCoordinate> m_extrapolatedCoordinate;
    QProperty<Units::Angle> m_extrapolatedTrueTrack;
    QProperty<Units::Distance> m_uncertainityRadius;
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_WithPosition, QString, m_icon, &Traffic::TrafficFactor_WithPosition::iconChanged);
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_WithPosition, Positioning::PositionInfo, m_positionInfo, &Traffic::TrafficFactor_WithPosition::positionInfoChanged);
};

} // namespace Traffic
