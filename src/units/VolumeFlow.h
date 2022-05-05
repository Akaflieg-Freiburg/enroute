/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

    /*! \brief Convenience class for volume flow computations
     *
     * This extremely simple class allows computation with volumes flows, without the
     * need to worry about units. On construction, the volume is set to NaN.
     */
    class VolumeFlow {
        Q_GADGET

    public:
        /*! \brief Constructs a volume flow
         *
         * @param volumeFlowInLPH volume flow in liters per hour
         *
         * @returns volume flow
         */
        Q_INVOKABLE static constexpr auto fromLPH(double volumeFlowInLPH) -> Units::VolumeFlow
        {
            VolumeFlow result;
            result.m_volumeFlowInLPH = volumeFlowInLPH;
            return result;
        }

        /*! \brief Constructs a volume flow
         *
         * @param volumeFlowInGPH volume in gallons per hour
         *
         * @returns volume flow
         */
        Q_INVOKABLE static constexpr auto fromGPH(double volumeFlowInGPH) -> Units::VolumeFlow
        {
            VolumeFlow result;
            result.m_volumeFlowInLPH = LitersPerGallon*volumeFlowInGPH;
            return result;
        }

        /*! \brief Checks if the volume is valid
         *
         * @returns True is the volume is a finite number
         */
        Q_INVOKABLE [[nodiscard]] auto isFinite() const -> bool
        {
            return std::isfinite(m_volumeFlowInLPH);
        }

        /*! \brief Comparison: less than
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator<(Units::VolumeFlow rhs) const
        {
            return m_volumeFlowInLPH < rhs.m_volumeFlowInLPH;
        }

        /*! \brief Comparison: larger equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator>(Units::VolumeFlow rhs) const
        {
            return m_volumeFlowInLPH > rhs.m_volumeFlowInLPH;
        }

        /*! \brief Comparison: not equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator!=(Units::VolumeFlow rhs) const
        {
            return m_volumeFlowInLPH != rhs.m_volumeFlowInLPH;
        }

        /*! \brief Comparison: equal
         *
         *  @param rhs Right hand side of the comparison
         *
         *  @returns Result of the comparison
         */
        Q_INVOKABLE auto operator==(Units::VolumeFlow rhs) const
        {
            return m_volumeFlowInLPH == rhs.m_volumeFlowInLPH;
        }

        /*! \brief Convert to liters per hour
         *
         * @returns volume flow in liters per hour
         */
        Q_INVOKABLE [[nodiscard]] auto toLPH() const -> double
        {
            return m_volumeFlowInLPH;
        }

        /*! \brief Convert to gallons per hour
         *
         * @returns volume flow in gallons per hour
         */
        Q_INVOKABLE [[nodiscard]] auto toGPH() const -> double
        {
            return m_volumeFlowInLPH/LitersPerGallon;
        }

    private:
        static constexpr double LitersPerGallon = 4.54609;

        // Speed in meters per second
        double m_volumeFlowInLPH{ NAN };
    };
};


// Declare meta types
Q_DECLARE_METATYPE(Units::VolumeFlow)
