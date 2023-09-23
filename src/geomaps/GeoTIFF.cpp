

/****************************************************************************
 *
 * This file contains code based on Debao Zhang's QtTiffTagViewer
 *
 * https://github.com/dbzhang800/QtTiffTagViewer
 *
 * The code for QtTiffTagViewer is licensed as follows:
 *
 * Copyright (c) 2018 Debao Zhang <hello@debao.me>
 * All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/


#include <QImage>
#include <QScopedPointer>
#include <QVector>
#include <QVariant>
#include <QExplicitlySharedDataPointer>
#include "GeoTIFF.h"
#include <QFile>
#include <QLoggingCategory>
#include <QtEndian>
#include <QSharedData>
#include <algorithm>
#include <QGeoRectangle>

class QByteArray;
class TiffFile;
class TiffIfd;
class TiffIfdEntry;

enum Tiff_ByteOrder { LittleEndian, BigEndian };


namespace  {

template<typename T> inline auto getValueFromBytes(const char *bytes, Tiff_ByteOrder byteOrder) -> T
{
    if (byteOrder == LittleEndian)
    {
        return qFromLittleEndian<T>(reinterpret_cast<const uchar *>(bytes));
    }
    return qFromBigEndian<T>(reinterpret_cast<const uchar *>(bytes));
}

template<typename T> inline auto fixValueByteOrder(T value, Tiff_ByteOrder byteOrder) -> T
{
    if (byteOrder == LittleEndian)
    {
        return qFromLittleEndian<T>(value);
    }
    return qFromBigEndian<T>(value);
}

}



QString err_256_not_set = QString::fromLatin1("Tag 256 is not set");
QString err_257_not_set = QString::fromLatin1("Tag 257 is not set");
QString err_33550_not_set = QString::fromLatin1("Tag 33550 is not set");
QString err_33922_not_set = QString::fromLatin1("Tag 33922 is not set");
QString err_file_read = QString::fromLatin1("File read error");
QString err_seek_pos = QString::fromLatin1("Fail to seek pos: ");


class TiffIfdEntry
{
public:
    enum DataType {
        DT_Byte = 1,
        DT_Ascii,
        DT_Short,
        DT_Long,
        DT_Rational,
        DT_SByte,
        DT_Undefined,
        DT_SShort,
        DT_SLong,
        DT_SRational,
        DT_Float,
        DT_Double,
        DT_Ifd,
        DT_Long8,
        DT_SLong8,
        DT_Ifd8
    };

private:
    friend class TiffFile;

    [[nodiscard]] auto typeSize() const -> int
    {
        switch (m_type) {
        case TiffIfdEntry::DT_Byte:
        case TiffIfdEntry::DT_SByte:
        case TiffIfdEntry::DT_Ascii:
        case TiffIfdEntry::DT_Undefined:
            return 1;
        case TiffIfdEntry::DT_Short:
        case TiffIfdEntry::DT_SShort:
            return 2;
        case TiffIfdEntry::DT_Long:
        case TiffIfdEntry::DT_SLong:
        case TiffIfdEntry::DT_Ifd:
        case TiffIfdEntry::DT_Float:
            return 4;

        case TiffIfdEntry::DT_Rational:
        case TiffIfdEntry::DT_SRational:
        case TiffIfdEntry::DT_Long8:
        case TiffIfdEntry::DT_SLong8:
        case TiffIfdEntry::DT_Ifd8:
        case TiffIfdEntry::DT_Double:
            return 8;
        default:
            return 0;
        }
    }

    void parserValues(const char *bytes, Tiff_ByteOrder byteOrder)
    {
        // To make things simple, save normal integer as qint32 or quint32 here.
        for (auto i = 0; i < m_count; ++i)
        {
            switch (m_type)
            {
            case TiffIfdEntry::DT_Short:
                m_values.append(static_cast<quint32>(getValueFromBytes<quint16>(bytes + i * 2, byteOrder)));
                break;
            case TiffIfdEntry::DT_Double:
                double f;
                if (byteOrder == BigEndian)
                {
                    char rbytes[8];
                    rbytes[0] = bytes[i*8+7];
                    rbytes[1] = bytes[i*8+6];
                    rbytes[2] = bytes[i*8+5];
                    rbytes[3] = bytes[i*8+4];
                    rbytes[4] = bytes[i*8+3];
                    rbytes[5] = bytes[i*8+2];
                    rbytes[6] = bytes[i*8+1];
                    rbytes[7] = bytes[i*8];
                    memcpy( &f, rbytes, 8 );
                }
                else
                {
                    memcpy( &f, bytes + i * 8, 8 );
                }
                m_values.append(f);
                break;
            default:
                break;
            }
        }
    }

    quint16 m_tag;
    quint16 m_type;
    quint64 m_count{ 0 };
    QByteArray m_valueOrOffset; // 12 bytes for tiff or 20 bytes for bigTiff
    QVariantList m_values;
};

class TiffIfd
{
    friend class TiffFile;

    QVector<TiffIfdEntry> m_ifdEntries;
    qint64 m_nextIfdOffset{ 0 };
};

class TiffFile
{
public:
    TiffFile(const QString &filePath);

    [[nodiscard]] auto getRect() const -> QGeoRectangle;


private:
    void setError(const QString &errorString);
    auto readHeader() -> bool;
    auto readIfd(qint64 offset, TiffIfd *parentIfd = nullptr) -> bool;

    template<typename T> auto getValueFromFile() -> T
    {
        T v{ 0 };
        auto bytesRead = m_file.read(reinterpret_cast<char *>(&v), sizeof(T));
        if (bytesRead != sizeof(T))
        {
            throw err_file_read;
        }
        return fixValueByteOrder(v, m_header.byteOrder);
    }

    struct Header
    {
        QByteArray rawBytes;
        Tiff_ByteOrder byteOrder{ LittleEndian };
        quint16 version{ 42 };
        qint64 ifd0Offset{ 0 };

        [[nodiscard]] auto isBigTiff() const -> bool { return version == 43; }
    } m_header;

    struct Geo
    {
        quint16 width = 0;
        quint16 height = 0;
        double longitute = 0;
        double latitude = 0;
        double pixelWidth = 0;
        double pixelHeight = 0;
    } m_geo;

    QVector<TiffIfd> m_ifds;

    QFile m_file;
    QString m_errorString;
    bool m_hasError {false};
};


auto GeoMaps::GeoTIFF::readCoordinates(const QString& path) -> QGeoRectangle
{
    TiffFile const tiff(path);

    try
    {
        return tiff.getRect();
    }
    catch (QString& ex)
    {
        qWarning() << " " << ex;
    }

    return {}; // Return a default-constructed (hence invalid) QGeoRectangle
}





/*!
 * \class TiffIfd
 */



void TiffFile::setError(const QString &errorString)
{
    m_hasError = true;
    m_errorString = errorString;
}


auto TiffFile::readHeader() -> bool
{
    auto headerBytes = m_file.peek(8);
    if (headerBytes.size() != 8)
    {
        setError(QStringLiteral("Invalid tiff file"));
        return false;
    }

    // magic bytes
    auto magicBytes = headerBytes.left(2);
    if (magicBytes == QByteArray("II"))
    {
        m_header.byteOrder = LittleEndian;
    }
    else if (magicBytes == QByteArray("MM"))
    {
        m_header.byteOrder = BigEndian;
    }
    else
    {
        setError(QStringLiteral("Invalid tiff file"));
        return false;
    }

    // version
    m_header.version = getValueFromBytes<quint16>(headerBytes.data() + 2, m_header.byteOrder);
    if (m_header.version != 42 && m_header.version != 43)
    {
        setError(QStringLiteral("Invalid tiff file: Unknown version"));
        return false;
    }
    m_header.rawBytes = m_file.read(m_header.isBigTiff() ? 16 : 8);

    // ifd0Offset
    if (!m_header.isBigTiff())
    {
        m_header.ifd0Offset =
            getValueFromBytes<quint32>(m_header.rawBytes.data() + 4, m_header.byteOrder);
    }
    else
    {
        m_header.ifd0Offset = getValueFromBytes<qint64>(m_header.rawBytes.data() + 8, m_header.byteOrder);
    }

    return true;
}

auto TiffFile::readIfd(qint64 offset, TiffIfd * /*parentIfd*/) -> bool
{
    if (!m_file.seek(offset))
    {
        setError(m_file.errorString());
        return false;
    }

    TiffIfd ifd;

    if (!m_header.isBigTiff())
    {
        auto const deCount = getValueFromFile<quint16>();
        for (int i = 0; i < deCount; ++i)
        {
            TiffIfdEntry ifdEntry;
            auto &dePrivate = ifdEntry;
            dePrivate.m_tag = getValueFromFile<quint16>();
            dePrivate.m_type = getValueFromFile<quint16>();
            dePrivate.m_count = getValueFromFile<quint32>();
            dePrivate.m_valueOrOffset = m_file.read(4);
            if ((dePrivate.m_tag == 256) || (dePrivate.m_tag == 257) || (dePrivate.m_tag == 33550) || (dePrivate.m_tag == 33922))
            {
                ifd.m_ifdEntries.append(ifdEntry);
            }
        }
        ifd.m_nextIfdOffset = getValueFromFile<quint32>();
    }
    else
    {
        auto const deCount = getValueFromFile<quint64>();
        for (quint64 i = 0; i < deCount; ++i)
        {
            TiffIfdEntry ifdEntry;
            auto &dePrivate = ifdEntry;
            dePrivate.m_tag = getValueFromFile<quint16>();
            dePrivate.m_type = getValueFromFile<quint16>();
            dePrivate.m_count = getValueFromFile<quint64>();
            dePrivate.m_valueOrOffset = m_file.read(8);
            if ((dePrivate.m_tag == 256) || (dePrivate.m_tag == 257) || (dePrivate.m_tag == 33550) || (dePrivate.m_tag == 33922))
            {
                ifd.m_ifdEntries.append(ifdEntry);
            }
        }
        ifd.m_nextIfdOffset = getValueFromFile<qint64>();
    }

    // parser data of ifdEntry
    foreach (auto de, ifd.m_ifdEntries)
    {
        auto &dePrivate = de;

        auto valueBytesCount = dePrivate.m_count * dePrivate.typeSize();
        // skip unknown datatype
        if (valueBytesCount == 0)
        {
            continue;
        }
        QByteArray valueBytes;
        if (!m_header.isBigTiff() && valueBytesCount > 4)
        {
            auto valueOffset = getValueFromBytes<quint32>(de.m_valueOrOffset, m_header.byteOrder);
            if (!m_file.seek(valueOffset))
            {
                throw err_seek_pos + QString::number(valueOffset);
            }
            valueBytes = m_file.read(valueBytesCount);
        }
        else if (m_header.isBigTiff() && valueBytesCount > 8)
        {
            auto valueOffset = getValueFromBytes<quint64>(de.m_valueOrOffset, m_header.byteOrder);
            if (!m_file.seek(valueOffset))
            {
                throw err_seek_pos + QString::number(valueOffset);
            }
            valueBytes = m_file.read(valueBytesCount);
        }
        else
        {
            valueBytes = dePrivate.m_valueOrOffset;
        }
        dePrivate.parserValues(valueBytes, m_header.byteOrder);
        if (dePrivate.m_tag == 256)
        {
            m_geo.width = dePrivate.m_values.last().toInt();
        } else if (dePrivate.m_tag == 257)
        {
            m_geo.height = dePrivate.m_values.last().toInt();
        } else if (dePrivate.m_tag == 33550)
        {
            m_geo.pixelWidth = dePrivate.m_values.at(0).toDouble();
            m_geo.pixelHeight = dePrivate.m_values.at(1).toDouble();
        } else if (dePrivate.m_tag == 33922)
        {
            m_geo.longitute = dePrivate.m_values.at(3).toDouble();
            m_geo.latitude = dePrivate.m_values.at(4).toDouble();
        }
    }

    return true;
}


/*!
 * Constructs the TiffFile object.
 */

TiffFile::TiffFile(const QString &filePath)
{
    m_file.setFileName(filePath);
    if (!m_file.open(QFile::ReadOnly))
    {
        m_hasError = true;
        m_errorString = m_file.errorString();
    }

    if (!readHeader())
    {
        return;
    }

    readIfd(m_header.ifd0Offset);
}


auto TiffFile::getRect() const -> QGeoRectangle
{
    if ((m_geo.longitute == 0) || (m_geo.latitude == 0))
    {
        throw err_33922_not_set;
    }
    if ((m_geo.pixelWidth == 0) || (m_geo.pixelHeight == 0))
    {
        throw err_33550_not_set;
    }
    if (m_geo.width == 0)
    {
        throw err_256_not_set;
    }
    if (m_geo.height == 0)
    {
        throw err_257_not_set;
    }

    QGeoRectangle rect;
    QGeoCoordinate coord;
    coord.setLongitude(m_geo.longitute);
    coord.setLatitude(m_geo.latitude);
    rect.setTopLeft(coord);
    coord.setLongitude(m_geo.longitute + (m_geo.width - 1) * m_geo.pixelWidth);
    coord.setLatitude(m_geo.latitude + (m_geo.height - 1) * m_geo.pixelHeight);
    rect.setBottomRight(coord);
    return rect;
}
