/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

    /*! \brief Convenience class for distance computations
     *
     * This extremely simple class allows computation with distances, without the
     * need to worry about units. On construction, the distance is set to NaN.
     */
    class Distance {
        Q_GADGET
        QML_VALUE_TYPE(distance)

    public:
        /*! \brief Units of measurement for distances */
        enum DistanceUnit {
            Feet,
            Meter,
            Kilometer,
            StatuteMile,
            NauticalMile,
        };
        Q_ENUM(DistanceUnit)

        /*! \brief Constructs a distance
         *
         * @param distanceInM distance in meters
         *
         * @returns distance
         */
        Q_INVOKABLE static constexpr Units::Distance fromM(double distanceInM)
        {
            Distance result;
            result.m_distanceInM = distanceInM;
            return result;
        }

        /*! \brief Constructs a distance
         *
         * @param distanceInKM distance in kilometers
         *
         * @returns distance
         */
        Q_INVOKABLE static constexpr Units::Distance fromKM(double distanceInKM)
        {
            Distance result;
            result.m_distanceInM = 1000.0*distanceInKM;
            return result;
        }

        /*! \brief Constructs a distance
         *
         * @param distanceInNM distance in nautical miles
         *
         * @returns distance
         */
        Q_INVOKABLE static constexpr Units::Distance fromNM(double distanceInNM)
        {
            Distance result;
            result.m_distanceInM = distanceInNM*MetersPerNauticalMile;
            return result;
        }

        /*! \brief Constructs a distance
         *
         * @param distanceInMIL distance in international miles
         *
         * @returns distance
         */
        Q_INVOKABLE static constexpr Units::Distance fromMIL(double distanceInMIL)
        {
            Distance result;
            result.m_distanceInM = distanceInMIL*MetersPerMile;
            return result;
        }

        /*! \brief Constructs a distance
         *
         * @param distanceInFT distance in feet
         *
         * @returns distance
         */
        Q_INVOKABLE static constexpr Units::Distance fromFT(double distanceInFT)
        {
            Distance result;
            result.m_distanceInM = distanceInFT * MetersPerFeet;
            return result;
        }

        /*! \brief Constructs an invalid distance
         *
         * @returns Invalid distance
         */
        Q_INVOKABLE static constexpr Units::Distance nan()
        {
            return {};
        }

        /*! \brief Add distance to this distance
         *
         * @param other distance to be added
         *
         * @returns reference to this distance
         */
        Q_INVOKABLE Units::Distance& operator+=(Units::Distance other)
        {
            m_distanceInM += other.m_distanceInM;
            return *this;
        }

        /*! \brief Checks if the distance is valid
         *
         * @returns True is the distance is a finite number
         */
        [[nodiscard]] Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(m_distanceInM);
        }

        /*! \brief Checks if the distance is negative
         *
         * @returns True is the distance is negative
         */
        [[nodiscard]] Q_INVOKABLE bool isNegative() const
        {
            return m_distanceInM < 0.0;
        }

        /*! \brief Addition
         *
         *  @param rhs Right hand side of the addition
         *
         *  @returns Result of the addition
         */
        [[nodiscard]] Q_INVOKABLE Units::Distance operator+(Units::Distance rhs) const
        {
            return fromM(m_distanceInM + rhs.m_distanceInM);
        }

        /*! \brief Subtraction
         *
         *  @param rhs Right hand side of the subtraction
         *
         *  @returns Result of the subtraction
         */
        [[nodiscard]] Q_INVOKABLE Units::Distance operator-(Units::Distance rhs) const
        {
            return fromM(m_distanceInM - rhs.m_distanceInM);
        }

        /*! \brief Subtraction
         *
         *  @param rhs Right hand side of the subtraction
         *
         *  @returns Result of the subtraction
         */
        [[nodiscard]] Q_INVOKABLE Units::Distance subtract(Units::Distance rhs) const
        {
            return fromM(m_distanceInM - rhs.m_distanceInM);
        }

        /*! \brief Scaling
         *
         *  @param factor Factor of scaling
         *
         *  @returns Result of the scaling
         */
        [[nodiscard]] Q_INVOKABLE Units::Distance operator*(double factor) const
        {
            return fromM(m_distanceInM*factor);
        }

        /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        [[nodiscard]] Q_INVOKABLE std::partial_ordering operator<=>(const Units::Distance& rhs) const = default;

        /*! \brief Convert to feet
         *
         * @returns distance in feet
         */
        [[nodiscard]] Q_INVOKABLE double toFeet() const
        {
            return m_distanceInM / MetersPerFeet;
        }

        /*! \brief Convert to meters
         *
         * @returns distance in meters
         */
        [[nodiscard]] Q_INVOKABLE double toKM() const
        {
            return m_distanceInM / 1000.;
        }

        /*! \brief Convert to meters
         *
         * @returns distance in meters
         */
        [[nodiscard]] Q_INVOKABLE double toM() const
        {
            return m_distanceInM;
        }

        /*! \brief Convert to international miles
         *
         * @returns distance in international miles
         */
        [[nodiscard]] Q_INVOKABLE double toMIL() const
        {
            return m_distanceInM / MetersPerMile;
        }

        /*! \brief Convert to nautical miles
         *
         * @returns distance in nautical miles
         */
        [[nodiscard]] Q_INVOKABLE double toNM() const
        {
            return m_distanceInM / MetersPerNauticalMile;
        }

        /*! \brief Convert to string
         *
         *  This method converts the distance to string that is fit for
         *  human consumption, of the form "10.9 nm", "130 ft" or "3500 m".
         *  The distance is rounded to reasonable precision.
         *
         *  @param units Determines the units that are used.
         *
         *  @param roundBigNumbers If true, then round "1234" to "1200" and "117" to "120"
         *
         *  @param forceSign If 'true', then positive distances are prepended by a  "+" sign.
         *
         *  @returns A string that describes the distance, or an empty string if
         *  no reasonable distance is set.
         */
        [[nodiscard]] Q_INVOKABLE QString toString(Units::Distance::DistanceUnit units, bool roundBigNumbers=false, bool forceSign=false) const;

    private:
        static constexpr double MetersPerFeet = 0.3048;
        static constexpr double MetersPerNauticalMile = 1852;
        static constexpr double MetersPerMile = 1609.344;

        // Speed in meters per second
        double m_distanceInM{ NAN };
    };
} // namespace Units


/*! \brief Serialization */
QDataStream& operator<<(QDataStream& stream, const Units::Distance& distance);

/*! \brief Deserialization */
QDataStream& operator>>(QDataStream& stream, Units::Distance& distance);

// Declare meta types
Q_DECLARE_METATYPE(Units::Distance)
