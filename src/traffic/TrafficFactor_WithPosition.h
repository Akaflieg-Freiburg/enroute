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

#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor_Abstract.h"


namespace Traffic {


/*! \brief Traffic factor whose precise position is known
 *
 *  Objects of this class represent traffic factors whose precise position is known.
 *  Other properties of the traffic, such as heading and ground speed, mifght also
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
    ~TrafficFactor_WithPosition() override = default;

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
        setPositionInfo(other.positionInfo());
        TrafficFactor_Abstract::copyFrom(other); // This will also call updateDescription
    }


    //
    // PROPERTIES
    //

    /*! \brief Suggested icon
     *
     *  Depending on alarm level and movement of the traffic opponent, this
     *  property suggests an icon for GUI representation of the traffic.
     */
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property icon
     */
    [[nodiscard]] auto icon() const -> QString
    {
        return m_icon;
    }

    /*! \brief PositionInfo of the traffic */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo WRITE setPositionInfo NOTIFY positionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] auto positionInfo() const -> Positioning::PositionInfo
    {
        return Positioning::PositionInfo(m_positionInfo);
    }

    /*! \brief Setter function for property with the same name
     *
     *  @note Setting a new position info does not update the hDist or vDist properties.
     *
     *  @param newPositionInfo Property positionInfo
     */
    void setPositionInfo(const Positioning::PositionInfo& newPositionInfo);


signals:
    /*! \brief Notifier signal */
    void iconChanged();

    /*! \brief Notifier signal */
    void positionInfoChanged();


protected:
    // See documentation in base class
    void updateDescription() override;

    // Updates property icon
    void updateIcon();

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_WithPosition)

    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    void updateValid() override;

    //
    // Property values
    //
    QString m_icon;
    QGeoPositionInfo m_positionInfo;
    Units::Distance m_vDist;
    Units::Distance m_hDist;

};

} // namespace Traffic
