/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

    /*! \brief Convenience class for time computations
     *
     * This extremely simple class allows computation with times, without the need
     * to worry about units.
     */
class Timespan {
        Q_GADGET
        QML_VALUE_TYPE(time)

    public:        
        /*! \brief Constructs a time
         *
         * @param timeInH  time in hours
         *
         * @returns time
         */
        static auto fromH(double timeInH) -> Timespan {
            Timespan result;
            result.m_timeInS = timeInH*Seconds_per_Hour;
            return result;
        }

        /*! \brief Constructs a time
         *
         * @param timeInMS  time in milliseconds
         *
         * @returns time
         */
        static Timespan fromMS(double timeInMS)
        {
            Timespan result;
            result.m_timeInS = timeInMS/1000.0;
            return result;
        }

        /*! \brief Constructs a time
         *
         * @param timeInS  time in seconds
         *
         * @returns time
         */
        static auto fromS(double timeInS) -> Timespan {
            Timespan result;
            result.m_timeInS = timeInS;
            return result;
        }

        /*! \brief Checks if the time is valid
         *
         * @returns True is the time is a finite number
         */
        [[nodiscard]] Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(m_timeInS);
        }

        /*! \brief Checks if the time is negative
         *
         * @returns True is the time is negative
         */
        [[nodiscard]] Q_INVOKABLE bool isNegative() const
        {
            return m_timeInS < 0.0;
        }

        /*! \brief Add time to this time
         *
         * @param other time to be added
         *
         * @returns reference to this time
         */
        Q_INVOKABLE Units::Timespan& operator+=(Units::Timespan other)
        {
            m_timeInS += other.m_timeInS;
            return *this;
        }

        /*! \brief Comparison
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        [[nodiscard]] Q_INVOKABLE std::partial_ordering operator<=>(const Units::Timespan& rhs) const = default;

        /*! \brief Convert time to seconds
         *
         * @return time in seconds
         */
        [[nodiscard]] Q_INVOKABLE double toS() const
        {
            return m_timeInS;
        }

        /*! \brief Convert time to minutes
         *
         * @return time in minutes
         */
        [[nodiscard]] Q_INVOKABLE double toM() const
        {
            return m_timeInS / Seconds_per_Minute;
        }

        /*! \brief Convert time to hours
         *
         * @return time in hours
         */
        [[nodiscard]] Q_INVOKABLE double toH() const
        {
            return m_timeInS / Seconds_per_Hour;
        }

        /*! \brief Convert time to string
         *
         * @returns time in hours and minutes, as a string of the form 7:12 or -0:05
         */
        [[nodiscard]] Q_INVOKABLE QString toHoursAndMinutes() const;

    private:
        static constexpr double Seconds_per_Minute = 60.0;
        static constexpr double Seconds_per_Hour = 60.0 * 60.0;

        // Speed in meters per second
        double m_timeInS{qQNaN()};
    };
} // namespace Units


// Declare meta types
Q_DECLARE_METATYPE(Units::Timespan)
