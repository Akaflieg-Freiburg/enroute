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

#include <QDir>
#include <QFileInfo>
#include <QImage>

#include "fileFormats/GeoTIFF.h"
#include "geomaps/VAC.h"

GeoMaps::VAC::VAC(const QString& fileName)
    : m_fileName(fileName), m_image(fileName)
{
    m_bBox = VAC::bBoxFromFileName(fileName);
    if (!m_bBox.isValid())
    {
        FileFormats::GeoTIFF geoTIFF(fileName);
        if (geoTIFF.isValid())
        {
            m_bBox = geoTIFF.bBox();
        }
    }

    // Guess base name from file name
    m_baseName = VAC::baseNameFromFileName(fileName);

    // Generate errors and warnings
    generateErrorsAndWarnings();
}



//
// Methods
//

auto GeoMaps::VAC::isValid() const -> bool
{
    return m_bBox.isValid() && !m_image.isNull();
}

auto GeoMaps::VAC::save(const QString &directory) -> QString
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

QString GeoMaps::VAC::baseNameFromFileName(const QString& fileName)
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

QGeoRectangle GeoMaps::VAC::bBoxFromFileName(const QString& fileName)
{
    if (fileName.size() <= 5)
    {
        return {};
    }
    auto idx = fileName.lastIndexOf('.');
    if (idx == -1)
    {
        return {};
    }

    auto list = fileName.left(idx).split('_');
    if (list.size() < 4)
    {
        return {};
    }
    list = list.last(4);

    bool success = false;
    auto top = list[1].toDouble(&success);
    if (!success)
    {
        return {};
    }
    if ((top < -90.0) || (top > 90.0))
    {
        return {};
    }
    auto left = list[0].toDouble(&success);
    if (!success)
    {
        return {};
    }
    if ((left < -180.0) || (left > 180.0))
    {
        return {};
    }
    auto bottom = list[3].toDouble(&success);
    if (!success)
    {
        return {};
    }
    if ((bottom < -90.0) || (bottom > 90.0) || (bottom >= top))
    {
        return {};
    }
    auto right = list[2].toDouble(&success);
    if (!success)
    {
        return {};
    }
    if ((right < -180.0) || (right > 180.0) || (left >= right))
    {
        return {};
    }

    QGeoRectangle result;
    result.setTopLeft({top, left});
    result.setBottomRight({bottom, right});
    return result;
}



//
// Private Methods
//

void GeoMaps::VAC::generateErrorsAndWarnings()
{
    if (m_bBox.isValid())
    {
        m_error = QObject::tr("Unable to find georeferencing data for the file %1.", "VAC").arg(m_fileName);
        return;
    }
    if (m_image.isNull())
    {
        m_error = QObject::tr("Unable to load raster data from file %1.", "VAC").arg(m_fileName);
        return;
    }

    auto diameter_in_m = m_bBox.topLeft().distanceTo(m_bBox.bottomRight());
    if (diameter_in_m < 200)
    {
        m_warning = QObject::tr("The georeferencing data for the file %1 suggests that the image diagonal is less than 200m, which makes it unlikely that this is an approach chart.", "VAC").arg(m_fileName);
    }
    if (diameter_in_m > 50000)
    {
        m_warning = QObject::tr("The georeferencing data for the file %1 suggests that the image diagonal is more than 50km, which makes it unlikely that this is an approach chart.", "VAC").arg(m_fileName);
    }
}
