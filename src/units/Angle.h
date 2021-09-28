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

#include <QtMath>
#include <QObject>

/*! \brief Conversion between units used in aviation
 *
 * This class contains a few classes that help with the conversion of
 * units, as used in aviation. It contains a few subclasses that computation
 * with angles, speeds, distances, etc, without the need to worry about units.
 */

namespace Units {

    /*! \brief Convenience class for angle computations
     *
     * This extremely simple class allows computation with angles, without the
     * need to worry about units.
     */
    class Angle {
        Q_GADGET

    public:
        /*! \brief Constructs an angle
         *
         * @param angleInRAD Angle in radian
         *
         * @returns Angle
         */
        Q_INVOKABLE static Units::Angle fromRAD(double angleInRAD)
        {
            Angle result;
            result.m_angleInRAD = angleInRAD;
            return result;
        }

        /*! \brief Constructs an angle
         *
         * @param angleInDEG Angle in degrees
         *
         * @returns Angle
         */
        Q_INVOKABLE static Units::Angle fromDEG(double angleInDEG)
        {
            Angle result;
            result.m_angleInRAD = qDegreesToRadians(angleInDEG);
            return result;
        }

        /*! \brief Constructs an invalid angle
         *
         * @returns Invalid Angle
         */
        Q_INVOKABLE static Units::Angle nan()
        {
            return {};
        }

        /*! \brief Checks if the angle is valid
         *
         * @returns True is the angle is a finite number
         */
        Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(m_angleInRAD);
        }

        /*! \brief Sum of two angles
         *
         * @param rhs Right hand side of the sum
         *
         * @returns Sum of the two angles
         */
        Q_INVOKABLE Units::Angle operator+(Units::Angle rhs)
        {
            Angle result;
            result.m_angleInRAD = m_angleInRAD + rhs.m_angleInRAD;
            return result;
        }

        /*! \brief Difference of two angles
         *
         * @param rhs Right hand side of the difference
         *
         * @returns Difference of the two angles
         */
        Q_INVOKABLE Units::Angle operator-(Units::Angle rhs)
        {
            Angle result;
            result.m_angleInRAD = m_angleInRAD - rhs.m_angleInRAD;
            return result;
        }

        /*! \brief Comparison: equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator==(Units::Angle rhs) const
        {
            return m_angleInRAD == rhs.m_angleInRAD;
        }

        /*! \brief Comparison: not equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator!=(Units::Angle rhs) const
        {
            return m_angleInRAD != rhs.m_angleInRAD;
        }

        /*! \brief Cosine of an angle, as a dimension-less number
         *
         * @returns Cosine of the angle
         */
        Q_INVOKABLE double cos() const
        {
            return std::cos(m_angleInRAD);
        }

        /*! \brief Sine of an angle, as a dimension-less number
         *
         * @returns Sine of the angle
         */
        Q_INVOKABLE double sin() const
        {
            return std::sin(m_angleInRAD);
        }

        /*! \brief Arcsine of a dimension-less number as an angle
         *
         * @param arg Number whose arcsine is computed
         *
         * @returns Angle computed
         */
        Q_INVOKABLE static Units::Angle asin(double arg) {
            Angle result;
            result.m_angleInRAD = std::asin(arg);
            return result;
        }

        /*! \brief Convert angle to clock position
         *
         * @returns Translated, human-readable string of of the form "12 o'clock", or "-" if the angle is not finite.
         */
        Q_INVOKABLE QString toClock() const;

        /*! \brief Convert angle to degrees
         *
         * @returns Angle, as a number in degrees. The result is NaN, or lies in the interval [0.0, 360.0]
         */
        Q_INVOKABLE double toDEG() const
        {
            if (!qIsFinite(m_angleInRAD)) {
                return qQNaN();
            }

            auto d = std::fmod(qRadiansToDegrees(m_angleInRAD), 360.0);
            if (d > 0)
                return d;
            return d+360.0;
        }

        /*! \brief Convert angle to radian
         *
         * @returns Angle, as a number in radian
         */
        Q_INVOKABLE double toRAD() const
        {
            return m_angleInRAD;
        }

    private:
        // Angle in Radians
        double m_angleInRAD{qQNaN()};
    };
};

// Declare meta types
Q_DECLARE_METATYPE(Units::Angle)
