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

#include <QGeoPositionInfo>
#include <QPointer>

#include "traffic/TrafficDataSource_Abstract.h"


namespace Traffic {

/*! \brief Traffic receiver: Simulator that provides constant data
 *
 *  For testing purposes, this class provides constant traffic data.
 */
class TrafficDataSource_Simulate : public TrafficDataSource_Abstract {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Simulate(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Simulate() override = default;

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property icon
     */
    [[nodiscard]] QString icon() const override { return u"/icons/material/ic_file_download.svg"_qs; }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] auto sourceName() const -> QString override
    {
        return tr("Simulator data");
    }

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void connectToTrafficReceiver() override;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void disconnectFromTrafficReceiver() override;

    /*! \brief Set distance that is to be reported by this class as the barometric altitude of ownship
     *
     *  @param barAlt Barometric altitude of simulated ownship
     */
    void setBarometricHeight(Units::Distance barAlt)
    {
        barometricHeight = barAlt;
    }

    /*! \brief Set coordinate that is to be reported by this class as the position of ownship
     *
     *  @param coordinate Coordinate of simulated ownship
     */
    void setCoordinate(const QGeoCoordinate& coordinate)
    {
        geoInfo.setCoordinate(coordinate);
    }

    /*! \brief Set speed that is to be reported by this class as the ground speed of ownship
     *
     *  @param GS Ground speed of simulated ownship
     */
    void setGS(Units::Speed GS)
    {
        geoInfo.setAttribute(QGeoPositionInfo::GroundSpeed, GS.toMPS());
    }

    /*! \brief Set angle that is to be reported by this class as the true track of ownship
     *
     *  @param TT True track of simulated ownship
     */
    void setTT(Units::Angle TT)
    {
        geoInfo.setAttribute(QGeoPositionInfo::Direction, TT.toDEG());
    }

    /*! \brief Set traffic factor (distance only) that is to be reported by this class
     *
     *  @param factor Traffic factor, or nullptr to remove all distance-only
     *  traffic. The traffic factor will be owned by this class.
     */
    void setTrafficFactor_DistanceOnly(Traffic::TrafficFactor_DistanceOnly* factor=nullptr)
    {
        delete trafficFactor_DistanceOnly;
        if (factor != nullptr) {
            factor->setParent( this );
            trafficFactor_DistanceOnly = factor;
        }
    }

    /*! \brief Add a traffic factor that is to be reported by this class
     *
     *  @param factor Traffic factor to be added. The pointer must be valid. The
     *  traffic factor will be owned by this class.
     */
    void addTraffic(Traffic::TrafficFactor_WithPosition* factor)
    {
        factor->setParent(this);
        trafficFactors.append(factor);
    }

    /*! \brief Remove all traffic factors (with position) */
    void removeTraffic()
    {
        qDeleteAll(trafficFactors);
        trafficFactors.clear();
    }

private slots:
    // Send out simulated data. This slot will be called once per second once
    // connectToTrafficReceiver() has been called
    void sendSimulatorData();

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_Simulate)

    // Simulator related members
    QTimer simulatorTimer;
    QGeoPositionInfo geoInfo;
    Units::Distance barometricHeight;
    QVector<QPointer<TrafficFactor_WithPosition>> trafficFactors;
    QPointer<TrafficFactor_DistanceOnly> trafficFactor_DistanceOnly;
};

} // namespace Traffic
