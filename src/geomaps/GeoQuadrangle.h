/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include "units/Distance.h"


namespace GeoMaps {

/*! \brief Geographic Quadrangle.
 *
 * This extremely simple class defined a geographic quadrangle, by storing
 * an ordered list of QGeoCoordinates for the four edges.
 */

class GeoQuadrangle
{
    friend QDataStream& operator<<(QDataStream& stream, const GeoMaps::GeoQuadrangle& quadrangle);
    friend QDataStream& operator>>(QDataStream& stream, GeoMaps::GeoQuadrangle& quadrangle);

public:
    //
    // Getter Methods
    //

    /*! \brief Coordinate 1
     *
     *  @returns Coordinate 1
     */
    [[nodiscard]] QGeoCoordinate coordinate_1() const { return m_coordinate_1; }

    /*! \brief Coordinate 2
     *
     *  @returns Coordinate 2
     */
    [[nodiscard]] QGeoCoordinate coordinate_2() const { return m_coordinate_2; }

    /*! \brief Coordinate 3
     *
     *  @returns Coordinate 3
     */
    [[nodiscard]] QGeoCoordinate coordinate_3() const { return m_coordinate_3; }

    /*! \brief Coordinate 4
     *
     *  @returns Coordinate 4
     */
    [[nodiscard]] QGeoCoordinate coordinate_4() const { return m_coordinate_4; }



    //
    // Setter Methods
    //

    /*! \brief Setter function for coordinate 1
     *
     *  @param coord Coordinate
     */
    void setCoordinate_1(const QGeoCoordinate& coord) { m_coordinate_1 = coord; }

    /*! \brief Setter function for coordinate 2
     *
     *  @param coord Coordinate
     */
    void setCoordinate_2(const QGeoCoordinate& coord) { m_coordinate_2 = coord; }

    /*! \brief Setter function for coordinate 3
     *
     *  @param coord Coordinate
     */
    void setCoordinate_3(const QGeoCoordinate& coord) { m_coordinate_3 = coord; }

    /*! \brief Setter function for coordinate 4
     *
     *  @param coord Coordinate
     */
    void setCoordinate_4(const QGeoCoordinate& coord) { m_coordinate_4 = coord; }



    //
    // Methods
    //

    /*! \brief Validity
     *
     * @returns True if all four coordinates are valid
     */
    [[nodiscard]] bool isValid() const
    {
        return m_coordinate_1.isValid()
               && m_coordinate_2.isValid()
               && m_coordinate_3.isValid()
               && m_coordinate_4.isValid();
    }

    /*! \brief Diameter
     *
     * @returns The diameter if the GeoQuadrangle is valid, and NaN otherwise.
     */
    [[nodiscard]] Units::Distance diameter() const;


private:
    QGeoCoordinate m_coordinate_1 {};
    QGeoCoordinate m_coordinate_2 {};
    QGeoCoordinate m_coordinate_3 {};
    QGeoCoordinate m_coordinate_4 {};
};

/*! \brief Serialization */
QDataStream& operator<<(QDataStream& stream, const GeoMaps::GeoQuadrangle& quadrangle);

/*! \brief Deserialization */
QDataStream& operator>>(QDataStream& stream, GeoMaps::GeoQuadrangle& quadrangle);

} // namespace GeoMaps
