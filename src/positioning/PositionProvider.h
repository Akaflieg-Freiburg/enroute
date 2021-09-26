/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include "positioning/PositionInfoSource_Abstract.h"
#include "positioning/PositionInfoSource_Satellite.h"


namespace Positioning {

/*! \brief Central Position Provider
 *
 *  This class collects position data from the various sources (satellite,
 *  network, traffic receiver, …)  chooses the best available source and exposes
 *  the data to QML and other parts of the program.
 *
 *  There exists one static instance of this class, which can be accessed via
 *  the method globalInstance().  No other instance of this class should be
 *  used.
 *
 *  The methods in this class are reentrant, but not thread safe.
 */

class PositionProvider : public PositionInfoSource_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit PositionProvider(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
    ~PositionProvider() override;

    /*! \brief Last valid coordinate reading
     *
     *  This property holds the last valid coordinate known.  At the first
     *  start, this property is set to the location Freiburg Airport, EDTF.  The
     *  value is stored in a QSetting at destruction, and restored in the
     *  construction.
     */
    Q_PROPERTY(QGeoCoordinate lastValidCoordinate READ lastValidCoordinate NOTIFY lastValidCoordinateChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidCoordinate
     */
    static QGeoCoordinate lastValidCoordinate();

    /*! \brief Last valid true track
     *
     *  This property holds the last valid true track known.  At the first
     *  start, this property is set to 0°.  The value is stored in a QSetting at
     *  destruction, and restored in the construction.
     */
    Q_PROPERTY(Units::Angle lastValidTT READ lastValidTT NOTIFY lastValidTTChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidTrack
     */
    static Units::Angle lastValidTT();

signals:
    /*! \brief Notifier signal */
    void lastValidTTChanged(Units::Angle);

    /*! \brief Notifier signal */
    void lastValidCoordinateChanged(QGeoCoordinate);

private slots:   
    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of globalInstance().
    void deferredInitialization() const;

    // Connected to sources, in order to receive new data
    void onPositionUpdated();

    // Connected to sources, in order to receive new data
    void onPressureAltitudeUpdated();

    // Saves last valid position and track
    void savePositionAndTrack();

    // Setter method for property with the same name
    void setLastValidCoordinate(const QGeoCoordinate &newCoordinate);

    // Setter method for property with the same name
    void setLastValidTT(Units::Angle newTT);

    // Setter method for property with the same name
    void updateStatusString();

private:
    Q_DISABLE_COPY_MOVE(PositionProvider)

    // Aircraft is considered flying if speed is at least this high
    static constexpr double minFlightSpeedInKT = 30.0;
    // Hysteresis for flight speed
    static constexpr double flightSpeedHysteresis = 5.0;

    // Coordinates of EDTF airfield
    static constexpr double EDTF_lat = 48.022653;
    static constexpr double EDTF_lon = 7.832583;
    static constexpr double EDTF_ele = 244;

    PositionInfoSource_Satellite satelliteSource;

    QGeoCoordinate m_lastValidCoordinate {EDTF_lat, EDTF_lon, EDTF_ele};
    Units::Angle m_lastValidTT {};
};

}
