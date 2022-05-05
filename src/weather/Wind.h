/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "units/Angle.h"
#include "units/Speed.h"

namespace Weather {

/*! \brief This extremely simple class holds the wind speed and direction */

class Wind {
    Q_GADGET

public:

    //
    // Properties
    //

    /*! \brief Minimal wind speed that is considered valid */
    Q_PROPERTY(Units::Speed minWindSpeed MEMBER minWindSpeed CONSTANT)

    /*! \brief Maximal wind speed that is considered valid */
    Q_PROPERTY(Units::Speed maxWindSpeed MEMBER maxWindSpeed CONSTANT)

    /*! \brief Wind Direction
     *
     *  This property holds the wind direction. This is NaN if no value has been
     *  set.
     */
    Q_PROPERTY(Units::Angle directionFrom READ directionFrom WRITE setDirectionFrom)

    /*! \brief Wind Speed
     *
     *  This property holds the wind speed. This is a number that lies in the
     *  interval [minWindSpeed, maxWindSpeed] or NaN if the wind speed has not
     *  been set.
     */
    Q_PROPERTY(Units::Speed speed READ speed WRITE setSpeed)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property windDirection
     */
    [[nodiscard]] auto directionFrom() const -> Units::Angle { return m_directionFrom; }

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property windSpeed
     */
    [[nodiscard]] auto speed() const -> Units::Speed { return m_speed; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     *  If newWindSpeed is
     *  outside of the interval [minWindSpeed, maxWindSpeed], the property will
     *  be set to NaN.
     *
     *  @param newSpeed Property speed
     */
    void setSpeed(Units::Speed newSpeed);

    /*! \brief Setter function for property of the same name
     *
     *  @param newDirectionFrom Property directionFrom
     */
    void setDirectionFrom(Units::Angle newDirectionFrom);
    

    //
    // Methods
    //

    /*! \brief Equality check
     *
     *  @param other Wind that is compared to this
     *
     *  @result equality
     */
    Q_INVOKABLE bool operator==(Weather::Wind other) const;

private:
    static constexpr Units::Speed minWindSpeed = Units::Speed::fromKN(0.0);
    static constexpr Units::Speed maxWindSpeed = Units::Speed::fromKN(100.0);
    
    Units::Speed m_speed {};
    Units::Angle m_directionFrom {};
};

}

// Declare meta types
Q_DECLARE_METATYPE(Weather::Wind)
