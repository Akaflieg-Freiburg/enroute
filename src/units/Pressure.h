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

    /*! \brief Convenience class for pressure computations
     *
     * This extremely simple class allows computation with pressures, without the
     * need to worry about units. On construction, the pressure is set to NaN.
     */
    class Pressure {
        Q_GADGET
        QML_VALUE_TYPE(volume)

    public:
        /*! \brief Constructs a pressure
         *
         * @param pressureInHPa pressure in hectoPascal
         *
         * @returns pressure
         */
        static constexpr auto fromHPa(double pressureInHPa) -> Pressure
        {
            Pressure result;
            result.m_pressureInPa = 100.0*pressureInHPa;
            return result;
        }

        /*! \brief Constructs a pressure
         *
         * @param pressureInInHg pressure in inches mercury
         *
         * @returns pressure
         */
        static constexpr auto fromInHg(double pressureInInHg) -> Pressure
        {
            Pressure result;
            result.m_pressureInPa = PascalPerInHg*pressureInInHg;
            return result;
        }

        /*! \brief Constructs a pressure
         *
         * @param pressureInPa pressure in Pascal
         *
         * @returns pressure
         */
        static constexpr auto fromPa(double pressureInPa) -> Pressure
        {
            Pressure result;
            result.m_pressureInPa = pressureInPa;
            return result;
        }

        /*! \brief Checks if the pressure is valid
         *
         * @returns True is the volume is a finite number
         */
        Q_INVOKABLE [[nodiscard]] bool isFinite() const
        {
            return std::isfinite(m_pressureInPa);
        }

        /*! \brief Add pressure to this pressure
         *
         * @param other pressure to be added
         *
         * @returns reference to this volume
         */
        Q_INVOKABLE Units::Pressure& operator+=(Units::Pressure other)
        {
            m_pressureInPa += other.m_pressureInPa;
            return *this;
        }

        /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE [[nodiscard]] std::partial_ordering operator<=>(const Units::Pressure& rhs) const = default;

        /*! \brief Convert to Hectopascal
         *
         *  @returns pressure in Hectopascal
         */
        Q_INVOKABLE [[nodiscard]] double toHPa() const
        {
            return m_pressureInPa/100.0;
        }

        /*! \brief Convert to Inches of Mercury
         *
         *  @returns pressure in inches of Mercury
         */
        Q_INVOKABLE [[nodiscard]] double toInHg() const
        {
            return m_pressureInPa/PascalPerInHg;
        }

        /*! \brief Convert to Pascal
         *
         *  @returns pressure in Pascal
         */
        Q_INVOKABLE [[nodiscard]] double toPa() const
        {
            return m_pressureInPa;
        }

    private:
        static constexpr double PascalPerInHg = 3386.39;

        // Pressure in Pascal
        double m_pressureInPa{ NAN };
    };
} // namespace Units


// Declare meta types
Q_DECLARE_METATYPE(Units::Pressure)
