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

#include <QGeoCoordinate>
#include <QtMath>
#include <QString>

#ifndef AVIATIONUNITS_H
#define AVIATIONUNITS_H

/*! \brief Conversion between units used in aviation
 *
 * This class contains a few static methods that help with the conversion of
 * units, as used in aviation. It contains a few subclasses that computation
 * with angles, speeds, distances, etc, without the need to worry about units.
 */

class AviationUnits {
public:
    /*! \brief Converts string representation of a geographical coordinate into
      QGeoCoordinate
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
        /*! \brief Constructs a speed object from a speed given in meters per
          second */
        static Angle fromRAD(double angleInRAD) {
            Angle result;
            result._angleInRAD = angleInRAD;
            return result;
        }

        /*! \brief Constructs a speed object from a speed given in knots */
        static Angle fromDEG(double angleInDEG) {
            Angle result;
            result._angleInRAD = angleInDEG*RAD_per_DEG;
            return result;
        }

        /*! \brief Checks if the speed is valid */
        bool isFinite() const { return std::isfinite(_angleInRAD); }

        /*! \brief Sum of two angles */
        Angle operator+(const Angle &rhs) {
            Angle result;
            result._angleInRAD = _angleInRAD + rhs._angleInRAD;
            return result;
        }

        /*! \brief Difference of two angles */
        Angle operator-(const Angle &rhs) {
            Angle result;
            result._angleInRAD = _angleInRAD - rhs._angleInRAD;
            return result;
        }

        /*! \brief Sine of an angle, as a dimension-less number */
        static double cos(const Angle &arg) { return std::cos( arg._angleInRAD ); }

        /*! \brief Sine of an angle, as a dimension-less number */
        static double sin(const Angle &arg) { return std::sin( arg._angleInRAD ); }

        /*! \brief Arcsine of a dimension-less number as an angle */
        static Angle asin(double arg) {
            Angle result;
            result._angleInRAD = std::asin(arg);
            return result;
        }

        /*! \brief Returns angle in radian */
        double toRAD() const { return _angleInRAD; }

        /*! \brief Returns angle in degrees */
        double toDEG() const { return _angleInRAD/RAD_per_DEG; }

        /*! \brief Returns angle in degrees. The result is NaN, or lies in the interval [0.0, 360.0] */
        double toNormalizedDEG() const;

        /*! \brief Returns a string of the form 167° 31' 13.23" */
        QString toString() const;

    private:
        static constexpr double RAD_per_DEG = M_PI/180.0;

        // Speed in meters per second
        double _angleInRAD {qQNaN()};
    };


    /*! \brief Convenience class for distance computations
   *
   * This extremely simple class allows computation with distances, without the
   * need to worry about units.
   */
    class Distance {
    public:

        /*! \brief Constructs a distance object from a distance given in meters */
        static Distance fromM(double distanceInM) {
            Distance result;
            result._distanceInM = distanceInM;
            return result;
        }

        /*! \brief Constructs a distance object from a distance given in feet */
        static Distance fromFT(double distanceInFT) {
            Distance result;
            result._distanceInM = distanceInFT*MetersPerFeet;
            return result;
        }

        /*! \brief Adding distances */
        Distance& operator+=(const Distance &other) {
            _distanceInM += other._distanceInM;
            return *this;
        }

        /*! \brief Checks if the distance is valid */
        bool isFinite() const { return std::isfinite(_distanceInM); }

        /*! \brief Checks if the distance is negative */
        bool isNegative() const { return _distanceInM < 0.0; }

        /*! \brief Returns distance in nautical miles */
        double toNM() const { return _distanceInM/MetersPerNauticalMile; }

        /*! \brief Returns distance in meters */
        double toM() const { return _distanceInM; }

        /*! \brief Returns distance in feet */
        double toFeet() const { return _distanceInM/MetersPerFeet; }

    private:
        static constexpr double MetersPerFeet         = 0.3048;
        static constexpr double MetersPerNauticalMile = 1852;

        // Speed in meters per second
        double _distanceInM {qQNaN()};
    };


    /*! \brief Convenience class for speed computations
   *
   * This extremely simple class allows computation with speeds, without the
   * need to worry about units.
   */
    class Speed {
    public:

        /*! \brief Constructs a speed object from a speed given in meters per
          second */
        static Speed fromMPS(double speedInMPS) {
            Speed result;
            result._speedInMPS=speedInMPS;
            return result;
        }

        /*! \brief Constructs a speed object from a speed given in knots */
        static Speed fromKT(double speedInKT) {
            Speed result;
            result._speedInMPS = speedInKT/KT_per_MPS;
            return result;
        }

        /*! \brief Checks if the speed is valid */
        bool isFinite() const { return std::isfinite(_speedInMPS); }

        /*! \brief Checks if the speed is negative */
        bool isNegative() const { return _speedInMPS < 0.0; }

        /*! \brief Divides two speeds, returns a dimension-less number */
        double operator/(const Speed &rhs) {
            if (qFuzzyIsNull(rhs._speedInMPS))
                return qQNaN();
            return _speedInMPS/rhs._speedInMPS;
        }

        /*! \brief Returns speed in meters per second */
        double toMPS() const { return _speedInMPS; }

        /*! \brief Returns speed in knots */
        double toKT() const { return _speedInMPS*KT_per_MPS; }

    private:
        static constexpr double KT_per_MPS = 1.943844;

        // Speed in meters per second
        double _speedInMPS {qQNaN()};
    };


    /*! \brief Convenience class for time computations
   *
   * This extremely simple class allows computation with times, without the need
   * to worry about units.
   */
    class Time {
    public:

        /*! \brief Constructs a time object from a time given in seconds */
        static Time fromS(double timeInS) {
            Time result;
            result._timeInS = timeInS;
            return result;
        }

        /*! \brief Checks if the time is valid */
        bool isFinite() const { return std::isfinite(_timeInS); }

        /*! \brief Checks if the time is negative */
        bool isNegative() const { return _timeInS < 0.0; }

        /*! \brief Adding times */
        Time& operator+=(const Time &other) {
            _timeInS += other._timeInS;
            return *this;
        }

        /*! \brief Returns time in seconds */
        double toS() const { return _timeInS; }

        /*! \brief Returns time in minutes */
        double toM() const { return _timeInS/Seconds_per_Minute; }

        /*! \brief Returns time in hours */
        double toH() const { return _timeInS/Seconds_per_Hour; }

        /*! \brief Returns time in hours and minutes, as a string of the form 7:12 or -0:05 */
        QString toHoursAndMinutes() const;

    private:
        static constexpr double Seconds_per_Minute = 60.0;
        static constexpr double Seconds_per_Hour   = 60.0*60.0;

        // Speed in meters per second
        double _timeInS {qQNaN()};
    };
};


/*! \brief Division of distance and speed gives time */
static AviationUnits::Time operator/ (const AviationUnits::Distance& dist, const AviationUnits::Speed &speed) {
    if ( (!dist.isFinite()) || (!speed.isFinite()) || (qFuzzyIsNull(speed.toMPS())) )
        return {};
    return AviationUnits::Time::fromS(dist.toM() / speed.toMPS());
}

#endif
