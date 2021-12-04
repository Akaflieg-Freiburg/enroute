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

#include <QSettings>

#include "units/Speed.h"
#include "units/VolumeFlow.h"


namespace Navigation {

/*! \brief This extremely simple class holds a few numbers that describe an
    aircraft */

class Aircraft : public QObject {
    Q_OBJECT

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
    // Constructor and destructor
    //

    /*! \brief Default constructor
     *
     * This constructor reads the values of the properties listed below via
     * QSettings. The values are set to NaN if no valid numbers can be found
     * in the settings object.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Aircraft(QObject *parent = nullptr);

    // Standard destructor
    ~Aircraft() override = default;


    //
    // Properties
    //

    /*! \brief Cruise Speed
     *
     * This property holds the cruise speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed cruiseSpeed READ cruiseSpeed WRITE setCruiseSpeed NOTIFY cruiseSpeedChanged)

    /*! \brief Decent Speed
     *
     * This property holds the descent speed of the aircraft. This is a
     * number that lies in the interval [minAircraftSpeed, maxAircraftSpeed]
     * or NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed descentSpeed READ descentSpeed WRITE setDescentSpeed NOTIFY descentSpeedChanged)

    /*! \brief Fuel Consumption
     *
     * This property holds the fuel consumption of the aircraft. This is a
     * number that lies in the interval [minFuelConsumption,
     * maxFuelConsumption] or NaN if no value has been set.
     */
    Q_PROPERTY(Units::VolumeFlow fuelConsumption READ fuelConsumption WRITE setFuelConsumption NOTIFY fuelConsumptionChanged)

    /*! \brief Preferred units of measurement for fuel consumption */
    Q_PROPERTY(FuelConsumptionUnit fuelConsumptionUnit READ fuelConsumptionUnit WRITE setFuelConsumptionUnit NOTIFY fuelConsumptionUnitChanged)

    /*! \brief Preferred units of measurement for horizontal distances */
    Q_PROPERTY(HorizontalDistanceUnit horizontalDistanceUnit READ horizontalDistanceUnit WRITE setHorizontalDistanceUnit NOTIFY horizontalDistanceUnitChanged)

    /*! \brief Maximal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed maxValidSpeed MEMBER maxValidSpeed CONSTANT)

    /*! \brief Maximal fuel consumption that is considered valid */
    Q_PROPERTY(Units::VolumeFlow maxValidFuelConsumption MEMBER maxValidFuelConsumption CONSTANT)

    /*! \brief Minimum Speed
     *
     * This property holds the minimum speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the minimum speed has not been set.
     */
    Q_PROPERTY(Units::Speed minimumSpeed READ minimumSpeed WRITE setMinimumSpeed NOTIFY minimumSpeedChanged)

    /*! \brief Minimal fuel consumption that is considered valid */
    Q_PROPERTY(Units::VolumeFlow minValidFuelConsumption MEMBER minValidFuelConsumption CONSTANT)

    /*! \brief Minimal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed minValidSpeed MEMBER minValidSpeed CONSTANT)

    /*! \brief Name
     *
     * This property holds the name.
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /*! \brief Preferred units of measurement for vertical distances */
    Q_PROPERTY(VerticalDistanceUnit verticalDistanceUnit READ verticalDistanceUnit WRITE setVerticalDistanceUnit NOTIFY verticalDistanceUnitChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property cruise speed
     */
    Units::Speed cruiseSpeed() const { return _cruiseSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property descentSpeed
     */
    Units::Speed descentSpeed() const { return _descentSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property fuelConsumptionInLPH
     */
    Units::VolumeFlow fuelConsumption() const { return _fuelConsumption; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVolumeUnit
     */
    FuelConsumptionUnit fuelConsumptionUnit() const { return _fuelConsumptionUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredHorizontalDistanceUnit
     */
    HorizontalDistanceUnit horizontalDistanceUnit() const { return _horizontalDistanceUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property minimum speed
     */
    Units::Speed minimumSpeed() const { return _minimumSpeed; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property name
     */
    QString name() const { return _name; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVertialDistanceUnit
     */
    VerticalDistanceUnit verticalDistanceUnit() const { return _verticalDistanceUnit; }


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
    Q_INVOKABLE QString loadFromJSON(const QString& fileName);


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
    Q_INVOKABLE QString loadFromJSON(const QByteArray &JSON);

    /*! \brief Saves aircraft to a file
     *
     * This method saves the aircraft as a JSON file.
     *
     * @param fileName File name, needs to include path and extension
     *
     * @returns Empty string in case of success, human-readable, translated
     * error message otherwise.
     */
    Q_INVOKABLE QString save(const QString& fileName) const;

    /*! \brief Exports to route to JSON
     *
     * This method serialises the object as a JSON
     * document.
     *
     * @returns QByteArray containing JSON code
     */
    Q_INVOKABLE QByteArray toJSON() const;


signals:   
    /*! \brief Notifier signal */
    void cruiseSpeedChanged();

    /*! \brief Notifier signal */
    void descentSpeedChanged();

    /*! \brief Notifier signal */
    void fuelConsumptionChanged();

    /*! \brief Notifier signal */
    void fuelConsumptionUnitChanged();

    /*! \brief Notifier signal */
    void horizontalDistanceUnitChanged();

    /*! \brief Notifier signal */
    void minimumSpeedChanged();

    /*! \brief Notifier signal */
    void nameChanged();

    /*! \brief Notifier signal */
    void verticalDistanceUnitChanged();

private:
    Q_DISABLE_COPY_MOVE(Aircraft)

    static constexpr Units::Speed minValidSpeed = Units::Speed::fromKN(10.0);
    static constexpr Units::Speed maxValidSpeed = Units::Speed::fromKN(400.0);
    static constexpr Units::VolumeFlow minValidFuelConsumption = Units::VolumeFlow::fromLPH(0.0);
    static constexpr Units::VolumeFlow maxValidFuelConsumption = Units::VolumeFlow::fromLPH(300.0);

    Units::Speed _cruiseSpeed {};
    Units::Speed _descentSpeed {};
    Units::VolumeFlow _fuelConsumption {};
    FuelConsumptionUnit _fuelConsumptionUnit {LiterPerHour};
    HorizontalDistanceUnit _horizontalDistanceUnit {NauticalMile};
    Units::Speed _minimumSpeed {};
    QString _name {};
    VerticalDistanceUnit _verticalDistanceUnit {Feet};

    QSettings settings;
};

}
