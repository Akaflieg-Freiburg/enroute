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
    if (isValid())
    {
        interpretGeoData();
    }
}

FileFormats::GeoTIFF::GeoTIFF(QIODevice& device)
    : TIFF(device)
{
    if (isValid())
    {
        interpretGeoData();
    }
}


//
// Private Methods
//

void FileFormats::GeoTIFF::checkGeoKeySupport(const QMap<quint16, QVariantList>& TIFFFields)
{
    // Tag 34735 = GeoKeyDirectoryTag
    if (!TIFFFields.contains(34735))
    {
        throw QObject::tr("No GeoKeyDirectoryTag found — not a valid GeoTIFF.", "FileFormats::GeoTIFF");
    }

    auto values = TIFFFields.value(34735);

    // Header occupies the first 4 entries:
    // [0] KeyDirectoryVersion, [1] KeyRevision, [2] MinorRevision, [3] NumberOfKeys
    if (values.size() < 4)
    {
        throw QObject::tr("GeoKeyDirectoryTag is too short to be valid.", "FileFormats::GeoTIFF");
    }

    bool ok = false;
    auto numberOfKeys = values.at(3).toUInt(&ok);
    if (!ok)
    {
        throw QObject::tr("Invalid key count in GeoKeyDirectoryTag.", "FileFormats::GeoTIFF");
    }

    // Each key entry is 4 values: [KeyID, TIFFTagLocation, Count, Value_Offset]
    if (values.size() < static_cast<int>(4 + numberOfKeys * 4))
    {
        throw QObject::tr("GeoKeyDirectoryTag is truncated.", "FileFormats::GeoTIFF");
    }

    // Walk key entries looking for GTModelTypeGeoKey (1024)
    for (auto i = 0u; i < numberOfKeys; i++)
    {
        auto base     = 4 + static_cast<int>(i) * 4;
        auto keyID    = values.at(base + 0).toUInt(&ok);
        if (!ok)
        {
            continue;
        }
        auto location = values.at(base + 1).toUInt(&ok);
        if (!ok)
        {
            continue;
        }
        auto value    = values.at(base + 3).toUInt(&ok);
        if (!ok)
        {
            continue;
        }

        if (keyID != 1024)
        {
            continue;
        }

        // GTModelTypeGeoKey must be stored inline (location == 0)
        if (location != 0)
        {
            throw QObject::tr("GTModelTypeGeoKey is not stored inline — malformed GeoTIFF.", "FileFormats::GeoTIFF");
        }

        // ModelTypeGeographic (2) = lat/lon degrees — supported
        // ModelTypeProjected  (1) = Lambert, UTM, Mercator, etc. — not supported
        // ModelTypeGeocentric (3) = 3D Cartesian — not supported
        if (value == 1)
        {
            throw QObject::tr("This file uses an projected coordinate reference system, which is not supported by Enroute Flight Navigation. "
                              "Consult the manual for an explanation how to convert the file to geographic coordinates.",
                              "FileFormats::GeoTIFF");
        }
        if (value == 3)
        {
            throw QObject::tr("This file uses geocentric (3D Cartesian) coordinates, which are not supported by Enroute Flight Navigation. "
                              "Consult the manual for an explanation how to convert this file to geographic coordinates.",
                              "FileFormats::GeoTIFF");
        }
        if (value != 2)
        {
            throw QObject::tr("Unsupported coordinate system: unknown GTModelType %1.",
                              "FileFormats::GeoTIFF").arg(value);
        }

        return; // value == 2, geographic CRS — all good
    }

    // GTModelTypeGeoKey was not found at all — be permissive and let it through,
    // since some real-world GeoTIFFs omit it when the CRS is obvious from context.
}

QList<double> FileFormats::GeoTIFF::getTransformation(const QMap<quint16, QVariantList>& TIFFFields)
{
    auto transformation = readTransformation(TIFFFields);
    if (transformation.size() == 16)
    {
        return transformation;
    }

    auto tiepoints = readTiepoints(TIFFFields);
    if (tiepoints.empty())
    {
        return {};
    }
    const auto &tiepoint = tiepoints[0];

    auto pixelSize = readPixelSize(TIFFFields);
    if (!pixelSize.isValid())
    {
        return {};
    }

    return {
        pixelSize.width(), 0.0, 0.0, tiepoint.geoCoordinate.longitude() - tiepoint.rasterCoordinate.x()*pixelSize.width(),
        0.0, -pixelSize.height(), 0.0, tiepoint.geoCoordinate.latitude() + tiepoint.rasterCoordinate.y()*pixelSize.height(),
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
}

QList<FileFormats::GeoTIFF::Tiepoint> FileFormats::GeoTIFF::readTiepoints(const QMap<quint16, QVariantList>& TIFFFields)
{
    // Handle Tag 33922, compute top left of the bounding box
    if (!TIFFFields.contains(33922))
    {
        return {};
    }

    auto values = TIFFFields.value(33922);
    auto numTiepoints = values.size()/6;
    QVector<Tiepoint> tiepoints;
    tiepoints.reserve(numTiepoints);
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
        tiepoints.append({rasterPoint, coord});
    }

    return tiepoints;
}

QString FileFormats::GeoTIFF::readName(const QMap<quint16, QVariantList>& TIFFFields)
{
    // Handle Tag 270, name
    if (!TIFFFields.contains(270))
    {
        return {};
    }

    auto values = TIFFFields.value(270);
    if (values.isEmpty())
    {
        throw QObject::tr("No data for tag 270.", "FileFormats::GeoTIFF");
    }
    return values.constFirst().toString();
}

QSizeF FileFormats::GeoTIFF::readPixelSize(const QMap<quint16, QVariantList> &TIFFFields)
{
    if (!TIFFFields.contains(33550)) {
        return {};
    }

    // Handle Tag 33550, compute pixel width and height
    auto values = TIFFFields.value(33550);
    if (values.size() < 2)
    {
        throw QObject::tr("Invalid data for tag 33550.", "FileFormats::GeoTIFF");
    }

    bool globalOK = true;
    bool ok = false;
    auto pixelWidth = values.at(0).toDouble(&ok);
    globalOK = globalOK && ok;

    auto pixelHeight = values.at(1).toDouble(&ok);
    globalOK = globalOK && ok;

    if (!globalOK)
    {
        throw QObject::tr("Invalid data for tag 33550.", "FileFormats::GeoTIFF");
    }

    return {qAbs(pixelWidth), qAbs(pixelHeight)};
}

QList<double> FileFormats::GeoTIFF::readTransformation(const QMap<quint16, QVariantList> &TIFFFields)
{
    if (!TIFFFields.contains(34264)) {
        return {};
    }
    auto values = TIFFFields.value(34264);
    if (values.size() < 16)
    {
        throw QObject::tr("Invalid data for tag 34264.", "FileFormats::GeoTIFF");
    }

    QVector<double> transformation(16, NAN);
    bool globalOK = true;
    for(int i=0; i<16; i++)
    {
        bool ok = false;
        transformation[i] = values.at(i).toDouble(&ok);
        globalOK = globalOK && ok;
    }
    if (!globalOK)
    {
        throw QObject::tr("Invalid data for tag 34264.", "FileFormats::GeoTIFF");
    }

    return transformation;
}

void FileFormats::GeoTIFF::interpretGeoData()
{
    try
    {
        auto TIFFFields = fields();

        m_name = readName(TIFFFields);
        checkGeoKeySupport(TIFFFields);

        auto _rasterSize = rasterSize();
        if (!_rasterSize.isValid())
        {
            throw QObject::tr("No raster size data.", "FileFormats::GeoTIFF");
        }

        auto transformation = getTransformation(TIFFFields);
        if (transformation.size() != 16)
        {
            throw QObject::tr("No transformation data.", "FileFormats::GeoTIFF");
        }

        m_topLeft = {transformation[7],
                     transformation[3]};
        m_topRight = {transformation[4] * _rasterSize.width() + transformation[7],
                      transformation[0] * _rasterSize.width() + transformation[3]};
        m_bottomLeft = {transformation[5] * _rasterSize.height() + transformation[7],
                        transformation[1] * _rasterSize.height() + transformation[3]};
        m_bottomRight = {transformation[4] * _rasterSize.width() + transformation[5] * _rasterSize.height() + transformation[7],
                         transformation[0] * _rasterSize.width() + transformation[1] * _rasterSize.height() + transformation[3]};
        if (!m_topLeft.isValid()
            || !m_topRight.isValid()
            || !m_bottomLeft.isValid()
            || !m_bottomRight.isValid())
        {
            throw QObject::tr("Invalid coordinate data.", "FileFormats::GeoTIFF");
        }
    }
    catch (QString& message)
    {
        setError(message);
    }
}
