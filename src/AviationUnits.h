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

#include <QGeoCoordinate>
#include <QString>
#include <QtMath>

/*! \brief Conversion between units used in aviation
 *
 * This class contains a few static methods that help with the conversion of
 * units, as used in aviation. It contains a few subclasses that computation
 * with angles, speeds, distances, etc, without the need to worry about units.
 */

class AviationUnits {
public:
    /*! \brief Converts string representation of a geographical coordinate into
     * QGeoCoordinate
     *
     * @param geoLat Geographical latitude, as a string of the form
     * "49.00864900N".
     *
     * @param geoLong Geographical longitude, as a string of the form
     * "7.22559722E".
     *
     * @returns A (possibly invalid) QGeoCoordinate
     */
    static QGeoCoordinate stringToCoordinate(const QString &geoLat, const QString &geoLong);

    /*! \brief Convenience class for angle computations
     *
     * This extremely simple class allows computation with angles, without the
     * need to worry about units.
     */
    class Angle {
    public:
        /*! \brief Constructs an angle
         *
         * @param angleInRAD Angle in radian
         *
         * @returns Angle
         */
        static Angle fromRAD(double angleInRAD) {
            Angle result;
            result._angleInRAD = angleInRAD;
            return result;
        }

        /*! \brief Constructs an angle
         *
         * @param angleInDEG Angle in degrees
         *
         * @returns Angle
         */
        static Angle fromDEG(double angleInDEG) {
            Angle result;
            result._angleInRAD = angleInDEG * RAD_per_DEG;
            return result;
        }

        /*! \brief Checks if the angle is valid
         *
         * @returns True is the angle is a finite number
         */
        bool isFinite() const { return std::isfinite(_angleInRAD); }

        /*! \brief Sum of two angles
         *
         * @param rhs Right hand side of the sum
         *
         * @returns Sum of the two angles
         */
        Angle operator+(const Angle &rhs) {
            Angle result;
            result._angleInRAD = _angleInRAD + rhs._angleInRAD;
            return result;
        }

        /*! \brief Difference of two angles
         *
         * @param rhs Right hand side of the difference
         *
         * @returns Difference of the two angles
         */
        Angle operator-(const Angle &rhs) {
            Angle result;
            result._angleInRAD = _angleInRAD - rhs._angleInRAD;
            return result;
        }

        /*! \brief Cosine of an angle, as a dimension-less number
         *
         * @param arg Angle whose cosine is computed
         *
         * @returns Cosine of the angle
         */
        static double cos(const Angle &arg) { return std::cos(arg._angleInRAD); }

        /*! \brief Sine of an angle, as a dimension-less number
         *
         * @param arg Angle whose sine is computed
         *
         * @returns Sine of the angle
         */
        static double sin(const Angle &arg) { return std::sin(arg._angleInRAD); }

        /*! \brief Arcsine of a dimension-less number as an angle
         *
         * @param arg Number whose arcsine is computed
         *
         * @returns Angle computed
         */
        static Angle asin(double arg) {
            Angle result;
            result._angleInRAD = std::asin(arg);
            return result;
        }

        /*! \brief Convert angle to radian
         *
         * @returns Angle, as a number in radian
         */
        double toRAD() const { return _angleInRAD; }

        /*! \brief Convert angle to degrees
         *
         * @returns Angle, as a number in degrees
         */
        double toDEG() const { return _angleInRAD / RAD_per_DEG; }

        /*! \brief Returns angle in degrees, normalized to lie in the interval
         * [0.0, 360.0]
         *
         * @returns Angle, as a number in degrees. The result is NaN, or lies in
         * the interval [0.0, 360.0]
         */
        double toNormalizedDEG() const;

        /*! \brief Returns the angle as a string of the form 167° 31' 13.23"
         *
         * @returns a string of the form 167° 31' 13.23"
         */
        QString toString() const;

    private:
        static constexpr double RAD_per_DEG = M_PI / 180.0;

        // Speed in meters per second
        double _angleInRAD{qQNaN()};
    };

    /*! \brief Convenience class for distance computations
     *
     * This extremely simple class allows computation with distances, without the
     * need to worry about units.
     */
    class Distance {
    public:
        /*! \brief Constructs a distance
         *
         * @param distanceInM distance in meters
         *
         * @returns distance
         */
        static Distance fromM(double distanceInM) {
            Distance result;
            result._distanceInM = distanceInM;
            return result;
        }

        /*! \brief Constructs a distance
         *
         * @param distanceInFT distance in feet
         *
         * @returns distance
         */
        static Distance fromFT(double distanceInFT) {
            Distance result;
            result._distanceInM = distanceInFT * MetersPerFeet;
            return result;
        }

        /*! \brief Add distance to this distance
         *
         * @param other distance to be added
         *
         * @returns reference to this distance
         */
        Distance &operator+=(const Distance &other) {
            _distanceInM += other._distanceInM;
            return *this;
        }

        /*! \brief Checks if the distance is valid
         *
         * @returns True is the distance is a finite number
         */
        bool isFinite() const { return std::isfinite(_distanceInM); }

        /*! \brief Checks if the distance is negative
         *
         * @returns True is the distance is negative
         */
        bool isNegative() const { return _distanceInM < 0.0; }

        /*! \brief Convert to nautical miles
         *
         * @returns distance in nautical miles
         */
        double toNM() const { return _distanceInM / MetersPerNauticalMile; }

        /*! \brief Convert to meters
         *
         * @returns distance in meters
         */
        double toM() const { return _distanceInM; }

        /*! \brief Convert to feet
         *
         * @returns distance in feet
         */
        double toFeet() const { return _distanceInM / MetersPerFeet; }

    private:
        static constexpr double MetersPerFeet = 0.3048;
        static constexpr double MetersPerNauticalMile = 1852;

        // Speed in meters per second
        double _distanceInM{qQNaN()};
    };

    /*! \brief Convenience class for speed computations
     *
     * This extremely simple class allows computation with speeds, without the
     * need to worry about units.
     */
    class Speed {
    public:
        /*! \brief Constructs a speed
         *
         * @param speedInMPS speed in meters per second
         *
         * @returns speed
         */
        static Speed fromMPS(double speedInMPS) {
            Speed result;
            result._speedInMPS = speedInMPS;
            return result;
        }

        /*! \brief Constructs a speed
         *
         * @param speedInKT  speed in knots
         *
         * @returns speed
         */
        static Speed fromKT(double speedInKT) {
            Speed result;
            result._speedInMPS = speedInKT / KT_per_MPS;
            return result;
        }

        /*! \brief Constructs a speed
         *
         * @param speedInKMH  speed in km/h
         *
         * @returns speed
         */
        static Speed fromKMH(double speedInKMH) {
            Speed result;
            result._speedInMPS = speedInKMH / KMH_per_MPS;
            return result;
        }

        /*! \brief Checks if the speed is valid
         *
         * @returns True is the distance is a finite number
         */
        bool isFinite() const { return std::isfinite(_speedInMPS); }

        /*! \brief Checks if the speed is negative
         *
         * @returns True is the distance is negative
         */
        bool isNegative() const { return _speedInMPS < 0.0; }

        /*! \brief Divides two speeds
         *
         * @param rhs Denominator of the division
         *
         * @returns Quotient as a dimension-less number
         */
        double operator/(const Speed &rhs) {
            if (qFuzzyIsNull(rhs._speedInMPS))
                return qQNaN();
            return _speedInMPS / rhs._speedInMPS;
        }

        /*! \brief Convert to meters per second
         *
         * @returns speed in meters per second
         */
        double toMPS() const { return _speedInMPS; }

        /*! \brief Convert to knots
         *
         * @returns speed in knots (=Nautical miles per hour)
         */
        double toKT() const { return _speedInMPS * KT_per_MPS; }

        /*! \brief Convert to knots
         *
         * @returns speed in knots (=Nautical miles per hour)
         */
        double toKMH() const { return _speedInMPS * KMH_per_MPS; }

        /*! \brief Unitless constant: one knot / meters per second
         */
        static constexpr double KT_per_MPS = 1.943844;

        /*! \brief Unitless constant: one km/h / meters per second
         */
        static constexpr double KMH_per_MPS = 3.6;

        /*! \brief Unitless constant: one km/h / knot
         */
        static constexpr double KMH_per_KT = KMH_per_MPS / KT_per_MPS;

    private:
        // Speed in meters per second
        double _speedInMPS{qQNaN()};
    };

    /*! \brief Convenience class for time computations
     *
     * This extremely simple class allows computation with times, without the need
     * to worry about units.
     */
    class Time {
    public:
        /*! \brief Constructs a time
         *
         * @param timeInS  time in seconds
         *
         * @returns time
         */
        static Time fromS(double timeInS) {
            Time result;
            result._timeInS = timeInS;
            return result;
        }

        /*! \brief Checks if the time is valid
         *
         * @returns True is the distance is a finite number
         */
        bool isFinite() const { return std::isfinite(_timeInS); }

        /*! \brief Checks if the time is negative
         *
         * @returns True is the time is negative
         */
        bool isNegative() const { return _timeInS < 0.0; }

        /*! \brief Add time to this time
         *
         * @param other time to be added
         *
         * @returns reference to this time
         */
        Time &operator+=(const Time &other) {
            _timeInS += other._timeInS;
            return *this;
        }

        /*! \brief Convert time to seconds
         *
         * @return time in seconds
         */
        double toS() const { return _timeInS; }

        /*! \brief Convert time to minutes
         *
         * @return time in minutes
         */
        double toM() const { return _timeInS / Seconds_per_Minute; }

        /*! \brief Convert time to hours
         *
         * @return time in hours
         */
        double toH() const { return _timeInS / Seconds_per_Hour; }

        /*! \brief Convert time to string
         *
         * @returns time in hours and minutes, as a string of the form 7:12 or -0:05
         */
        QString toHoursAndMinutes() const;

    private:
        static constexpr double Seconds_per_Minute = 60.0;
        static constexpr double Seconds_per_Hour = 60.0 * 60.0;

        // Speed in meters per second
        double _timeInS{qQNaN()};
    };
};

/*! \brief Divide distance and speed
 *
 * @param dist numerator
 *
 * @param speed denominator
 *
 * @returns quotient of numerator and denominator as time
 */
[[maybe_unused]] static AviationUnits::Time operator/(const AviationUnits::Distance &dist,
                                                      const AviationUnits::Speed &speed) {
    if ((!dist.isFinite()) || (!speed.isFinite()) || (qFuzzyIsNull(speed.toMPS())))
        return {};
    return AviationUnits::Time::fromS(dist.toM() / speed.toMPS());
}
