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

#include <QFile>

#include "fileFormats/DataFileAbstract.h"
#include "GeoTIFF.h"


//
// Enums and static helper functions
//

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

// Checks the status of dataStream. If status is OK, this method does nothing.
// Otherwiese, it throws a QString with a human-readable, translated error
// message.
void checkForError(QDataStream& dataStream)
{
    switch(dataStream.status())
    {
    case QDataStream::ReadCorruptData:
        throw QObject::tr("Found corrupt data while reading the data stream.", "FileFormats::GeoTIFF");
    case QDataStream::ReadPastEnd:
        throw QObject::tr("Read past end of data stream.", "FileFormats::GeoTIFF");
    case QDataStream::WriteFailed:
        throw QObject::tr("Error writing to data stream.", "FileFormats::GeoTIFF");
    case QDataStream::Ok:
        break;
    }
}



//
// Constructors
//

FileFormats::GeoTIFF::GeoTIFF(const QString& fileName)
{
    auto inFile = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!inFile->open(QFile::ReadOnly))
    {
        setError(inFile->errorString());
        return;
    }

    readTIFFData(*inFile.data());
}

FileFormats::GeoTIFF::GeoTIFF(QIODevice& device)
{
    readTIFFData(device);
}



//
// Private Methods
//

void FileFormats::GeoTIFF::readTIFFData(QIODevice& device)
{
    QDataStream dataStream(&device);

    try
    {
        // Move to beginning of the data stream
        if (!device.seek(0))
        {
            throw device.errorString();
        }

        // Check magic bytes
        auto magicBytes = device.read(2);
        if (magicBytes == "II")
        {
            dataStream.setByteOrder(QDataStream::LittleEndian);
        }
        else if (magicBytes == "MM")
        {
            dataStream.setByteOrder(QDataStream::BigEndian);
        }
        else
        {
            throw QObject::tr("Found invalid TIFF file data.", "FileFormats::GeoTIFF");
        }

        // version
        quint16 version = 0;
        dataStream >> version;
        if (version == 43)
        {
            throw QObject::tr("BigTIFF files are not supported.", "FileFormats::GeoTIFF");
        }
        if (version != 42)
        {
            throw QObject::tr("Found an unsupported TIFF version.", "FileFormats::GeoTIFF");
        }

        // ifd0Offset
        quint32 result = 0;
        dataStream >> result;
        checkForError(dataStream);
        if (!device.seek(result))
        {
            throw device.errorString();
        }

        quint16 tagCount = 0;
        dataStream >> tagCount;
        checkForError(dataStream);
        if (tagCount > 100)
        {
            addWarning( QObject::tr("Found more than 100 tags in the TIFF file. Reading only the first 100.", "FileFormats::GeoTIFF") );
            tagCount = 100;
        }

        for (quint16 i=0; i<tagCount; ++i)
        {
            readTIFFField(device, dataStream);
        }

        interpretGeoData();
    }
    catch (QString& message)
    {
        setError(message);
    }
}

void FileFormats::GeoTIFF::readTIFFField(QIODevice& device, QDataStream& dataStream)
{
    // Read tag, type, and count
    quint16 tag = 0;
    quint16 type = DT_Undefined;
    quint32 count = 0;
    dataStream >> tag;
    dataStream >> type;
    dataStream >> count;
    checkForError(dataStream);

    // Compute the data size of this type
    int typeSize = 0;
    switch(type)
    {
    case DT_Byte:
    case DT_SByte:
    case DT_Ascii:
    case DT_Undefined:
        typeSize = 1;
        break;

    case DT_Short:
    case DT_SShort:
        typeSize = 2;
        break;

    case DT_Long:
    case DT_SLong:
    case DT_Ifd:
    case DT_Float:
        typeSize = 4;
        break;

    case DT_Rational:
    case DT_SRational:
    case DT_Long8:
    case DT_SLong8:
    case DT_Ifd8:
    case DT_Double:
        typeSize = 8;
        break;

    default:
        typeSize = 0;
        break;
    }

    // Save file position and move to the position where the data actually
    // resides.
    auto filePos = device.pos();
    auto byteSize = typeSize*count;
    if (byteSize > 4)
    {
        quint32 newPos = 0;
        dataStream >> newPos;
        checkForError(dataStream);
        if (!device.seek(newPos))
        {
            throw device.errorString();
        }
    }

    // Read data entries from the device
    QVariantList values;
    switch (type)
    {
    case DT_Ascii:
    {
        auto tmpString = device.read(count);
        if (tmpString.size() != count)
        {
            throw QObject::tr("Cannot read data.", "FileFormats::GeoTIFF");
        }
        foreach(auto subStrings, tmpString.split(0))
        {
            values.append(QString::fromLatin1(subStrings));
        }
    }
    break;
    case DT_Short:
        values.reserve(count);
        for (quint32 i = 0; i < count; ++i)
        {
            quint16 tmpInt = 0;
            dataStream >> tmpInt;
            checkForError(dataStream);
            values.append(tmpInt);
        }
        break;
    case DT_Double:
        values.reserve(count);
        for (quint32 i = 0; i < count; ++i)
        {
            double tmpFloat = NAN;
            dataStream >> tmpFloat;
            checkForError(dataStream);
            values.append(tmpFloat);
        }
        break;
    default:
        break;
    }

    // Position the device at the byte following the current entry
    if (!device.seek(filePos+4))
    {
        throw device.errorString();
    }

    m_TIFFFields[tag] = values;
}

void FileFormats::GeoTIFF::interpretGeoData()
{
    // Handle Tag 33922, name
    if (m_TIFFFields.contains(270))
    {
        auto values = m_TIFFFields[270];
        if (values.isEmpty())
        {
            throw QObject::tr("No data for tag 270.", "FileFormats::GeoTIFF");
        }
        m_name = values.constFirst().toString();
    }

    // Handle Tag 33922, compute top left of the bounding box
    {
        if (m_TIFFFields.contains(33922))
        {
            auto values = m_TIFFFields[33922];
            if (values.size() < 5)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }

            bool ok = false;
            auto lat = m_TIFFFields[33922].at(4).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            auto lon = m_TIFFFields[33922].at(3).toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }

            QGeoCoordinate const coord(lat, lon);
            if (!coord.isValid())
            {
                throw QObject::tr("Invalid data for tag 33922.", "FileFormats::GeoTIFF");
            }
            m_bBox.setTopLeft(coord);
        }
        else
        {
            throw QObject::tr("Tag 33922 is not set.", "FileFormats::GeoTIFF");
        }
    }

    // Handle Tag 33922, compute pixel width and height
    double pixelWidth = NAN;
    double pixelHeight = NAN;
    {
        if (m_TIFFFields.contains(33550))
        {
            auto values = m_TIFFFields[33550];
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

    // Handle Tag 256, compute width
    quint16 width = 0;
    {
        if (m_TIFFFields.contains(256))
        {
            auto values = m_TIFFFields.value(256);
            if (values.isEmpty())
            {
                throw QObject::tr("No data for tag 256.", "FileFormats::GeoTIFF");
            }
            bool ok = false;
            width = values.constLast().toUInt(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 256.", "FileFormats::GeoTIFF");
            }
        }
        else
        {
            throw QObject::tr("Tag 256 is not set.", "FileFormats::GeoTIFF");
        }
    }

    // Handle Tag 257, compute height
    quint16 height = 0;
    {
        if (m_TIFFFields.contains(257))
        {
            auto values = m_TIFFFields.value(257);
            if (values.isEmpty())
            {
                throw QObject::tr("No data for tag 257.", "FileFormats::GeoTIFF");
            }
            bool ok = false;
            height = values.constLast().toUInt(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid data for tag 257.", "FileFormats::GeoTIFF");
            }
        }
        else
        {
            throw QObject::tr("Tag 257 is not set.", "FileFormats::GeoTIFF");
        }
    }

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
