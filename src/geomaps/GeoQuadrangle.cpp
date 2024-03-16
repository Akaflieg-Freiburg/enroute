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


#include "geomaps/GeoQuadrangle.h"


[[nodiscard]] Units::Distance GeoMaps::GeoQuadrangle::diameter() const
{
    if (!isValid()) {
        return {};
    }

    auto diameter_in_m = m_coordinate_1.distanceTo(m_coordinate_2);
    diameter_in_m = qMax(diameter_in_m, m_coordinate_1.distanceTo(m_coordinate_3));
    diameter_in_m = qMax(diameter_in_m, m_coordinate_1.distanceTo(m_coordinate_4));
    diameter_in_m = qMax(diameter_in_m, m_coordinate_2.distanceTo(m_coordinate_3));
    diameter_in_m = qMax(diameter_in_m, m_coordinate_2.distanceTo(m_coordinate_4));
    diameter_in_m = qMax(diameter_in_m, m_coordinate_3.distanceTo(m_coordinate_4));

    return Units::Distance::fromM(diameter_in_m);
}


QDataStream& GeoMaps::operator<<(QDataStream& stream, const GeoMaps::GeoQuadrangle& quadrangle)
{
    stream << quadrangle.m_coordinate_1;
    stream << quadrangle.m_coordinate_2;
    stream << quadrangle.m_coordinate_3;
    stream << quadrangle.m_coordinate_4;
    return stream;
}

QDataStream& GeoMaps::operator>>(QDataStream& stream, GeoMaps::GeoQuadrangle& quadrangle)
{
    stream >> quadrangle.m_coordinate_1;
    stream >> quadrangle.m_coordinate_2;
    stream >> quadrangle.m_coordinate_3;
    stream >> quadrangle.m_coordinate_4;
    return stream;
}
