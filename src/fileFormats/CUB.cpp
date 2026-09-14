/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringDecoder>
#include <QtEndian>
#include <QtMath>

#include "fileFormats/CUB.h"
#include "fileFormats/DataFileAbstract.h"

#include <bit>
#include <cmath>

using namespace Qt::Literals::StringLiterals;


namespace {

// File identifier, stored little-endian in the first four bytes of every CUB file
constexpr quint32 cubFileIdentifier = 0x425543C2;

// Size of the CubHeader structure
constexpr qsizetype cubHeaderSize = 210;

// A CubItem must at least contain the fields up to and including PointsOffset
constexpr qint32 cubItemMinimumSize = 26;

// Offset of the ExtendedType field within a CubItem
constexpr qint32 cubItemExtendedTypeOffset = 42;


/* Bounds-checked reader for the binary file content.
 *
 * Per specification, floats are always little-endian while integers follow the
 * PcByteOrder flag of the header. All methods throw a QString with a
 * human-readable, translated error message when a read would go beyond the end
 * of the data.
 */
class CubReader
{
public:
    explicit CubReader(QByteArray data) : m_data(std::move(data)) {}

    void setLittleEndian(bool littleEndian) { m_littleEndian = littleEndian; }

    [[nodiscard]] qsizetype size() const { return m_data.size(); }

    [[nodiscard]] quint8 u8(qsizetype offset) const
    {
        checkRange(offset, 1);
        return static_cast<quint8>(m_data.at(offset));
    }

    [[nodiscard]] qint16 i16(qsizetype offset) const
    {
        checkRange(offset, 2);
        if (m_littleEndian)
        {
            return qFromLittleEndian<qint16>(m_data.constData() + offset);
        }
        return qFromBigEndian<qint16>(m_data.constData() + offset);
    }

    [[nodiscard]] qint32 i32(qsizetype offset) const
    {
        checkRange(offset, 4);
        if (m_littleEndian)
        {
            return qFromLittleEndian<qint32>(m_data.constData() + offset);
        }
        return qFromBigEndian<qint32>(m_data.constData() + offset);
    }

    [[nodiscard]] quint32 u32LE(qsizetype offset) const
    {
        checkRange(offset, 4);
        return qFromLittleEndian<quint32>(m_data.constData() + offset);
    }

    [[nodiscard]] float f32(qsizetype offset) const
    {
        return std::bit_cast<float>(u32LE(offset));
    }

    // Reads a string, UTF-8 with Latin-1 fallback, truncated at the first NUL byte
    [[nodiscard]] QString string(qsizetype offset, qsizetype length) const
    {
        checkRange(offset, length);
        auto bytes = m_data.mid(offset, length);
        const auto nulIndex = bytes.indexOf('\0');
        if (nulIndex >= 0)
        {
            bytes.truncate(nulIndex);
        }
        auto decoder = QStringDecoder(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
        QString result = decoder.decode(bytes);
        if (decoder.hasError())
        {
            result = QString::fromLatin1(bytes);
        }
        return result.simplified();
    }

private:
    void checkRange(qsizetype offset, qsizetype length) const
    {
        if ((offset < 0) || (length < 0) || (offset > m_data.size() - length))
        {
            throw QObject::tr("Attempt to read beyond the end of the file", "CUB");
        }
    }

    QByteArray m_data;
    bool m_littleEndian {true};
};


/* Maps CubStyle/CubClass/ExtendedType to the airspace categories understood by
 * this app. The category vocabulary is the one used by the OpenAir importer and
 * the map styling code: A–G, ATZ, CTR, DNG, FIR, FIS, GLD, P, R, RMZ, SUA,
 * TIA, TIZ, TMZ.
 */
QString category(quint8 type, quint8 extendedType)
{
    if (extendedType != 0)
    {
        switch (extendedType)
        {
        case 0x01: // Upper Info Region
            return u"FIR"_s;
        case 0x02: // Military Training Route
        case 0x07: // Military Training Area
        case 0x08: // Overflight Restriction
            return u"R"_s;
        default:
            return u"SUA"_s;
        }
    }

    const quint8 cubClass = (type >> 4) & 0x07;
    QString classLetter;
    if ((cubClass >= 1) && (cubClass <= 7))
    {
        classLetter = QString(QChar('A' + cubClass - 1));
    }

    switch (type & 0x8F)
    {
    case 0x00: // Unknown
        return classLetter.isEmpty() ? u"SUA"_s : classLetter;
    case 0x01: // Control Zone
        return u"CTR"_s;
    case 0x02: // Restricted Area
    case 0x05: // Temporary Reserved Area
    case 0x86: // Temporary Flight Restriction
    case 0x8E: // Temporary Segregated Area
        return u"R"_s;
    case 0x03: // Prohibited Area
        return u"P"_s;
    case 0x04: // Danger Area
    case 0x8D: // Alert
    case 0x8F: // Warning
        return u"DNG"_s;
    case 0x06: // Terminal Control Area
    case 0x09: // Control Area
        return classLetter.isEmpty() ? u"SUA"_s : classLetter;
    case 0x07: // Traffic Information Zone
        return u"TIZ"_s;
    case 0x84: // Traffic Information Area
        return u"TIA"_s;
    case 0x0A: // Glider Sector
        return u"GLD"_s;
    case 0x0B: // Transponder Mandatory Zone
        return u"TMZ"_s;
    case 0x0C: // Military Aerodrome Traffic Zone
    case 0x87: // Aerodrome Traffic Zone
        return u"ATZ"_s;
    case 0x0D: // Radio Mandatory Zone
    case 0x89: // Legacy, maps to RMZ
        return u"RMZ"_s;
    case 0x82: // Flight Information Region
    case 0x83: // Delegated FIR
        return u"FIR"_s;
    case 0x88: // Flight Information Service Area
        return u"FIS"_s;
    default:
        return u"SUA"_s;
    }
}


/* Renders an altitude bound as a string of the form used in this app's GeoJSON
 * files: "GND", "1500", "1500 AGL" (both in feet) or "FL 65". CUB stores
 * altitudes as integer meters; feet values are rounded to the nearest 10 ft to
 * undo the loss of precision incurred by that storage format.
 */
QString altitudeString(qint16 altMeters, quint8 altStyle)
{
    const double feet = altMeters / 0.3048;

    switch (altStyle)
    {
    case 1: // AGL
    {
        const int roundedFeet = qRound(feet / 10.0) * 10;
        if (roundedFeet == 0)
        {
            return u"GND"_s;
        }
        return u"%1 AGL"_s.arg(roundedFeet);
    }
    case 3: // Flight Level
        return u"FL %1"_s.arg(qRound(feet / 100.0));
    case 4: // Unlimited; has no equivalent in the GeoJSON vocabulary, FL 660 is
            // above the ceiling of all airspace systems worldwide
        return u"FL 660"_s;
    default: // MSL, also used as fallback for "Unknown" and "By NOTAM"
    {
        const int roundedFeet = qRound(feet / 10.0) * 10;
        if (roundedFeet == 0)
        {
            return u"GND"_s;
        }
        return QString::number(roundedFeet);
    }
    }
}


// Computes the SBO property (simplified bottom altitude in feet), following the
// conventions of the OpenAir importer
int simplifiedBottomAltitude(const QString& bottom)
{
    if (bottom.compare(u"GND"_s) == 0)
    {
        return 0;
    }
    if (bottom.startsWith(u"FL "_s))
    {
        bool ok = false;
        const int fl = bottom.sliced(3).toInt(&ok);
        return ok ? fl * 100 : 0;
    }
    bool ok = false;
    const int ft = bottom.split(u' ', Qt::SkipEmptyParts).first().toInt(&ok);
    return ok ? ft : 0;
}


/* Final cleanup of a polygon read from the file: close the ring if it is open,
 * delete doubled points, and make sure that the ring is oriented
 * counterclockwise, as required by the GeoJSON standard.
 */
void finalizePolygon(QVector<QGeoCoordinate>& polygon)
{
    if ((polygon.first().latitude() != polygon.last().latitude()) ||
        (polygon.first().longitude() != polygon.last().longitude()))
    {
        polygon.append(polygon.first());
    }

    for (auto i = polygon.size() - 1; i > 0; i--)
    {
        if ((polygon.at(i).latitude() == polygon.at(i - 1).latitude()) &&
            (polygon.at(i).longitude() == polygon.at(i - 1).longitude()))
        {
            polygon.removeAt(i);
        }
    }

    double area = 0;
    for (auto i = 0; i < polygon.size(); i++)
    {
        const auto j = (i + 1) % polygon.size();
        area += polygon.at(i).longitude() * polygon.at(j).latitude() - polygon.at(j).longitude() * polygon.at(i).latitude();
    }
    if (area < 0)
    {
        std::reverse(polygon.begin(), polygon.end());
    }
}

} // namespace


bool FileFormats::CUB::isValid(const QString& fileName, QString* info)
{
    QStringList errorList;
    QStringList warnings;
    auto json = parse(fileName, errorList, warnings);

    if (info != nullptr)
    {
        *info = {};
        if (!warnings.isEmpty())
        {
            *info += u"<p>"_s + QObject::tr("Warnings", "CUB") + u"</p>"_s;
            *info += u"<ul style='margin-left:-25px;'>"_s;
            foreach(auto warning, warnings)
            {
                *info += u"<li>"_s + warning + u"</li>"_s;
            }
            *info += u"</ul>"_s;
        }
    }

    return (!json.isEmpty()) && errorList.isEmpty();
}


QJsonDocument FileFormats::CUB::parse(const QString& fileName, QStringList& errorList, QStringList& warningList)
{
    auto inputFile = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!inputFile->open(QIODeviceBase::ReadOnly))
    {
        errorList << QObject::tr("Cannot open file %1", "CUB").arg(fileName);
        return {};
    }
    CubReader reader(inputFile->readAll());

    qint32 sizeOfItem = 0;
    qint32 sizeOfPoint = 0;
    qint32 numItems = 0;
    double loLaScale = 0;
    qint32 headerOffset = 0;
    qint32 dataOffset = 0;

    try
    {
        if (reader.size() < cubHeaderSize)
        {
            throw QObject::tr("File is too short to contain a valid header", "CUB");
        }
        if (reader.u32LE(0) != cubFileIdentifier)
        {
            throw QObject::tr("Not a CUB file, invalid file identifier", "CUB");
        }
        if (reader.u8(133) != 0)
        {
            throw QObject::tr("Encrypted CUB files are not supported", "CUB");
        }

        reader.setLittleEndian(reader.u8(132) != 0);

        sizeOfItem = reader.i32(154);
        sizeOfPoint = reader.i32(158);
        numItems = reader.i32(162);
        loLaScale = reader.f32(194);
        headerOffset = reader.i32(198);
        dataOffset = reader.i32(202);

        if ((sizeOfItem < cubItemMinimumSize) || (sizeOfPoint < 5))
        {
            throw QObject::tr("Invalid item or point size specification", "CUB");
        }
        if ((numItems <= 0) ||
            (headerOffset < 0) ||
            (dataOffset < 0) ||
            (headerOffset + static_cast<qint64>(numItems)*sizeOfItem > reader.size()))
        {
            throw QObject::tr("Invalid item table specification", "CUB");
        }
        if (!std::isfinite(loLaScale) || (loLaScale <= 0))
        {
            throw QObject::tr("Invalid coordinate scale specification", "CUB");
        }
    }
    catch (QString& ex)
    {
        errorList << ex;
        return {};
    }

    QJsonArray featureArray;
    for (qint32 i = 0; i < numItems; i++)
    {
        const qsizetype itemOffset = headerOffset + static_cast<qsizetype>(i)*sizeOfItem;
        QString name;

        try
        {
            const double itemLeft = reader.f32(itemOffset);
            const double itemBottom = reader.f32(itemOffset + 12);
            const quint8 type = reader.u8(itemOffset + 16);
            const quint8 altStyle = reader.u8(itemOffset + 17);
            const qint16 minAlt = reader.i16(itemOffset + 18);
            const qint16 maxAlt = reader.i16(itemOffset + 20);
            const qint32 pointsOffset = reader.i32(itemOffset + 22);
            quint8 extendedType = 0;
            if (sizeOfItem > cubItemExtendedTypeOffset)
            {
                extendedType = reader.u8(itemOffset + cubItemExtendedTypeOffset);
            }

            // Walk the point records associated with this item. The stream
            // starts with origin/point records and is terminated by the record
            // carrying the airspace name, which is followed by attribute
            // records that are not used here.
            double originX = itemLeft;
            double originY = itemBottom;
            QVector<QGeoCoordinate> polygon;
            qsizetype pos = static_cast<qsizetype>(dataOffset) + pointsOffset;
            bool done = false;
            while (!done)
            {
                const quint8 flag = reader.u8(pos);
                if (flag == 0x81)
                {
                    originX += reader.i16(pos + 1)*loLaScale;
                    originY += reader.i16(pos + 3)*loLaScale;
                    pos += sizeOfPoint;
                }
                else if (flag == 0x01)
                {
                    const double longitude = qRadiansToDegrees(originX + reader.i16(pos + 1)*loLaScale);
                    const double latitude = qRadiansToDegrees(originY + reader.i16(pos + 3)*loLaScale);
                    const QGeoCoordinate coordinate(latitude, longitude);
                    if (!coordinate.isValid())
                    {
                        throw QObject::tr("Invalid coordinate found", "CUB");
                    }
                    polygon.append(coordinate);
                    pos += sizeOfPoint;
                }
                else if ((flag & 0xC0) == 0x40)
                {
                    name = reader.string(pos + sizeOfPoint, flag & 0x3F);
                    done = true;
                }
                else
                {
                    done = true;
                }
            }

            if ((type & 0x8F) == 0x0F)
            {
                warningList.append(QObject::tr("Airspace %1 is defined by NOTAM; Airspace ignored.", "CUB").arg(name.isEmpty() ? QString::number(i + 1) : name));
                continue;
            }
            if (polygon.size() < 3)
            {
                throw QObject::tr("Airspace has no usable geometry", "CUB");
            }
            finalizePolygon(polygon);
            // GeoJSON requires closed rings with at least 4 positions. Rings that
            // collapse below that during cleanup would poison the whole map
            // rendering, so they must not be emitted.
            if (polygon.size() < 4)
            {
                throw QObject::tr("Airspace has no usable geometry", "CUB");
            }

            const QString bottom = altitudeString(minAlt, altStyle & 0x0F);
            const QString top = altitudeString(maxAlt, altStyle >> 4);

            QJsonObject propObj;
            propObj.insert(u"NAM"_s, name);
            propObj.insert(u"ID"_s, name);
            propObj.insert(u"CAT"_s, category(type, extendedType));
            propObj.insert(u"TYP"_s, u"AS"_s);
            propObj.insert(u"BOT"_s, bottom);
            propObj.insert(u"TOP"_s, top);
            propObj.insert(u"SBO"_s, simplifiedBottomAltitude(bottom));

            // Nesting is built with append() throughout: QJsonArray({x}) with a
            // single QJsonArray element resolves to the copy constructor on some
            // compilers (CWG 2137), silently flattening the ring nesting.
            QJsonArray coordArray;
            for (const auto& coordinate : polygon)
            {
                QJsonArray coord;
                coord.append(coordinate.longitude());
                coord.append(coordinate.latitude());
                coordArray.append(coord);
            }
            QJsonArray polygonArray;
            polygonArray.append(coordArray);

            QJsonObject geomObj;
            geomObj.insert(u"type"_s, u"Polygon"_s);
            geomObj.insert(u"coordinates"_s, polygonArray);

            QJsonObject featureObj;
            featureObj.insert(u"type"_s, u"Feature"_s);
            featureObj.insert(u"properties"_s, propObj);
            featureObj.insert(u"geometry"_s, geomObj);
            featureArray.append(featureObj);
        }
        catch (QString& ex)
        {
            warningList.append(QObject::tr("Error reading airspace %1: %2; Airspace ignored.", "CUB").arg(name.isEmpty() ? QString::number(i + 1) : name, ex));
        }
    }

    if (featureArray.isEmpty())
    {
        errorList << QObject::tr("No usable airspaces found in file %1", "CUB").arg(fileName);
        return {};
    }

    QJsonObject recObj;
    recObj.insert(u"type"_s, u"FeatureCollection"_s);
    recObj.insert(u"info"_s, fileName);
    recObj.insert(u"features"_s, featureArray);
    return QJsonDocument(recObj);
}
