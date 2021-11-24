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

#include <QDataStream>
#include <QtMath>
#include <QObject>

namespace Units {

    /*! \brief Convenience class for speed computations
     *
     * This extremely simple class allows computation with speeds, without the
     * need to worry about units.
     */
    class Speed {
        Q_GADGET

    public:        
        /*! \brief Constructs a speed
         *
         * @param speedInFPM speed in feet per minute
         *
         * @returns speed
         */
        Q_INVOKABLE static constexpr Units::Speed fromFPM(double speedInFPM)
        {
            Speed result;
            result._speedInMPS = speedInFPM/FPM_per_MPS;
            return result;
        }

        /*! \brief Constructs a speed
         *
         * @param speedInMPS speed in meters per second
         *
         * @returns speed
         */
        Q_INVOKABLE static constexpr Units::Speed fromMPS(double speedInMPS)
        {
            Speed result;
            result._speedInMPS = speedInMPS;
            return result;
        }


        /*! \brief Constructs a speed
         *
         * @param speedInMPH speed in statute miles per hous
         *
         * @returns speed
         */
        Q_INVOKABLE static constexpr Units::Speed fromMPH(double speedInMPH)
        {
            Speed result;
            result._speedInMPS = speedInMPH*MPS_per_MPH;
            return result;
        }

        /*! \brief Constructs a speed
         *
         * @param speedInKT  speed in knots
         *
         * @returns speed
         */
        Q_INVOKABLE static constexpr Units::Speed fromKN(double speedInKT)
        {
            Speed result;
            result._speedInMPS = speedInKT / KN_per_MPS;
            return result;
        }

        /*! \brief Constructs a speed
         *
         * @param speedInKMH speed in km/h
         *
         * @returns speed
         */
        Q_INVOKABLE static constexpr Units::Speed fromKMH(double speedInKMH)
        {
            Speed result;
            result._speedInMPS = speedInKMH / KMH_per_MPS;
            return result;
        }

        /*! \brief Checks if the speed is valid
         *
         * @returns True is the speed is a finite number
         */
        Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(_speedInMPS);
        }

        /*! \brief Checks if the speed is negative
         *
         * @returns True is the distance is negative
         */
        Q_INVOKABLE bool isNegative() const
        {
            return _speedInMPS < 0.0;
        }

        /*! \brief Divides two speeds
         *
         * @param rhs Denominator of the division
         *
         * @returns Quotient as a dimension-less number
         */
        Q_INVOKABLE double operator/(Units::Speed rhs)
        {
            if (qFuzzyIsNull(rhs._speedInMPS))
                return qQNaN();
            return _speedInMPS / rhs._speedInMPS;
        }

        /*! \brief Equality check
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE bool operator==(Units::Speed rhs) const
        {
            return _speedInMPS == rhs._speedInMPS;
        }

        /*! \brief Equality check
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE bool operator!=(Units::Speed rhs) const
        {
            return _speedInMPS != rhs._speedInMPS;
        }

        /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE bool operator<(Units::Speed rhs) const
        {
            return _speedInMPS < rhs._speedInMPS;
        }

        /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE bool operator>(Units::Speed rhs) const
        {
            return _speedInMPS > rhs._speedInMPS;
        }

        /*! \brief Convert to feet per minute
         *
         * @returns Speed in feet per minute
         */
        Q_INVOKABLE double toFPM() const
        {
            return _speedInMPS * FPM_per_MPS;
        }

        /*! \brief Convert to meters per second
         *
         * @returns speed in meters per second
         */
        Q_INVOKABLE double toMPS() const
        {
            return _speedInMPS;
        }

        /*! \brief Convert to meters per second
         *
         * @returns speed in status miles per hour
         */
        Q_INVOKABLE double toMPH() const
        {
            return _speedInMPS/MPS_per_MPH;
        }

        /*! \brief Convert to knots
         *
         * @returns speed in knots (=Nautical miles per hour)
         */
        Q_INVOKABLE double toKN() const
        {
            return _speedInMPS * KN_per_MPS;
        }

        /*! \brief Convert to km/h
         *
         * @returns speed in knots (=Nautical miles per hour)
         */
        Q_INVOKABLE double toKMH() const
        {
            return _speedInMPS * KMH_per_MPS;
        }

        /*! \brief Unitless constant: one feet per minute / meters per second */
        static constexpr double FPM_per_MPS = 196.85039370079;

        /*! \brief Unitless constant: one mile per hour / meters per second */
        static constexpr double MPS_per_MPH = 0.44704;

        /*! \brief Unitless constant: one km/h / meters per second */
        static constexpr double KMH_per_MPS = 3.6;

        /*! \brief Unitless constant: one knot / meters per second */
        static constexpr double KN_per_MPS = 1.943844;



        /*! \brief Unitless constant: one km/h / knot */
        static constexpr double KMH_per_KT = KMH_per_MPS / KN_per_MPS;

    private:
        // Speed in meters per second
        double _speedInMPS{ NAN };
    };

};


/*! \brief Serialization of a speed object into a QDataStream
 *
 * @param out QDataStream that the object is written to
 *
 * @param speed Speed object that is written to the QDataStrem
 *
 * @returns Reference to the QDataStream
 */
QDataStream &operator<<(QDataStream &out, Units::Speed speed);


/*! \brief Deserialization of a speed object into a QDataStream
 *
 * @param in QDataStream that the object is written from
 *
 * @param speed Speed object that is read from the QDataStrem
 *
 * @returns Reference to the QDataStream
 */
QDataStream &operator>>(QDataStream &in, Units::Speed &speed);


// Declare meta types
Q_DECLARE_METATYPE(Units::Speed)
