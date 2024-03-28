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

#include <QDir>
#include <QFileInfo>
#include <QImage>

#include "fileFormats/GeoTIFF.h"
#include "fileFormats/VAC.h"

FileFormats::VAC::VAC(const QString& fileName)
    : m_fileName(fileName)
{
    // Check if file is a GeoTIFF file. If so, extract information from there.
    FileFormats::GeoTIFF const geoTIFF(fileName);
    if (geoTIFF.isValid())
    {
        m_topLeft = geoTIFF.topLeft();
        m_topRight = geoTIFF.topRight();
        m_bottomLeft = geoTIFF.bottomLeft();
        m_bottomRight = geoTIFF.bottomRight();
        if (!geoTIFF.name().isEmpty())
        {
            m_name = geoTIFF.name();
        }
    }

    // If no baseName is known, try to extract a base name from the file name
    if (m_name.isEmpty())
    {
        m_name = VAC::getNameFromFileName(fileName);
    }

    // If coordinates are not valid, try to extract coordinates from the file name
    if (!hasValidCoordinates())
    {
        getCoordsFromFileName();
    }
}

FileFormats::VAC::VAC(const QString& fileName,
                      const QGeoCoordinate& topLeft,
                      const QGeoCoordinate& topRight,
                      const QGeoCoordinate& bottomLeft,
                      const QGeoCoordinate& bottomRight)
    : m_fileName(fileName),
      m_topLeft(topLeft), m_topRight(topRight), m_bottomLeft(bottomLeft), m_bottomRight(bottomRight)
{
    // Check if file is a GeoTIFF file. If so, extract information from there.
    FileFormats::GeoTIFF const geoTIFF(fileName);
    if (geoTIFF.isValid() && !geoTIFF.name().isEmpty())
    {
        m_name = geoTIFF.name();
    }

    // If no baseName is known, try to extract a base name from the file name
    if (m_name.isEmpty())
    {
        m_name = VAC::getNameFromFileName(fileName);
    }
}


//
// Methods
//

QString FileFormats::VAC::description() const
{
    QFileInfo const fileInfo(m_fileName);
    if (!fileInfo.exists())
    {
        return QObject::tr("No information available.", "VAC");
    }
    return QStringLiteral("<table><tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr><tr><td><strong>%3 :&nbsp;&nbsp;</strong></td><td>%4</td></tr></table>")
        .arg(QObject::tr("Installed", "VAC"),
             fileInfo.lastModified().toUTC().toString(),
             QObject::tr("File Size", "VAC"),
             QLocale::system().formattedDataSize(fileInfo.size(), 1, QLocale::DataSizeSIFormat));




}

QString FileFormats::VAC::infoText() const
{
    auto displayText = QObject::tr("manually imported", "VAC");
    QFileInfo const info(m_fileName);
    if (info.exists())
    {
        displayText += " • " + QLocale::system().formattedDataSize(info.size(), 1, QLocale::DataSizeSIFormat);
    }
    return displayText;
}

QGeoCoordinate FileFormats::VAC::center() const
{
    return {0.25*(m_topLeft.latitude()+m_topRight.latitude()+m_bottomLeft.latitude()+m_bottomRight.latitude()),
            0.25*(m_topLeft.longitude()+m_topRight.longitude()+m_bottomLeft.longitude()+m_bottomRight.longitude())};
}


QString FileFormats::VAC::getNameFromFileName(const QString& fileName)
{
    QFileInfo const fileInfo(fileName);
    auto baseName = fileInfo.fileName();
    auto idx = baseName.lastIndexOf(u"."_qs);
    if (idx != -1)
    {
        baseName = baseName.left(idx);
    }
    idx = baseName.lastIndexOf(u"-geo_"_qs);
    if (idx != -1)
    {
        baseName = baseName.left(idx);
    }
    return baseName;
}

bool FileFormats::VAC::isValid() const
{
    return hasValidCoordinates()
           && QFile::exists(m_fileName)
           && !m_name.isEmpty();
}


bool FileFormats::VAC::hasValidCoordinates() const
{
    return m_topLeft.isValid()
           && m_topRight.isValid()
           && m_bottomLeft.isValid()
           && m_bottomRight.isValid();
}



bool FileFormats::VAC::getCoordsFromFileName()
{
#warning too complicated

    if (m_fileName.size() <= 5)
    {
        return false;
    }
    auto idx = m_fileName.lastIndexOf('.');
    if (idx == -1)
    {
        return false;
    }

    auto list = m_fileName.left(idx).split('_');
    if (list.size() < 4)
    {
        return {};
    }
    list = list.last(4);

    bool success = false;
    auto top = list[1].toDouble(&success);
    if (!success)
    {
        return false;
    }
    if ((top < -90.0) || (top > 90.0))
    {
        return false;
    }
    auto left = list[0].toDouble(&success);
    if (!success)
    {
        return false;
    }
    if ((left < -180.0) || (left > 180.0))
    {
        return false;
    }
    auto bottom = list[3].toDouble(&success);
    if (!success)
    {
        return false;
    }
    if ((bottom < -90.0) || (bottom > 90.0) || (bottom >= top))
    {
        return false;
    }
    auto right = list[2].toDouble(&success);
    if (!success)
    {
        return false;
    }
    if ((right < -180.0) || (right > 180.0) || (left >= right))
    {
        return false;
    }

    m_topLeft = {top, left};
    m_topRight = {top, right};
    m_bottomLeft = {bottom, left};
    m_bottomRight = {bottom, right};

    return m_topLeft.isValid() && m_topRight.isValid()
           && m_bottomLeft.isValid() && m_bottomRight.isValid();
}

