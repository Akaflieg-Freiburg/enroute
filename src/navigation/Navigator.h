/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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

#include <QQmlEngine>
#include <QStandardPaths>

#include "FlightRoute.h"
#include "GlobalObject.h"
#include "navigation/FlightRoute.h"
#include "navigation/RemainingRouteInfo.h"

using namespace Qt::Literals::StringLiterals;

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
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief FlightStatus */
    enum FlightStatus : quint8
      {
        Ground, /*!< Device is on the ground */
        Flight, /*!< Device is flying */
        Unknown /*!< Unknown */
      };
    Q_ENUM(FlightStatus)



    //
    // Constructors and Destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Navigator(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit Navigator() = delete;

    /*! \brief Standard destructor */
    ~Navigator() override = default;

    // factory function for QML singleton
    static Navigation::Navigator* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::navigator();
    }


    //
    // PROPERTIES
    //

    /*! \brief Current aircraft
     *
     *  The aircraft returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::Aircraft aircraft READ aircraft BINDABLE bindableAircraft WRITE setAircraft NOTIFY aircraftChanged)
    [[nodiscard]] Navigation::Aircraft aircraft() const { return m_aircraft.value(); }
    [[nodiscard]] QBindable<Navigation::Aircraft> bindableAircraft() { return &m_aircraft; }
    void setAircraft(const Navigation::Aircraft& newAircraft);

    /*! \brief Indicates whether an aviation map is installed for the current location
     *
     *  For performance reasons, this method only checks whether the approximate last valid coordinate provided by PositionProvider
     *  is contained in the bounding box of one of the installed aviation maps.
     */
    Q_PROPERTY(bool hasAviationMapForCurrentLocation READ hasAviationMapForCurrentLocation BINDABLE bindableHasAviationMapForCurrentLocation)
    [[nodiscard]] bool hasAviationMapForCurrentLocation() const {return m_hasAviationMapForCurrentLocation.value();}
    [[nodiscard]] QBindable<bool> bindableHasAviationMapForCurrentLocation() {return &m_hasAviationMapForCurrentLocation;}

    /*! \brief Current flight route
     *
     *  This flight route returned here is owned by this class and must not be deleted.
     *  QML ownership has been set to QQmlEngine::CppOwnership.
     */
    Q_PROPERTY(Navigation::FlightRoute* flightRoute READ flightRoute CONSTANT)
    [[nodiscard]] Navigation::FlightRoute* flightRoute();

    /*! \brief Estimate whether the device is flying or on the ground
     *
     *  This property holds an estimate, as to whether the device is flying or
     *  on the ground.
     */
    Q_PROPERTY(FlightStatus flightStatus READ flightStatus NOTIFY flightStatusChanged)
    [[nodiscard]] FlightStatus flightStatus() const { return m_flightStatus; }

    /*! \brief Up-to-date information about the remaining route */
    Q_PROPERTY(Navigation::RemainingRouteInfo remainingRouteInfo READ remainingRouteInfo BINDABLE bindableRemainingRouteInfo)
    [[nodiscard]] Navigation::RemainingRouteInfo remainingRouteInfo() const {return m_remainingRouteInfo.value();}
    [[nodiscard]] QBindable<Navigation::RemainingRouteInfo> bindableRemainingRouteInfo() {return &m_remainingRouteInfo;}

    /*! \brief Current wind */
    Q_PROPERTY(Weather::Wind wind READ wind WRITE setWind NOTIFY windChanged)
    [[nodiscard]] Weather::Wind wind() const { return m_wind; }
    void setWind(Weather::Wind newWind);

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
    void updateAltitudeLimit();

    // Update flight status. Connected to positioning source.
    void updateFlightStatus();

    // Re-computes the Remaining Route Info. The argument must be the current position info of the own aircraft.
    void updateRemainingRouteInfo();

private:
    Q_DISABLE_COPY_MOVE(Navigator)

    // Updater function for the property with the same name
    void setFlightStatus(FlightStatus newFlightStatus);

    QProperty<bool> m_hasAviationMapForCurrentLocation {false};
    bool computeHasAviationMapForCurrentLocation();


    // Hysteresis for flight speed
    static constexpr auto flightSpeedHysteresis = Units::Speed::fromKN(5.0);

    FlightStatus m_flightStatus {Unknown};

    QProperty<Aircraft> m_aircraft {};

    QPointer<FlightRoute> m_flightRoute {nullptr};
    const QString m_flightRouteFileName {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/flight route.geojson"_s};

    Weather::Wind m_wind {};

    QString m_aircraftFileName;

    QProperty<RemainingRouteInfo> m_remainingRouteInfo;
};

} // namespace Navigation
