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

#include "units/Distance.h"
#include "units/Speed.h"
#include "units/Volume.h"
#include "units/VolumeFlow.h"

#include <QGeoCoordinate>
#include <QQmlEngine>

namespace Navigation {

/*! \brief This extremely simple class holds a few numbers that describe an aircraft */

class Aircraft {
    Q_GADGET
    QML_VALUE_TYPE(aircraft)

public:
    /*! \brief Units of measurement for volumes */
    enum FuelConsumptionUnit {
        /*! \brief Liter per hour */
        LiterPerHour = 0,

        /*! \brief Gallon per hour */
        GallonPerHour = 1
    };
    Q_ENUM(FuelConsumptionUnit)

    /*! \brief Units of measurement for horizontal distances */
    enum HorizontalDistanceUnit {
        /*! \brief Nautical Mile */
        NauticalMile = 0,

        /*! \brief Kilometer */
        Kilometer = 1,

        /*! \brief Statute Mile */
        StatuteMile = 2
    };
    Q_ENUM(HorizontalDistanceUnit)

    /*! \brief Units of measurement for vertical distances */
    enum VerticalDistanceUnit {
        /*! \brief Feet */
        Feet = 0,

        /*! \brief Meters */
        Meters = 1
    };
    Q_ENUM(VerticalDistanceUnit)


    //
    // Properties
    //

    /*! \brief Cruise Speed
     *
     * This property holds the cruise speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed cruiseSpeed READ cruiseSpeed WRITE setCruiseSpeed)

    /*! \brief Decent Speed
     *
     * This property holds the descent speed of the aircraft. This is a
     * number that lies in the interval [minAircraftSpeed, maxAircraftSpeed]
     * or NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed descentSpeed READ descentSpeed WRITE setDescentSpeed)

    /*! \brief Fuel Consumption
     *
     * This property holds the fuel consumption of the aircraft. This is a
     * number that lies in the interval [minFuelConsumption,
     * maxFuelConsumption] or NaN if no value has been set.
     */
    Q_PROPERTY(Units::VolumeFlow fuelConsumption READ fuelConsumption WRITE setFuelConsumption)

    /*! \brief Preferred units of measurement for fuel consumption */
    Q_PROPERTY(FuelConsumptionUnit fuelConsumptionUnit READ fuelConsumptionUnit WRITE setFuelConsumptionUnit)

    /*! \brief Preferred units of measurement for horizontal distances */
    Q_PROPERTY(HorizontalDistanceUnit horizontalDistanceUnit READ horizontalDistanceUnit WRITE setHorizontalDistanceUnit)

    /*! \brief Maximal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed maxValidSpeed MEMBER maxValidSpeed CONSTANT)

    /*! \brief Maximal fuel consumption that is considered valid */
    Q_PROPERTY(Units::VolumeFlow maxValidFuelConsumption MEMBER maxValidFuelConsumption CONSTANT)

    /*! \brief Minimum Speed
     *
     * This property holds the minimum speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the minimum speed has not been set.
     */
    Q_PROPERTY(Units::Speed minimumSpeed READ minimumSpeed WRITE setMinimumSpeed)

    /*! \brief Minimal fuel consumption that is considered valid */
    Q_PROPERTY(Units::VolumeFlow minValidFuelConsumption MEMBER minValidFuelConsumption CONSTANT)

    /*! \brief Minimal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed minValidSpeed MEMBER minValidSpeed CONSTANT)

    /*! \brief Name
     *
     * This property holds the name.
     */
    Q_PROPERTY(QString name READ name WRITE setName)

    /*! \brief Preferred units of measurement for vertical distances */
    Q_PROPERTY(VerticalDistanceUnit verticalDistanceUnit READ verticalDistanceUnit WRITE setVerticalDistanceUnit)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property cruise speed
     */
    [[nodiscard]] auto cruiseSpeed() const -> Units::Speed { return m_cruiseSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property descentSpeed
     */
    [[nodiscard]] auto descentSpeed() const -> Units::Speed { return m_descentSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property fuelConsumptionInLPH
     */
    [[nodiscard]] auto fuelConsumption() const -> Units::VolumeFlow { return m_fuelConsumption; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVolumeUnit
     */
    [[nodiscard]] auto fuelConsumptionUnit() const -> FuelConsumptionUnit { return m_fuelConsumptionUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredHorizontalDistanceUnit
     */
    [[nodiscard]] auto horizontalDistanceUnit() const -> HorizontalDistanceUnit { return m_horizontalDistanceUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property minimum speed
     */
    [[nodiscard]] auto minimumSpeed() const -> Units::Speed { return m_minimumSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property name
     */
    [[nodiscard]] auto name() const -> QString { return m_name; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVertialDistanceUnit
     */
    [[nodiscard]] auto verticalDistanceUnit() const -> VerticalDistanceUnit { return m_verticalDistanceUnit; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is
     * outside of the interval [minAircraftSpeed, maxAircraftSpeed], the
     * property will be set to NaN.
     *
     * @param newSpeed Property cruise speed
     */
    void setCruiseSpeed(Units::Speed newSpeed);

    /*! \brief Setter function for property of the same name
     *
     * @param newSpeed Descent speed
     */
    void setDescentSpeed(Units::Speed newSpeed);

    /*! \brief Setter function for property of the same name
     *
     * @param newFuelConsumption Fuel consumption
     */
    void setFuelConsumption(Units::VolumeFlow newFuelConsumption);

    /*! \brief Setter function for property of the same name
     *
     * @param newUnit Property preferredFuelConsumptionUnit
     */
    void setFuelConsumptionUnit(FuelConsumptionUnit newUnit);

    /*! \brief Setter function for property of the same name
     *
     * @param newUnit Property preferredHorizontalDistanceUnit
     */
    void setHorizontalDistanceUnit(HorizontalDistanceUnit newUnit);

    /*! \brief Setter function for property of the same name
     *
     * If newSpeed is outside of the interval [minAircraftSpeed, maxAircraftSpeed], the
     * property will be set to NaN.
     *
     * @param newSpeed Property minimum speed
     */
    void setMinimumSpeed(Units::Speed newSpeed);

    /*! \brief Setter function for property of the same name
     *
     * @param newName Property name
     */
    void setName(const QString& newName);

    /*! \brief Setter function for property of the same name
     *
     * @param newUnit Property preferredVerticalDistanceUnit
     */
    void setVerticalDistanceUnit(VerticalDistanceUnit newUnit);


    //
    // Methods
    //

    /*! \brief Return copy of this object
     *
     *  This is a helper function, used in QML to create explicit copies of this object.
     *
     *  @returns Copy of this object
     */
    Q_INVOKABLE [[nodiscard]] Navigation::Aircraft clone() const { return {*this}; }

    /*! \brief Description of the way between two points
     *
     * @param from Starting point of the way
     *
     * @param to Endpoint of the way
     *
     * @returns A string such as "DIST 65.2 nm • QUJ 276°" or "DIST 65.2 km • QUJ 276°".  If the way cannot be described (e.g. because one of the coordinates is invalid), then an empty string is returned.
     */
    Q_INVOKABLE [[nodiscard]] QString describeWay(const QGeoCoordinate &from, const QGeoCoordinate &to) const;

    /*! \brief Convert horizontal distance to string
     *
     *  This method converts a horizontal distance to a localized string, taking horizontalDistanceUnit into account.
     *
     *  @param distance Distance
     *
     *  @returns A string of the form "280 km", or "-" for an invalid distance
     */
    Q_INVOKABLE [[nodiscard]] QString horizontalDistanceToString(Units::Distance distance) const;

    /*! \brief Convert horizontal speed to string
     *
     *  This method converts a horizontal speed to a localized string, taking horizontalDistanceUnit into account.
     *
     *  @param speed Speed
     *
     *  @returns A string of the form "98 kn", "154 km/h", or "-"
     */
    Q_INVOKABLE [[nodiscard]] QString horizontalSpeedToString(Units::Speed speed) const;

    /*! \brief Reads aircraft data from a JSON document
     *
     * This method loads reads data from a JSON document and stores it in the present object. Notifier signals are emitted as
     * appropriate.  If this method returns a non-empty string, then
     * the JSON data might be partially read.
     *
     * @param fileName File name
     *
     * @returns Empty string in case of success, human-readable, translated
     * error message otherwise.
     */
    Q_INVOKABLE [[nodiscard]] QString loadFromJSON(const QString& fileName);

    /*! \brief Reads aircraft data from a JSON document
     *
     * This method loads reads data from a JSON document and stores it in the present object. Notifier signals are emitted as
     * appropriate.  If this method returns a non-empty string, then
     * the JSON data might be partially read.
     *
     * @param JSON JSON data
     *
     * @returns Empty string in case of success, human-readable, translated
     * error message otherwise.
     */
    Q_INVOKABLE [[nodiscard]] QString loadFromJSON(const QByteArray &JSON);

    /*! \brief Equality check
     *
     *  @param other Aircraft that is compared to this
     *
     *  @result equality
     */
    Q_INVOKABLE bool operator==(const Navigation::Aircraft& other) const;

    /*! \brief Saves aircraft to a file
     *
     * This method saves the aircraft as a JSON file.
     *
     * @param fileName File name, needs to include path and extension
     *
     * @returns Empty string in case of success, human-readable, translated
     * error message otherwise.
     */
    Q_INVOKABLE [[nodiscard]] QString save(const QString& fileName) const;

    /*! \brief Exports to route to JSON
     *
     * This method serialises the object as a JSON
     * document.
     *
     * @returns QByteArray containing JSON code
     */
    Q_INVOKABLE [[nodiscard]] QByteArray toJSON() const;

    /*! \brief Convert vertical distance to string
     *
     *  This method converts a vertical distance to a localized string, taking verticalDistanceUnit into account.
     *
     *  @param distance Distance
     *
     *  @param forceSign Prepend positive number with a sign "+"
     *
     *  @returns A string of the form "1.280 m", "3.500 ft", or "-" for an invalid distance
     */
    Q_INVOKABLE [[nodiscard]] QString verticalDistanceToString(Units::Distance distance, bool forceSign=false) const;

    /*! \brief Convert vertical speed to string
     *
     *  This method converts a vertical speed to a localized string, taking verticalDistanceUnit into account.
     *
     *  @param speed Speed
     *
     *  @returns A string of the form "500 ft/min", "2,5 m/s", or "-"
     */
    Q_INVOKABLE [[nodiscard]] QString verticalSpeedToString(Units::Speed speed) const;

    /*! \brief Convert volume to string
     *
     *  This method converts a volume to a localized string, taking volumeUnit into account.
     *
     *  @param volume Volume
     *
     *  @returns A string of the form "5.2 l", or "-"
     */
    Q_INVOKABLE [[nodiscard]] QString volumeToString(Units::Volume volume) const;

private:
    static constexpr Units::Speed minValidSpeed = Units::Speed::fromKN(10.0);
    static constexpr Units::Speed maxValidSpeed = Units::Speed::fromKN(400.0);
    static constexpr Units::VolumeFlow minValidFuelConsumption = Units::VolumeFlow::fromLPH(0.0);
    static constexpr Units::VolumeFlow maxValidFuelConsumption = Units::VolumeFlow::fromLPH(300.0);

    Units::Speed m_cruiseSpeed {};
    Units::Speed m_descentSpeed {};
    Units::VolumeFlow m_fuelConsumption {};
    FuelConsumptionUnit m_fuelConsumptionUnit {LiterPerHour};
    HorizontalDistanceUnit m_horizontalDistanceUnit {NauticalMile};
    Units::Speed m_minimumSpeed {};
    QString m_name {};
    VerticalDistanceUnit m_verticalDistanceUnit {Feet};
};

} // namespace Navigation

// Make enums available in QML
namespace AircraftQML {
Q_NAMESPACE
QML_FOREIGN_NAMESPACE(Navigation::Aircraft)
QML_NAMED_ELEMENT(Aircraft)
} // Namespace AircraftQML

// Declare meta types
Q_DECLARE_METATYPE(Navigation::Aircraft)
