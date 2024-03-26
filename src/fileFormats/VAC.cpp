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
#include <utility>

#include "fileFormats/GeoTIFF.h"
#include "fileFormats/VAC.h"

FileFormats::VAC::VAC()
{
    setError("Default constructed object, no data set");
}

FileFormats::VAC::VAC(const QString& fileName)
    : m_fileName(fileName)
{
    m_baseName = VAC::baseNameFromFileName(fileName);

    if (!coordsFromFileName())
    {
        FileFormats::GeoTIFF const geoTIFF(fileName);
        if (geoTIFF.isValid())
        {
            m_topLeft = geoTIFF.topLeft();
            m_topRight = geoTIFF.topRight();
            m_bottomLeft = geoTIFF.bottomLeft();
            m_bottomRight = geoTIFF.bottomRight();
            if (!geoTIFF.name().isEmpty())
            {
                m_baseName = geoTIFF.name();
            }
        }
    }

    // Generate errors and warnings
    generateErrorsAndWarnings();
}

FileFormats::VAC::VAC(const QString &fileName,
                      QGeoCoordinate topLeft,
                      QGeoCoordinate topRight,
                      QGeoCoordinate bottomLeft,
                      QGeoCoordinate bottomRight)
    : m_topLeft(std::move(topLeft))
    , m_topRight(std::move(topRight))
    , m_bottomLeft(std::move(bottomLeft))
    , m_bottomRight(std::move(bottomRight))
{
    m_baseName = VAC::baseNameFromFileName(fileName);

    // Generate errors and warnings
    generateErrorsAndWarnings();
}


//
// Methods
//

QGeoCoordinate FileFormats::VAC::center() const
{
    return {
        0.25*(m_topLeft.latitude()+m_topRight.latitude()+m_bottomLeft.latitude()+m_bottomRight.latitude()),
        0.25*(m_topLeft.longitude()+m_topRight.longitude()+m_bottomLeft.longitude()+m_bottomRight.longitude())
    };
}


#warning
/*
auto FileFormats::VAC::save(const QString& directory) -> QString
{
    if (!isValid())
    {
        return {};
    }

    if (!QDir().mkpath(directory))
    {
        return {};
    }

    auto topLeft = m_bBox.topLeft();
    auto bottomRight = m_bBox.bottomRight();
    auto newFileName = u"%1/%2-geo_%3_%4_%5_%6.webp"_qs
                           .arg(directory, m_baseName)
                           .arg(topLeft.longitude())
                           .arg(topLeft.latitude())
                           .arg(bottomRight.longitude())
                           .arg(bottomRight.latitude());

    QFile::remove(newFileName);
    if (m_fileName.endsWith(u"webp"_qs) &&
        QFile::exists(m_fileName) &&
        QFile::copy(m_fileName, newFileName))
    {
        return newFileName;
    }

    if (m_image.save(newFileName))
    {
        return newFileName;
    }
    return {};
}
*/
QString FileFormats::VAC::baseNameFromFileName(const QString& fileName)
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

bool FileFormats::VAC::coordsFromFileName()
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



//
// Private Methods
//

void FileFormats::VAC::generateErrorsAndWarnings()
{
    if (!m_topLeft.isValid() || !m_topRight.isValid()
        || !m_bottomLeft.isValid() || !m_bottomRight.isValid())
    {
        setError( QObject::tr("Unable to find georeferencing data for the file %1.", "VAC").arg(m_fileName) );
        return;
        }

#warning
    auto diameter_in_m = m_topLeft.distanceTo(m_bottomRight);
    if (diameter_in_m < 200)
    {
        addWarning( QObject::tr("The georeferencing data for the file %1 suggests that the image diagonal is less than 200m, which makes it unlikely that this is an approach chart.", "VAC").arg(m_fileName) );
    }
    if (diameter_in_m > 50000)
    {
        addWarning( QObject::tr("The georeferencing data for the file %1 suggests that the image diagonal is more than 50km, which makes it unlikely that this is an approach chart.", "VAC").arg(m_fileName) );
    }
}
