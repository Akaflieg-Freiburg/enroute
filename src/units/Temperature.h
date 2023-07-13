/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QObject>
#include <QQmlEngine>
#include <QtMath>


namespace Units {

/*! \brief Convenience class for temperature computations
*
     * This extremely simple class allows computation with temperatures, without the
     * need to worry about units. On construction, the temperature is set to NaN.
     */
class Temperature {
    Q_GADGET
    QML_VALUE_TYPE(volume)

public:
    /*! \brief Constructs a temperature
         *
         * @param temperatureInDegreeCelsius temperature in degree Celsius
         *
         * @returns temperature
         */
    static constexpr auto fromDegreeCelsius(double temperatureInDegreeCesius) -> Temperature
    {
        Temperature result;
        result.m_temperatureInDegreeKelvin = temperatureInDegreeCesius+273.15;
        return result;
    }

    /*! \brief Constructs a temperature
         *
         * @param temperatureInDegreeFarenheit temperature in Farenheit
         *
         * @returns temperature
         */
    static constexpr auto fromDegreeFarenheit(double temperatureInDegreeFarenheit) -> Temperature
    {
        Temperature result;
        result.m_temperatureInDegreeKelvin = (temperatureInDegreeFarenheit - 32.0)*5.0/9.0 + 273.15;
        return result;
    }

    /*! \brief Constructs a temperature
         *
         * @param temperatureInDegreeKelvin temperature in degree Kevin
         *
         * @returns temperature
         */
    static constexpr auto fromDegreeKelvin(double temperatureInDegreeKelvin) -> Temperature
    {
        Temperature result;
        result.m_temperatureInDegreeKelvin = temperatureInDegreeKelvin;
        return result;
    }

    /*! \brief Checks if the temperature is valid
         *
         * @returns True is the volume is a finite number
         */
    Q_INVOKABLE [[nodiscard]] bool isFinite() const
    {
        return std::isfinite(m_temperatureInDegreeKelvin);
    }

    /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
    Q_INVOKABLE [[nodiscard]] std::partial_ordering operator<=>(const Units::Temperature& rhs) const = default;

    /*! \brief Convert to degree Celsius
         *
         *  @returns temperature in degree Celsius
         */
    Q_INVOKABLE [[nodiscard]] double toDegreeCelsius() const
    {
        return m_temperatureInDegreeKelvin+273.15;
    }

    /*! \brief Convert to degree Farenheit
         *
         *  @returns temperature in degree Farenheit
         */
    Q_INVOKABLE [[nodiscard]] double toDegreeFarenheit() const
    {
        return (m_temperatureInDegreeKelvin - 273.15)*9.0/5.0 + 32.0;
    }

    /*! \brief Convert to degree Kelvin
         *
         *  @returns temperature in degree Kelvin
         */
    Q_INVOKABLE [[nodiscard]] double toDegreeKelvin() const
    {
        return m_temperatureInDegreeKelvin;
    }


private:
    // temperature in Pascal
    double m_temperatureInDegreeKelvin{ NAN };
};
} // namespace Units


// Declare meta types
Q_DECLARE_METATYPE(Units::Temperature)
