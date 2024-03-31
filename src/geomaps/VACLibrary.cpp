/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
#include <QImage>
#include <QTemporaryDir>
#include <QTimer>

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


// -----------

QVector<GeoMaps::VAC> GeoMaps::VACLibrary::vacsByDistance(const QGeoCoordinate& position)
{
    std::sort(m_vacs.begin(), m_vacs.end(), [position](const GeoMaps::VAC& first, const GeoMaps::VAC& second) {return position.distanceTo(first.center()) < position.distanceTo(second.center()); });
    return m_vacs;
}

QString GeoMaps::VACLibrary::importVAC(const QString& _fileName, const QString& _newName)
{
    auto file = FileFormats::DataFileAbstract::openFileURL(_fileName);
    auto fileName = file->fileName();

#warning need to handle file URLs
    auto newName = _newName;
    // Check input data
    if (!QFile::exists(fileName))
    {
        return tr("Input file <strong>%1</strong> does not exist.").arg(fileName);
    }
    GeoMaps::VAC vac(fileName);
    if (!vac.isValid())
    {
        return tr("Input file <strong>%1</strong> does not contain a valid chart.").arg(fileName);
    }
    QImage const image(fileName);
    if (image.isNull())
    {
        return tr("Unable to read raster image data from input file <strong>%1</strong>.").arg(fileName);
    }

    // Set base name, delete all existing VACs with that basename
    if (newName.isEmpty())
    {
        newName = vac.name;
    }
    else
    {
        vac.name = newName;
    }
    remove(newName);

    // Copy file to VAC directory
    QDir const dir;
    dir.mkpath(m_vacDirectory);

    QString const newFileName = m_vacDirectory + "/" + vac.name + ".webp";
    QFile::remove(newFileName);
    if (fileName.endsWith(".webp"))
    {
#warning This cannot handle Android content URLs
        if (!QFile::copy(fileName, newFileName))
        {
            return tr("Error: Unable to copy VAC file <strong>%1</strong> to destination <strong>%2</strong>.").arg(fileName, newFileName);
        }
    }
    else
    {
        if (!image.save(newFileName))
        {
            return tr("Error: Unable to write VAC file <strong>%1</strong>.").arg(newFileName);
        }
    }

    vac.fileName = newFileName;
    m_vacs.append(vac);

    emit dataChanged();

    return {};
}

QString GeoMaps::VACLibrary::importTripKit(const QString& fileName)
{
    FileFormats::TripKit tripKit(fileName);

    // Unpack the VACs into m_vacDirectory
    auto size = tripKit.numCharts();
    int successfulImports = 0;
    for(auto idx=0; idx<size; idx++)
    {
        emit importTripKitStatus((double)idx/(double)size);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

#warning
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
#warning error handling
    auto vac = get(oldName);
    if (!vac.isValid())
    {
        return "xx";
    }

    m_vacs.removeAll(get(oldName));

    auto newFileName = m_vacDirectory+"/"+newName+".webp";
    QFile::rename(vac.fileName, newFileName);
    vac.fileName = newFileName;
    vac.name = newName;
    m_vacs.append(vac);

    emit dataChanged();
    return {};
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

void GeoMaps::VACLibrary::janitor()
{
    bool hasChange = false;

    // Go through the list of all VAC. Find all VACs without image file, and a list of all image file managed
    // by VACs in the list
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
            vacsWithoutImageFile.append(vac);
        }
    }

    // Delete all VACs without image file
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
        QFileInfo fInfo(fileIterator.filePath());
        if (!imageFilesWithVAC.contains(fInfo))
        {
            imageFilesWithoutVAC.append(fInfo);
        }
    }

    // Go through the list of image files without VAC. Try to import them.
    // Failing that, delete those files.

    if (hasChange)
    {
        emit dataChanged();
    }
}
