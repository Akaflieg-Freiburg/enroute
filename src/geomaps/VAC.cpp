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
#include "geomaps/VAC.h"

GeoMaps::VAC::VAC(const QString& fName)
    : fileName(fName)
{
    // Check if file is a GeoTIFF file. If so, extract information from there.
    FileFormats::GeoTIFF const geoTIFF(fileName);
    if (geoTIFF.isValid())
    {
        topLeft = geoTIFF.topLeft();
        topRight = geoTIFF.topRight();
        bottomLeft = geoTIFF.bottomLeft();
        bottomRight = geoTIFF.bottomRight();
        if (!geoTIFF.name().isEmpty())
        {
            name = geoTIFF.name();
        }
    }

    // If no baseName is known, try to extract a base name from the file name
    if (name.isEmpty())
    {
        name = VAC::getNameFromFileName(fileName);
    }

    // If coordinates are not valid, try to extract coordinates from the file name
    if (!hasValidCoordinates())
    {
        getCoordsFromFileName();
    }
}

GeoMaps::VAC::VAC(const QString& fName,
                      const QGeoCoordinate& topLeft,
                      const QGeoCoordinate& topRight,
                      const QGeoCoordinate& bottomLeft,
                      const QGeoCoordinate& bottomRight)
    : fileName(fName),
    topLeft(topLeft), topRight(topRight), bottomLeft(bottomLeft), bottomRight(bottomRight)
{
    // Check if file is a GeoTIFF file. If so, extract information from there.
    FileFormats::GeoTIFF const geoTIFF(fileName);
    if (geoTIFF.isValid() && !geoTIFF.name().isEmpty())
    {
        name = geoTIFF.name();
    }

    // If no baseName is known, try to extract a base name from the file name
    if (name.isEmpty())
    {
        name = VAC::getNameFromFileName(fileName);
    }
}


//
// Methods
//

QString GeoMaps::VAC::description() const
{
    QFileInfo const fileInfo(fileName);
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

QString GeoMaps::VAC::infoText() const
{
    auto displayText = QObject::tr("manually imported", "VAC");
    QFileInfo const info(fileName);
    if (info.exists())
    {
        displayText += " • " + QLocale::system().formattedDataSize(info.size(), 1, QLocale::DataSizeSIFormat);
    }
    return displayText;
}

QGeoCoordinate GeoMaps::VAC::center() const
{
    return {0.25*(topLeft.latitude()+topRight.latitude()+bottomLeft.latitude()+bottomRight.latitude()),
            0.25*(topLeft.longitude()+topRight.longitude()+bottomLeft.longitude()+bottomRight.longitude())};
}


QString GeoMaps::VAC::getNameFromFileName(const QString& fileName)
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

bool GeoMaps::VAC::isValid() const
{
    return hasValidCoordinates()
           && QFile::exists(fileName)
           && !name.isEmpty();
}


bool GeoMaps::VAC::hasValidCoordinates() const
{
    return topLeft.isValid()
           && topRight.isValid()
           && bottomLeft.isValid()
           && bottomRight.isValid();
}



bool GeoMaps::VAC::getCoordsFromFileName()
{
#warning too complicated

    if (fileName.size() <= 5)
    {
        return false;
    }
    auto idx = fileName.lastIndexOf('.');
    if (idx == -1)
    {
        return false;
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

    topLeft = {top, left};
    topRight = {top, right};
    bottomLeft = {bottom, left};
    bottomRight = {bottom, right};

    return topLeft.isValid() && topRight.isValid()
           && bottomLeft.isValid() && bottomRight.isValid();
}



QDataStream& GeoMaps::operator<<(QDataStream& stream, const GeoMaps::VAC& vac)
{
    return stream
           << vac.fileName
           << vac.name
           << vac.topLeft
           << vac.topRight
           << vac.bottomLeft
           << vac.bottomRight;
}

QDataStream& GeoMaps::operator>>(QDataStream& stream, GeoMaps::VAC& vac)
{
    return stream
           >> vac.fileName
           >> vac.name
           >> vac.topLeft
           >> vac.topRight
           >> vac.bottomLeft
           >> vac.bottomRight;
}

