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

#include "FlightRoute.h"
#include "navigation/Clock.h"
#include "navigation/FlightRoute.h"
#include "positioning/PositionInfo.h"


namespace Navigation {

/*! \brief Main hub for navigation data
 *
 *  This class collects all data items that are relevant for navigation.
 *
 *  The methods in this class are reentrant, but not thread safe.
 */

class Navigator : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Navigator(QObject *parent = nullptr);

    /*! \brief Standard destructor
     *
     */
    ~Navigator();

    //
    // METHODS
    //

    /*! \brief Description of the way between two points
     *
     * @param from Starting point of the way
     *
     * @param to Endpoint of the way
     *
     * @returns A string such as "DIST 65.2 nm • QUJ 276°" or (depending on the global settings) "DIST 65.2 km • QUJ 276°".  If the way cannot be described (e.g. because one of the coordinates is invalid), then an empty string is returned.
     */
    Q_INVOKABLE static QString describeWay(const QGeoCoordinate &from, const QGeoCoordinate &to);


    //
    // PROPERTIES
    //

    /*! \brief Current aircraft
     *
     *  The aircraft returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::Aircraft* aircraft READ aircraft CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aircraft
     */
    Navigation::Aircraft* aircraft();

    /*! \brief Global clock
     *
     *  The clock returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::Clock* clock READ clock CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property clock
     */
    Navigation::Clock* clock();

    /*! \brief Current flight route
     *
     *  This flight route returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(FlightRoute* flightRoute READ flightRoute CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property flightRoute
     */
    FlightRoute* flightRoute();

    /*! \brief Estimate whether the device is flying or on the ground
     *
     *  This property holds an estimate, as to whether the device is flying or
     *  on the ground.  The current implementation considers the device is
     *  flying if the groundspeed can be read and is greater then 30 knots.
     */
    Q_PROPERTY(bool isInFlight READ isInFlight NOTIFY isInFlightChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property isInFlight
     */
    bool isInFlight() const
    {
        return m_isInFlight;
    }

    /*! \brief Current wind
     *
     *  The wind returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Weather::Wind* wind READ wind CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property wind
     */
    Weather::Wind* wind();

signals:
    /*! \brief Notifier signal */
    void isInFlightChanged();

private slots:
    // Connected to sources, in order to receive new data
    void onPositionUpdated(const Positioning::PositionInfo& info);

    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of constructors in Global.
    void deferredInitialization() const;

    // Save current aircraft to standard location
    void saveAircraft() const;

private:
    Q_DISABLE_COPY_MOVE(Navigator)

    // Setter function for the property with the same name
    void setIsInFlight(bool newIsInFlight);

    // Aircraft is considered flying if speed is at least this high
    static constexpr double minFlightSpeedInKN = 30.0;
    // Hysteresis for flight speed
    static constexpr double flightSpeedHysteresisInKn = 5.0;

    bool m_isInFlight {false};

    QPointer<Aircraft> m_aircraft {nullptr};
    QPointer<Clock> m_clock {nullptr};
    QPointer<FlightRoute> m_flightRoute {nullptr};
    QPointer<Weather::Wind> m_wind {nullptr};

    QString m_aircraftFileName;
};

}
