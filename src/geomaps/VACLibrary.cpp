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

#include <QDirIterator>

#include "VACLibrary.h"

GeoMaps::VACLibrary::VACLibrary()
{
    qWarning() << "create VAC library";

    QDirIterator fileIterator(m_vacDirectory, QDir::Files);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        FileFormats::VAC const vac(fileIterator.filePath());
        if (!vac.isValid())
        {
            continue;
        }
        m_vacs.append(vac);
    }
}


QVector<FileFormats::VAC> GeoMaps::VACLibrary::vacsByDistance(const QGeoCoordinate& position)
{
    std::sort(m_vacs.begin(), m_vacs.end(), [position](const FileFormats::VAC& first, const FileFormats::VAC& second) {return position.distanceTo(first.center()) < position.distanceTo(second.center()); });
    return m_vacs;
}

QString GeoMaps::VACLibrary::importVAC(const QString& fileName, QString newName)
{
    // Check input data
    if (!QFile::exists(fileName))
    {
        return tr("Input file <strong>%1</strong> does not exist.").arg(fileName);
    }
    FileFormats::VAC vac(fileName);
    if (!vac.isValid())
    {
        return tr("Input file <strong>%1</strong> does not contain a valid chart. Error: %2").arg(fileName, vac.error());
    }
    QImage const image(fileName);
    if (image.isNull())
    {
        return tr("Unable to read raster image data from input file <strong>%1</strong>.").arg(fileName);
    }

    // Set base name, delete all existing VACs with that basename
    if (newName.isEmpty())
    {
        newName = vac.baseName();
    }
    else
    {
        vac.setBaseName(newName);
    }
    deleteVAC(newName);

    // Copy file to VAC directory
    QDir dir;
    dir.mkpath(m_vacDirectory);

    QString newFileName = m_vacDirectory+"/"+vac.baseName()+".webp";
#warning copy webp files!
    if (!image.save(newFileName))
    {
        return tr("Error: Unable to write VAC file <strong>%1</strong>.").arg(newFileName);
    }
    vac.setFileName(newFileName);
    m_vacs.append(vac);

    emit dataChanged();

    return {};
}

void GeoMaps::VACLibrary::deleteVAC(const QString& baseName)
{
    auto numDeletions = erase_if(m_vacs, [baseName](const FileFormats::VAC& vac) { return baseName == vac.baseName(); });
    if (numDeletions != 0)
    {
        emit dataChanged();
    }
}
