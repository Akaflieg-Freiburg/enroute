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

#include <QFileInfo>
#include <QImage>

#include "geomaps/VAC.h"

GeoMaps::VAC::VAC(const QString& fileName) : m_fileName(fileName)
{
    // Guess boundary box from file name
    if (fileName.size() > 5)
    {
        auto idx = fileName.lastIndexOf('.');
        if (idx == -1)
        {
            idx = fileName.size();
        }

        auto list = fileName.left(idx).split('_');
        if (list.size() >= 4)
        {
            list = list.last(4);
            m_bBox.setTopLeft(QGeoCoordinate(list[1].toDouble(), list[0].toDouble()));
            m_bBox.setBottomRight(QGeoCoordinate(list[3].toDouble(), list[2].toDouble()));
        }
    }

    // Guess base name from file name
    QFileInfo fi(fileName);
    m_baseName = fi.fileName();
    auto idx = m_baseName.lastIndexOf(".");
    if (idx != -1)
    {
        m_baseName = m_baseName.left(idx);
    }
    idx = m_baseName.lastIndexOf("-geo_");
    if (idx != -1)
    {
        m_baseName = m_baseName.left(idx);
    }

}



//
// Methods
//

bool GeoMaps::VAC::isValid() const
{
    return m_bBox.isValid() && !QImage(m_fileName).isNull();
}
