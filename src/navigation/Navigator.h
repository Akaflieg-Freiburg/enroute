/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
#include "GlobalObject.h"
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

class Navigator : public GlobalObject
{
    Q_OBJECT

public:

    /*! \brief FlightStatus */
    enum FlightStatus
      {
        Ground, /*!< Device is on the ground */
        Flight, /*!< Device is flying */
        Unknown /*!< Unknown */
      };
    Q_ENUM(FlightStatus)

    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Navigator(QObject *parent = nullptr);

    /*! \brief Standard destructor */
    ~Navigator() = default;

    // deferred initialization
    void deferredInitialization();


    //
    // PROPERTIES
    //

    /*! \brief Current aircraft
     *
     *  The aircraft returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::Aircraft aircraft READ aircraft WRITE setAircraft NOTIFY aircraftChanged)

    /*! \brief Global clock
     *
     *  The clock returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::Clock* clock READ clock CONSTANT)

    /*! \brief Current flight route
     *
     *  This flight route returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(FlightRoute* flightRoute READ flightRoute CONSTANT)

    /*! \brief Estimate whether the device is flying or on the ground
     *
     *  This property holds an estimate, as to whether the device is flying or
     *  on the ground.
     */
    Q_PROPERTY(FlightStatus flightStatus READ flightStatus NOTIFY flightStatusChanged)

    /*! \brief Current wind */
    Q_PROPERTY(Weather::Wind wind READ wind WRITE setWind NOTIFY windChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aircraft
     */
    Navigation::Aircraft aircraft() const { return m_aircraft; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property clock
     */
    Navigation::Clock* clock();

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property flightRoute
     */
    FlightRoute* flightRoute();

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property flightStatus
     */
    FlightStatus flightStatus() const { return m_flightStatus; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property wind
     */
    Weather::Wind wind() const { return m_wind; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     *  @param newAircraft Property aircraft
     */
    void setAircraft(const Navigation::Aircraft& newAircraft);

    /*! \brief Setter function for property of the same name
     *
     *  @param newWind Property wind
     */
    void setWind(const Weather::Wind& newWind);

signals:
    /*! \brief Notifier signal */
    void aircraftChanged();

    /*! \brief Emitted when the airspaceAltitudeLimit is adjusted
     *
     *  To ensure that all relevant airspaces are visible on the moving map,
     *  the navigator class will automatically adjust the property airspaceAltitudeLimit
     *  of the global settings object whenever (ownship true altitude + 1000ft)
     *  is higher than the present airspaceAltitudeLimit.
     *
     *  This signal is emitted whenever that happens.
     */
    void airspaceAltitudeLimitAdjusted();

    /*! \brief Notifier signal */
    void flightStatusChanged();

    /*! \brief Notifier signal */
    void windChanged();

private slots:
    // Check if altitude limit for flight maps needs to be lifted. Connected to positioning source.
    void updateAltitudeLimit(const Positioning::PositionInfo& info);

    // Update flight status. Connected to positioning source.
    void updateFlightStatus(const Positioning::PositionInfo& info);


private:
    Q_DISABLE_COPY_MOVE(Navigator)

    // Updater function for the property with the same name
    void setFlightStatus(FlightStatus newFlightStatus);

    // Hysteresis for flight speed
    static constexpr auto flightSpeedHysteresis = Units::Speed::fromKN(5.0);

    FlightStatus m_flightStatus {Unknown};

    Aircraft m_aircraft {};
    QPointer<Clock> m_clock {nullptr};
    QPointer<FlightRoute> m_flightRoute {nullptr};
    Weather::Wind m_wind {};

    QString m_aircraftFileName;
};

}
