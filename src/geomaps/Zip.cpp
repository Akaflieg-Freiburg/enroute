
#include <QCommandLineParser>
#include <QFile>
#include <QString>
#include <QDataStream>
#include <zip.h>

#include "geomaps/Zip.h"


GeoMaps::Zip::Zip(const QString& fileName)
{
    int error;
    m_zip = zip_open(fileName.toUtf8().data(), ZIP_RDONLY, &error);
    if (m_zip == nullptr)
    {
        return;
    }

    // Get number of files in this archive
    auto numEntries = zip_get_num_entries((zip_t*)m_zip, 0);
    m_fileNames.reserve(numEntries);
    m_fileSizes.reserve(numEntries);

    // Read info for every file
    for(auto i=0; i<numEntries; i++)
    {
        struct zip_stat zStat;
        auto error = zip_stat_index((zip_t*)m_zip, i, 0, &zStat);
        if (error != 0)
        {
            return;
        }
        if ( ((zStat.valid&ZIP_STAT_NAME) == 0) || ((zStat.valid&ZIP_STAT_SIZE) == 0))
        {
            return;
        }
        if ((zStat.valid&ZIP_STAT_NAME) == 0)
        {
            return;
        }
        m_fileNames += QString::fromUtf8(zStat.name);
        m_fileSizes += zStat.size;
    }
}

GeoMaps::Zip::~Zip()
{
    if (m_zip != nullptr)
    {
        zip_close((zip_t*)m_zip);
        m_zip = nullptr;
    }
}

QByteArray GeoMaps::Zip::extract(qsizetype index)
{
    if (m_zip == nullptr)
    {
        return {};
    }
    if ((index < 0) || (index >= m_fileNames.size()))
    {
        return {};
    }

    auto fileSize = m_fileSizes[index];
    QByteArray data(fileSize, 0);
    auto zf = zip_fopen_index((zip_t*)m_zip, index, 0);
    if (zf == nullptr)
    {
        return {};
    }
    auto numBytesRead = zip_fread(zf, data.data(), fileSize);
    if (numBytesRead != fileSize)
    {
        return {};
    }
    zip_fclose(zf);

    return data;
}

QByteArray GeoMaps::Zip::extract(const QString& fileName)
{
    auto idx = m_fileNames.indexOf(fileName);
    return extract(idx);
}
