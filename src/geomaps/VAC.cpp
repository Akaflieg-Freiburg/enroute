/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QImage>

#include "geomaps/VAC.h"


//
// Methods
//

bool GeoMaps::VAC::isValid(const QString& fileName, QString* info)
{
    if (!readBBox(fileName).isValid())
    {
        return false;
    }
    QImage img(fileName);
    return !img.isNull();
}


QGeoRectangle GeoMaps::VAC::readBBox(const QString& fileName)
{
    if (fileName.size() < 5)
    {
        return {};
    }
    auto list = fileName.chopped(4).split('_');
    if (list.size() < 4)
    {
        return {};
    }
    list = list.last(4);

    QGeoCoordinate topLeft(list[1].toDouble(), list[0].toDouble());
    QGeoCoordinate bottomRight(list[3].toDouble(), list[2].toDouble());
    return QGeoRectangle(topLeft, bottomRight);
}
