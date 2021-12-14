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

#include "units/Distance.h"
#include "units/Speed.h"

#include <QtMath>
#include <QObject>


namespace Units {

    /*! \brief Convenience class for time computations
     *
     * This extremely simple class allows computation with times, without the need
     * to worry about units.
     */
    class Time {
        Q_GADGET

    public:
        /*! \brief Constructs a time
         *
         * @param timeInMS  time in milliseconds
         *
         * @returns time
         */
        static Time fromMS(double timeInMS) {
            Time result;
            result._timeInS = timeInMS/1000.0;
            return result;
        }

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
         * @returns True is the time is a finite number
         */
        Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(_timeInS);
        }

        /*! \brief Checks if the time is negative
         *
         * @returns True is the time is negative
         */
        Q_INVOKABLE bool isNegative() const
        {
            return _timeInS < 0.0;
        }

        /*! \brief Add time to this time
         *
         * @param other time to be added
         *
         * @returns reference to this time
         */
        Q_INVOKABLE Units::Time &operator+=(Units::Time other)
        {
            _timeInS += other._timeInS;
            return *this;
        }

        /*! \brief Convert time to seconds
         *
         * @return time in seconds
         */
        Q_INVOKABLE double toS() const
        {
            return _timeInS;
        }

        /*! \brief Convert time to minutes
         *
         * @return time in minutes
         */
        Q_INVOKABLE double toM() const
        {
            return _timeInS / Seconds_per_Minute;
        }

        /*! \brief Convert time to hours
         *
         * @return time in hours
         */
        Q_INVOKABLE double toH() const
        {
            return _timeInS / Seconds_per_Hour;
        }

        /*! \brief Convert time to string
         *
         * @returns time in hours and minutes, as a string of the form 7:12 or -0:05
         */
        Q_INVOKABLE QString toHoursAndMinutes() const;

    private:
        static constexpr double Seconds_per_Minute = 60.0;
        static constexpr double Seconds_per_Hour = 60.0 * 60.0;

        // Speed in meters per second
        double _timeInS{qQNaN()};
    };
};


// Declare meta types
Q_DECLARE_METATYPE(Units::Time)
