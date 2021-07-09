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

#include <QGeoPositionInfo>
#include <QTimer>

#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor_Abstract.h"
#include "units/Distance.h"
#include "units/Speed.h"

class DemoRunner;

namespace Traffic {

#warning documentation
/*! \brief Traffic opponents
 *
 *  Objects of this class represent traffic opponents whose position is known, as detected by FLARM and
 *  similar devices.
 */

class TrafficFactor_WithPosition : public TrafficFactor_Abstract {
    Q_OBJECT

    // Only friends can set properties
    friend class ::DemoRunner;
    friend class TrafficDataProvider;
    friend class TrafficDataSource_Abstract;

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

    // Copy data from other object
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
    QString icon() const
    {
        return m_icon;
    }

    /*! \brief PositionInfo of the traffic
     *
     *  This property contains the coordinate part of the positionInfo. The
     *  property exists for better cooperation with QML
     */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo WRITE setPositionInfo NOTIFY positionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    Positioning::PositionInfo positionInfo() const
    {
        return Positioning::PositionInfo(m_positionInfo);
    }

    /*! \brief Setter function for property with the same name
     *
     *  Setting a new position info does not update the hDist or vDist properties.
     *
     *  @param newPositionInfo Property positionInfo
     */
    void setPositionInfo(const QGeoPositionInfo& positionInfo);


signals:
    /*! \brief Notifier signal */
    void iconChanged();

    /*! \brief Notifier signal */
    void positionInfoChanged();


protected:
    // See documentation in base class
    virtual void updateDescription() override;

    // Updates property icon
    void updateIcon();

private:
    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    virtual void updateValid() override;

    //
    // Property values
    //
    QString m_icon;
    QGeoPositionInfo m_positionInfo;
    AviationUnits::Distance m_vDist;
    AviationUnits::Distance m_hDist;

};

}
