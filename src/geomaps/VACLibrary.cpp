/***************************************************************************
 *   Copyright (C) 2024-2025 by Stefan Kebekus                             *
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

#include <QCoreApplication>
#include <QDirIterator>
#include <QGeoRectangle>
#include <QImage>
#include <QTemporaryDir>
#include <QTimer>

#include "Librarian.h"
#include "VACLibrary.h"
#include "fileFormats/TripKit.h"



//
// Constructor and destructor
//

GeoMaps::VACLibrary::VACLibrary(QObject *parent)
    : QObject(parent)
{
    // Wire up: Save library whenever the content changes
    connect(this, &GeoMaps::VACLibrary::dataChanged, this, &GeoMaps::VACLibrary::save, Qt::QueuedConnection);

    // Restore previously saves VAC library
    if (m_dataFile.open(QIODeviceBase::ReadOnly))
    {
        QDataStream dataStream(&m_dataFile);
        dataStream >> m_vacs;
    }
    m_dataFile.close();

    // Call the janitor as soon as we have some time
    QTimer::singleShot(0, this, &GeoMaps::VACLibrary::janitor);
}

GeoMaps::VACLibrary::~VACLibrary()
{
    save();
}


//
// Getter Methods
//


QVector<GeoMaps::VAC> GeoMaps::VACLibrary::vacs()
{
    std::sort(m_vacs.begin(), m_vacs.end(), [](const GeoMaps::VAC& first, const GeoMaps::VAC& second) { return first.name < second.name; });
    return m_vacs;
}



//
// Methods
//

void GeoMaps::VACLibrary::clear()
{
    if (m_vacs.isEmpty())
    {
        return;
    }
    foreach(auto vac, m_vacs)
    {
        QFile::remove(vac.fileName);
    }
    m_vacs.clear();
    emit dataChanged();
}

GeoMaps::VAC GeoMaps::VACLibrary::get(const QString& name)
{
    foreach(auto vac, m_vacs)
    {
        if (vac.name == name)
        {
            return vac;
        }
    }
    return {};
}

QString GeoMaps::VACLibrary::importTripKit(const QString& fileName)
{
    // Open Trip Kit
    FileFormats::TripKit tripKit(fileName);
    if (!tripKit.isValid())
    {
        return tr("Unable to open TripKit file <strong>%1</strong>. Error: %2.").arg(fileName, tripKit.error());
    }

    // Create directory
    QDir const dir;
    dir.mkpath(m_vacDirectory);

    // Unpack the VACs into m_vacDirectory
    auto size = tripKit.numCharts();
    int successfulImports = 0;
    for(auto idx=0; idx<size; idx++)
    {
        emit importTripKitStatus((double)idx/(double)size);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        auto vac = tripKit.extract(m_vacDirectory, idx);
        if (!vac.isValid())
        {
            continue;
        }
        auto oldVac = get(vac.name);
        if (oldVac.isValid())
        {
            m_vacs.removeAll(oldVac);
        }

        m_vacs.append(vac);
        successfulImports++;
    }
    emit importTripKitStatus(1.0);
    emit dataChanged();

    if (successfulImports == 0)
    {
        return tr("Error reading TripKip: No charts imported.");
    }
    if (successfulImports < size)
    {
        return tr("Error reading TripKip: Only %1 out of %2 charts were successfully imported.").arg(successfulImports).arg(size);
    }

    return {};
}

QString GeoMaps::VACLibrary::importVAC(const QString& fileName, const QString& newName)
{
    // Download content URL if necessary, get name of the downloaded file
    auto _file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto _fileName = _file->fileName();

    // Check input data
    if (!_file->exists())
    {
        return tr("Input file <strong>%1</strong> does not exist.").arg(_fileName);
    }
    GeoMaps::VAC vac(_fileName, {});
    if (!vac.isValid())
    {
        return tr("Input file <strong>%1</strong> does not contain a valid chart.").arg(_fileName);
    }
    QImage const image(_fileName);
    if (image.isNull())
    {
        return tr("Unable to read raster image data from the input file <strong>%1</strong>.").arg(_fileName);
    }

    // Get new name of VAC, either by taking the newName specified in the argument
    // or the name gleaned from the filename.
    auto _newName = newName;
    if (_newName.isEmpty())
    {
        _newName = vac.name;
    }
    else
    {
        vac.name = _newName;
    }

    // Delete all existing VACs with the new name
    remove(_newName);

    // Copy file to VAC directory
    QDir const dir;
    dir.mkpath(m_vacDirectory);
    QString const newFileName = absolutePathForVac(vac);
    QFile::remove(newFileName);
    if (_fileName.endsWith(u".webp"_s))
    {
        if (!QFile::copy(_fileName, newFileName))
        {
            return tr("Error: Unable to copy the VAC file <strong>%1</strong> to destination <strong>%2</strong>.").arg(_fileName, newFileName);
        }
    }
    else
    {
        if (!image.save(newFileName))
        {
            return tr("Error: Unable to write the VAC file <strong>%1</strong>.").arg(newFileName);
        }
    }

    // Set new file name and add to library
    vac.fileName = newFileName;
    m_vacs.append(vac);

    emit dataChanged();

    return {};
}

void GeoMaps::VACLibrary::remove(const QString& baseName)
{
    QVector<GeoMaps::VAC> vacsToDelete;
    foreach(auto vac, m_vacs)
    {
        if (vac.name == baseName)
        {
            vacsToDelete.append(vac);
        }
    }

    if (vacsToDelete.isEmpty())
    {
        return;
    }

    foreach (auto vac, vacsToDelete)
    {
        QFile::remove(vac.fileName);
        m_vacs.removeAll(vac);
    }
    emit dataChanged();
}

QString GeoMaps::VACLibrary::rename(const QString& oldName, const QString& newName)
{
    // Get VAC with old name
    auto vac = get(oldName);
    if (!vac.isValid())
    {
        return tr("VAC <strong>%1</strong> does not exist.").arg(oldName);
    }

    // Rename raster image file
    auto newFileName = absolutePathForVac(newName);
    if (!QFile::rename(vac.fileName, newFileName))
    {
        return tr("VAC file renaming failed.");
    }

    // Remove old VAC from list, update data and add agaib
    m_vacs.removeAll(vac);
    vac.fileName = newFileName;
    vac.name = newName;
    m_vacs.append(vac);

    emit dataChanged();
    return {};
}

QVector<GeoMaps::VAC> GeoMaps::VACLibrary::vacsByDistance(const QGeoCoordinate& position, const QString& filter)
{
    QStringList filterWords;
    foreach(auto word, filter.simplified().split(' ', Qt::SkipEmptyParts)) {
        QString const simplifiedWord = GlobalObject::librarian()->simplifySpecialChars(word);
        if (simplifiedWord.isEmpty()) {
            continue;
        }
        filterWords.append(simplifiedWord);
    }

    QVector<GeoMaps::VAC> result;
    const auto constvacs = m_vacs;
    for(const auto& vac : constvacs) {
        if (!vac.isValid())
        {
            continue;
        }
        bool allWordsFound = true;
        for(const auto& word : filterWords)
        {
            QString const fullName = GlobalObject::librarian()->simplifySpecialChars(vac.name);
            if (!fullName.contains(word, Qt::CaseInsensitive))
            {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound)
        {
            result.append(vac);
        }
    }

    std::sort(result.begin(), result.end(), [position](const GeoMaps::VAC& first, const GeoMaps::VAC& second) {return position.distanceTo(first.center()) < position.distanceTo(second.center()); });
    return result;
}

QVector<GeoMaps::VAC> GeoMaps::VACLibrary::vacs4Point(const QGeoCoordinate& position)
{
    QVector<GeoMaps::VAC> result;
    const auto constvacs = m_vacs;
    for(const auto& vac : constvacs) {
        if (!vac.isValid())
        {
            continue;
        }
        if (vac.boundingBox().contains(position))
        {
            result.append(vac);
        }
    }
    std::sort(result.begin(), result.end(), [](const GeoMaps::VAC& first, const GeoMaps::VAC& second) {return first.name < second.name; });
    return result;
}



//
// Private Methods
//

void GeoMaps::VACLibrary::janitor()
{
    // Go through the list of all VAC. Find all VACs without image file, and a
    // list of all image file managed by VACs in the list
    QVector<GeoMaps::VAC> vacsWithoutImageFile;
    QVector<QFileInfo> imageFilesWithVAC;
    foreach(auto vac, m_vacs)
    {
        if (QFile::exists(vac.fileName))
        {
            imageFilesWithVAC.append(QFileInfo(vac.fileName));
        }
        else
        {
            auto newFileName = absolutePathForVac(vac);
            if (QFile::exists(newFileName))
            {
                // This mechanism is necessary after an app update on iOS
                // devices. After an update, the path of the app container is
                // changed, and therefore the location of the vac files => we
                // have to set the path to the current location
                m_vacs.removeAll(vac);
                vac.fileName = newFileName;
                m_vacs.append(vac);
                imageFilesWithVAC.append(QFileInfo(vac.fileName));
            }
            else
            {
                vacsWithoutImageFile.append(vac);
            }
        }
    }

    // Delete all VACs without image file
    bool hasChange = false;
    foreach(auto vac, vacsWithoutImageFile)
    {
        hasChange = true;
        m_vacs.removeAll(vac);
    }

    // Find list of all image files without VAC
    QVector<QFileInfo> imageFilesWithoutVAC;
    QDirIterator fileIterator(m_vacDirectory, QDir::Files);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        QFileInfo const fInfo(fileIterator.filePath());
        if (!imageFilesWithVAC.contains(fInfo))
        {
            imageFilesWithoutVAC.append(fInfo);
        }
    }

    // Go through the list of image files without VAC. Try to import them.
    // Failing that, delete those files.
    foreach(auto fInfo, imageFilesWithVAC)
    {
        if (importVAC(fInfo.fileName()).isEmpty())
        {
            hasChange = true;
        }
        QFile::remove(fInfo.fileName());
    }

    if (hasChange)
    {
        emit dataChanged();
    }
}

void GeoMaps::VACLibrary::save()
{
    if (m_dataFile.open(QIODeviceBase::WriteOnly))
    {
        QDataStream dataStream(&m_dataFile);
        dataStream << m_vacs;
    }
    m_dataFile.close();
}

QString GeoMaps::VACLibrary::absolutePathForVac(GeoMaps::VAC vac)
{
    return absolutePathForVac(vac.name);
}

QString GeoMaps::VACLibrary::absolutePathForVac(const QString& name)
{
    return m_vacDirectory + "/" + name + ".webp";
}
