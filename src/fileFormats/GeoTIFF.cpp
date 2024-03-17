/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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

#include "GeoTIFF.h"



//
// Constructors
//

FileFormats::GeoTIFF::GeoTIFF(const QString& fileName)
    : TIFF(fileName)
{
    interpretGeoData();
}

FileFormats::GeoTIFF::GeoTIFF(QIODevice& device)
    : TIFF(device)
{
    interpretGeoData();
}



//
// Private Methods
//

void FileFormats::GeoTIFF::readGeoTiepoints(QMap<quint16, QVariantList>& TIFFFields)
{
    // Handle Tag 33922, compute top left of the bounding box
    if (TIFFFields.contains(33922))
    {
        auto values = TIFFFields.value(33922);
        auto numTiepoints = values.size()/6;
        for(auto numTiepoint = 0; numTiepoint < numTiepoints; numTiepoint++)
        {
            bool ok = false;
            auto x  = TIFFFields.value(33922).at(0+6*numTiepoint).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            auto y  = TIFFFields.value(33922).at(1+6*numTiepoint).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            QPointF const rasterPoint(x,y);

            auto lat = TIFFFields.value(33922).at(4+6*numTiepoint).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            auto lon = TIFFFields.value(33922).at(3+6*numTiepoint).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            QGeoCoordinate const coord(lat, lon);
            if (!coord.isValid())
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            m_tiepoints.append({rasterPoint, coord});
        }

        if (m_tiepoints.empty())
        {
            throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
        }
        m_bBox.setTopLeft(m_tiepoints[0].geoCoordinate);
    }
    else
    {
        throw QObject::tr("Tag 33922 is not set.", "FileFormats::GeoTIFF");
    }
}


void FileFormats::GeoTIFF::interpretGeoData()
{
    auto TIFFFields = fields();

    // Handle Tag 270, name
    if (TIFFFields.contains(270))
    {
        auto values = TIFFFields.value(270);
        if (values.isEmpty())
        {
            throw QObject::tr("No data for tag 270.", "FileFormats::GeoTIFF");
        }
        m_name = values.constFirst().toString();
    }

    readGeoTiepoints(TIFFFields);

    // Handle Tag 33550, compute pixel width and height
    double pixelWidth = NAN;
    double pixelHeight = NAN;
    {
        if (TIFFFields.contains(33550))
        {
            auto values = TIFFFields.value(33550);
            if (values.size() < 2)
            {
                throw QObject::tr("Invalid data for tag 33550.", "FileFormats::GeoTIFF");
            }
            bool ok = false;
            pixelWidth = values.at(0).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33550.", "FileFormats::GeoTIFF");
            }
            pixelHeight = values.at(1).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33550.", "FileFormats::GeoTIFF");
            }
        }
        else
        {
            throw QObject::tr("Tag 33550 is not set.", "FileFormats::GeoTIFF");
        }
    }

    auto width = rasterSize().width();
    auto height = rasterSize().height();

    // Computer bottom right of bounding box
    QGeoCoordinate coord = m_bBox.topLeft();
    coord.setLongitude(coord.longitude() + (width-1)*pixelWidth);
    if (pixelHeight > 0)
    {
        coord.setLatitude(coord.latitude() - (height-1)*pixelHeight);
    }
    else
    {
        coord.setLatitude(coord.latitude() + (height-1)*pixelHeight);
    }
    m_bBox.setBottomRight(coord);
    if (!m_bBox.isValid())
    {
        throw QObject::tr("The bounding box is invalid.", "FileFormats::GeoTIFF");
    }
}
