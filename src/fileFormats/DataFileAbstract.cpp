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

#include <QTemporaryFile>

#include "fileFormats/DataFileAbstract.h"

QSharedPointer<QFile> FileFormats::DataFileAbstract::openFileURL(const QString& fileName)
{
    if (fileName.startsWith("file://"))
    {
        auto* file = new QFile(fileName.mid(7));
        return QSharedPointer<QFile>(file);
    }

    if (fileName.startsWith("content://"))
    {
        auto* file = new QTemporaryFile();
        file->open();

        QFile contentFile(fileName);
        contentFile.open(QIODeviceBase::ReadOnly);
        auto buffer = contentFile.readAll();
        file->write(buffer);
        file->close();
        return QSharedPointer<QFile>(file);
    }


    auto file = new QFile(fileName);
    return QSharedPointer<QFile>(file);
}
