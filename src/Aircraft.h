/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <QSettings>


/*! \brief This extremely simple class holds a few numbers that describe an aircraft */

class Aircraft : public QObject
{
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * This constructor reads the values of the properties listed below via QSettings. The values are set to NaN if no valid numbers can be found in the settings object.
     */
    explicit Aircraft(QObject *parent = nullptr);

    // No copy constructor
    Aircraft(Aircraft const&) = delete;

    // No assign operator
    Aircraft& operator =(Aircraft const&) = delete;

    // No move constructor
    Aircraft(Aircraft&&) = delete;

    // No move assignment operator
    Aircraft& operator=(Aircraft&&) = delete;

    // Standard destructor
    ~Aircraft() override = default;

    /*! \brief Cruise Speed
     *
     * This property holds the cruise speed of the aircraft. This is a number that lies in the interval [minAircraftSpeed, maxAircraftSpeed] or NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(double cruiseSpeedInKT READ cruiseSpeedInKT WRITE setCruiseSpeedInKT NOTIFY valChanged)

    /*! \brief Getter function for property of the same name */
    double cruiseSpeedInKT() const { return _cruiseSpeedInKT; }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is outside of the interval [minAircraftSpeed, maxAircraftSpeed], the property will be set to NaN.
     */
    void setCruiseSpeedInKT(double speedInKT);

    /*! \brief Decent Speed
     *
     * This property holds the descent speed of the aircraft. This is a number that lies in the interval [minAircraftSpeed, maxAircraftSpeed] or NaN if the cruise speed has not been set.
     */
    Q_PROPERTY(double descentSpeedInKT READ descentSpeedInKT WRITE setDescentSpeedInKT NOTIFY valChanged)

    /*! \brief Getter function for property of the same name */
    double descentSpeedInKT() const { return _descentSpeedInKT; }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is outside of the interval [minAircraftSpeed, maxAircraftSpeed], the property will be set to NaN.
     */
    void setDescentSpeedInKT(double speedInKT);

    /*! \brief Fuel Consumption
     *
     * This property holds the fuel consumption of the aircraft. This is a number that lies in the interval [minFuelConsumption, maxFuelConsumption] or NaN if no value has been set.
     */
    Q_PROPERTY(double fuelConsumptionInLPH READ fuelConsumptionInLPH WRITE setFuelConsumptionInLPH NOTIFY valChanged)

    /*! \brief Getter function for property of the same name */
    double fuelConsumptionInLPH() const { return _fuelConsumptionInLPH; }

    /*! \brief Setter function for property of the same name
     *
     * This method saves the new value in a QSetting object. If speedInKT is outside of the interval [minFuelConsumption, maxFuelConsumption], the property will be set to NaN.
     */
    void setFuelConsumptionInLPH(double fuelConsumptionInLPH);

    /*! \brief Minimal speed of the aircraft that is considered valid */
    Q_PROPERTY(double minAircraftSpeed MEMBER minAircraftSpeed CONSTANT)

    /*! \brief Maximal speed of the aircraft that is considered valid */
    Q_PROPERTY(double maxAircraftSpeed MEMBER maxAircraftSpeed CONSTANT)

    /*! \brief Minimal fuel consumption that is considered valid */
    Q_PROPERTY(double minFuelConsuption MEMBER minFuelConsuption CONSTANT)

    /*! \brief Maximal fuel consumption that is considered valid */
    Q_PROPERTY(double maxFuelConsuption MEMBER maxFuelConsuption CONSTANT)

signals:
    /*! \brief Notifier signal */
    void valChanged();

private:
    static constexpr double minAircraftSpeed  =  40.0;
    static constexpr double maxAircraftSpeed  = 400.0;
    static constexpr double minFuelConsuption =   5.0;
    static constexpr double maxFuelConsuption = 100.0;

    double _cruiseSpeedInKT {qQNaN()};
    double _descentSpeedInKT {qQNaN()};
    double _fuelConsumptionInLPH {qQNaN()};

    QSettings settings;
};

#endif
