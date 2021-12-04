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


namespace Units {

    /*! \brief Convenience class for volume computations
     *
     * This extremely simple class allows computation with volumes, without the
     * need to worry about units. On construction, the volume is set to NaN.
     */
    class Volume {
        Q_GADGET

    public:
        /*! \brief Constructs a volume
         *
         * @param volumeInL volume in liters
         *
         * @returns volume
         */
        static constexpr Volume fromL(double volumeInL)
        {
            Volume result;
            result.m_volumeInL = volumeInL;
            return result;
        }

        /*! \brief Constructs a volume
         *
         * @param volumeInGAL volume in gallons
         *
         * @returns volume
         */
        static constexpr Volume fromGAL(double volumeInGAL)
        {
            Volume result;
            result.m_volumeInL = LitersPerGallon*volumeInGAL;
            return result;
        }

        /*! \brief Checks if the volume is valid
         *
         * @returns True is the volume is a finite number
         */
        Q_INVOKABLE bool isFinite() const
        {
            return std::isfinite(m_volumeInL);
        }

        /*! \brief Comparison: less than
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator<(Units::Volume rhs) const
        {
            return m_volumeInL < rhs.m_volumeInL;
        }

        /*! \brief Comparison: larger equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator>(Units::Volume rhs) const
        {
            return m_volumeInL > rhs.m_volumeInL;
        }

        /*! \brief Comparison: not equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator!=(Units::Volume rhs) const
        {
            return m_volumeInL != rhs.m_volumeInL;
        }

        /*! \brief Comparison: equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator==(Units::Volume rhs) const
        {
            return m_volumeInL == rhs.m_volumeInL;
        }

        /*! \brief Convert to liters
         *
         * @returns volume in liters
         */
        Q_INVOKABLE double toL() const
        {
            return m_volumeInL;
        }

        /*! \brief Convert to gallons
         *
         * @returns volume in gallons
         */
        Q_INVOKABLE double toGAL() const
        {
            return m_volumeInL/LitersPerGallon;
        }

    private:
        static constexpr double LitersPerGallon = 4.54609;

        // Speed in meters per second
        double m_volumeInL{ NAN };
    };
};


// Declare meta types
Q_DECLARE_METATYPE(Units::Volume)
