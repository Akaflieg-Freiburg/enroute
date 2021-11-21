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
#include "units/Volume.h"


namespace Navigation {

/*! \brief This extremely simple class holds a few numbers that describe an
    aircraft */

class Aircraft : public QObject {
    Q_OBJECT

public:
    /*! \brief Units of measurement for horizontal distances */
    enum HorizontalDistanceUnit {
        /*! \brief Nautical Mile */
        nauticalMile,

        /*! \brief Kilometer */
        kilometer,

        /*! \brief Statute Mile */
        statuteMile
    };

    /*! \brief Units of measurement for vertical distances */
    enum VerticalDistanceUnit {
        /*! \brief Feet */
        feet,

        /*! \brief Meters */
        meters
    };

    /*! \brief Units of measurement for volumes */
    enum VolumeUnit {
        /*! \brief Liter */
        liter,

        /*! \brief Gallon */
        gallon
    };

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

    /*! \brief Fuel Consumption
     *
     * This property holds the fuel consumption of the aircraft. This is a
     * number that lies in the interval [minFuelConsumption,
     * maxFuelConsumption] or NaN if no value has been set.
     */
    Q_PROPERTY(Units::Volume fuelConsumptionPerHour READ fuelConsumptionPerHour WRITE setFuelConsumptionPerHour NOTIFY fuelConsumptionPerHourChanged)

    /*! \brief Maximal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed maxValidTAS MEMBER maxValidTAS CONSTANT)

    /*! \brief Maximal fuel consumption that is considered valid */
    Q_PROPERTY(Units::Volume maxValidFuelConsuption MEMBER maxValidFuelConsuption CONSTANT)

    /*! \brief Minimal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed minValidTAS MEMBER minValidTAS CONSTANT)

    /*! \brief Minimal fuel consumption that is considered valid */
    Q_PROPERTY(Units::Volume minValidFuelConsuption MEMBER minValidFuelConsuption CONSTANT)

    /*! \brief Name
     *
     * This property holds the name.
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /*! \brief Preferred units of measurement for horizontal distances */
    Q_PROPERTY(HorizontalDistanceUnit preferredHorizontalDistanceUnit READ preferredHorizontalDistanceUnit WRITE setPreferredHorizontalDistanceUnit NOTIFY preferredHorizontalDistanceUnitChanged)

    /*! \brief Preferred units of measurement for vertical distances */
    Q_PROPERTY(VerticalDistanceUnit preferredVerticalDistanceUnit READ preferredVerticalDistanceUnit WRITE setPreferredVerticalDistanceUnit NOTIFY preferredVerticalDistanceUnitChanged)

    /*! \brief Preferred units of measurement for volumes */
    Q_PROPERTY(VolumeUnit preferredVolumeUnit READ preferredVolumeUnit WRITE setPreferredVolumeUnit NOTIFY preferredVolumeUnitChanged)

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

    /*! \brief Minimum Speed
     *
     * This property holds the minimum speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the minimum speed has not been set.
     */
    Q_PROPERTY(Units::Speed minimumSpeed READ minimumSpeed WRITE setMinimumSpeed NOTIFY minimumSpeedChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property fuelConsumptionInLPH
     */
    Units::Volume fuelConsumptionPerHour() const { return _fuelConsumptionPerHour; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property name
     */
    QString name() const { return _name; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredHorizontalDistanceUnit
     */
    HorizontalDistanceUnit preferredHorizontalDistanceUnit() const { return _preferredHorizontalDistanceUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVertialDistanceUnit
     */
    VerticalDistanceUnit preferredVerticalDistanceUnit() const { return _preferredVerticalDistanceUnit; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferredVolumeUnit
     */
    VolumeUnit preferredVolumeUnit() const { return _preferredVolumeUnit; }

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
     * @returns Property minimum speed
     */
    Units::Speed minimumSpeed() const { return _minimumSpeed; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     * @param newFuelConsumptionPerHour Fuel consumption per hour
     */
    void setFuelConsumptionPerHour(Units::Volume newFuelConsumptionPerHour);

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
     * @param newUnit Property preferredHorizontalDistanceUnit
     */
    void setPreferredHorizontalDistanceUnit(HorizontalDistanceUnit newUnit);

    /*! \brief Setter function for property of the same name
     *
     * @param newUnit Property preferredVerticalDistanceUnit
     */
    void setPreferredVerticalDistanceUnit(VerticalDistanceUnit newUnit);

    /*! \brief Setter function for property of the same name
     *
     * @param newUnit Property preferredVolumeUnit
     */
    void setPreferredVolumeUnit(VolumeUnit newUnit);

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

signals:   
    /*! \brief Notifier signal */
    void fuelConsumptionPerHourChanged();

    /*! \brief Notifier signal */
    void nameChanged();

    /*! \brief Notifier signal */
    void preferredHorizontalDistanceUnitChanged();

    /*! \brief Notifier signal */
    void preferredVerticalDistanceUnitChanged();

    /*! \brief Notifier signal */
    void preferredVolumeUnitChanged();

    /*! \brief Notifier signal */
    void cruiseSpeedChanged();

    /*! \brief Notifier signal */
    void descentSpeedChanged();

    /*! \brief Notifier signal */
    void minimumSpeedChanged();

private:
    Q_DISABLE_COPY_MOVE(Aircraft)

    static constexpr Units::Speed minValidTAS = Units::Speed::fromKN(10.0);
    static constexpr Units::Speed maxValidTAS = Units::Speed::fromKN(400.0);
    static constexpr Units::Volume minValidFuelConsuption = Units::Volume::fromL(0.0);
    static constexpr Units::Volume maxValidFuelConsuption = Units::Volume::fromL(300.0);

    Units::Speed _cruiseSpeed {};
    Units::Speed _descentSpeed {};
    Units::Volume _fuelConsumptionPerHour {};
    Units::Speed _minimumSpeed {};
    QString _name {};

    HorizontalDistanceUnit _preferredHorizontalDistanceUnit {nauticalMile};
    VerticalDistanceUnit _preferredVerticalDistanceUnit {feet};
    VolumeUnit _preferredVolumeUnit {liter};

    QSettings settings;
};

}
