/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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


namespace Navigation {

/*! \brief This extremely simple class holds a few numbers that describe an
    aircraft */

class Aircraft : public QObject {
    Q_OBJECT

public:
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

    /*! \brief Cruise Speed
     *
     * This property holds the cruise speed of the aircraft. This lies in the interval [minAircraftSpeed,
     * maxAircraftSpeed] or if NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed cruiseSpeed READ cruiseSpeed WRITE setCruiseSpeed NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property cruise speed
     */
    Units::Speed cruiseSpeed() const
    {
        return _cruiseSpeed;
    }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is
     * outside of the interval [minAircraftSpeed, maxAircraftSpeed], the
     * property will be set to NaN.
     *
     * @param newSpeed Property cruise speed
     */
    void setCruiseSpeed(Units::Speed newSpeed);

    /*! \brief Decent Speed
     *
     * This property holds the descent speed of the aircraft. This is a
     * number that lies in the interval [minAircraftSpeed, maxAircraftSpeed]
     * or NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(Units::Speed descentSpeed READ descentSpeed WRITE setDescentSpeed NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property descentSpeed
     */
    Units::Speed descentSpeed() const
    {
        return _descentSpeed;
    }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If newSpeed is
     * outside of the interval [minAircraftSpeed, maxAircraftSpeed], the
     * property will be set to NaN.
     *
     * @param newSpeed Descent speed
     */
    void setDescentSpeed(Units::Speed newSpeed);

    /*! \brief Fuel Consumption
     *
     * This property holds the fuel consumption of the aircraft. This is a
     * number that lies in the interval [minFuelConsumption,
     * maxFuelConsumption] or NaN if no value has been set.
     */
    Q_PROPERTY(double fuelConsumptionInLPH READ fuelConsumptionInLPH WRITE setFuelConsumptionInLPH NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property fuelConsumptionInLPH
     */
    double fuelConsumptionInLPH() const { return _fuelConsumptionInLPH; }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is
     * outside of the interval [minFuelConsumption, maxFuelConsumption], the
     * property will be set to NaN.
     *
     * @param fuelConsumptionInLPH Fuel consumption in liters per hour
     */
    void setFuelConsumptionInLPH(double fuelConsumptionInLPH);

    /*! \brief Minimal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed minAircraftSpeed MEMBER minAircraftSpeed CONSTANT)

    /*! \brief Maximal speed of the aircraft that is considered valid */
    Q_PROPERTY(Units::Speed maxAircraftSpeed MEMBER maxAircraftSpeed CONSTANT)

    /*! \brief Minimal fuel consumption that is considered valid */
    Q_PROPERTY(double minFuelConsuption MEMBER minFuelConsuption CONSTANT)

    /*! \brief Maximal fuel consumption that is considered valid */
    Q_PROPERTY(double maxFuelConsuption MEMBER maxFuelConsuption CONSTANT)

signals:
    /*! \brief Notifier signal */
    void valChanged();

private:
    Q_DISABLE_COPY_MOVE(Aircraft)

    static constexpr Units::Speed minAircraftSpeed = Units::Speed::fromKN(10.0);
    static constexpr Units::Speed maxAircraftSpeed = Units::Speed::fromKN(400.0);
    static constexpr double minFuelConsuption = 0.0;
    static constexpr double maxFuelConsuption = 300.0;

    Units::Speed _cruiseSpeed {};
    Units::Speed _descentSpeed {};
    double _fuelConsumptionInLPH{qQNaN()};

    QSettings settings;
};

}
