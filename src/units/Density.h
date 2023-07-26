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

/*! \brief Convenience class for density computations
*
     * This extremely simple class allows computation with Densitys, without the
     * need to worry about units. On construction, the Density is set to NaN.
     */
class Density {
    Q_GADGET
    QML_VALUE_TYPE(density)

public:
    /*! \brief Constructs a density
         *
         * @param densityInKgPerCubeMeter density in kg per m³
         *
         * @returns Density
         */
    static constexpr auto fromKgPerCubeMeter(double densityInKgPerCubeMeter) -> Density
    {
        Density result;
        result.m_densityInKgPerCubeMeter = densityInKgPerCubeMeter;
        return result;
    }


    /*! \brief Checks if the Density is valid
         *
         * @returns True is the volume is a finite number
         */
    [[nodiscard]] Q_INVOKABLE bool isFinite() const
    {
        return std::isfinite(m_densityInKgPerCubeMeter);
    }

    /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
    [[nodiscard]] Q_INVOKABLE std::partial_ordering operator<=>(const Units::Density& rhs) const = default;

    /*! \brief Convert to density to kg per m³degree Kelvin
         *
         *  @returns Density in kg per m³
         */
    [[nodiscard]] Q_INVOKABLE double toKgPerCubeMeter() const
    {
        return m_densityInKgPerCubeMeter;
    }


private:
    // Density in kg per m³
    double m_densityInKgPerCubeMeter{ NAN };
};
} // namespace Units


// Declare meta types
Q_DECLARE_METATYPE(Units::Density)
