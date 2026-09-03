/***************************************************************************
 *   Copyright (C) 2023-2025 by Stefan Kebekus                             *
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

#include <QDebug>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QUrl>

#include "fileFormats/DataFileAbstract.h"

using namespace Qt::Literals::StringLiterals;


QSharedPointer<QFile> FileFormats::DataFileAbstract::openFileURL(const QString& fileName)
{
    if (fileName.startsWith(u"file://"_s))
    {
        // Desktop file managers hand over percent-encoded URLs, so the path
        // must be decoded rather than obtained by stripping the scheme.
        auto* file = new QFile(QUrl(fileName).toLocalFile());
        return QSharedPointer<QFile>(file);
    }

    if (fileName.startsWith(u"content://"_s))
    {
        auto* file = new QTemporaryFile();
        if (file->open())
        {
            QFile contentFile(fileName);
            if (contentFile.open(QIODeviceBase::ReadOnly))
            {
                auto buffer = contentFile.readAll();
                file->write(buffer);
            }
            file->close();
        }
        return QSharedPointer<QFile>(file);
    }

    auto *file = new QFile(fileName);
    return QSharedPointer<QFile>(file);
}


bool FileFormats::DataFileAbstract::saveFileAtomically(const QString& path, const QByteArray& data, QString* error)
{
    auto fail = [&path, error](const QString& message) {
        qWarning() << "saveFileAtomically:" << path << message;
        if (error != nullptr)
        {
            *error = message;
        }
        return false;
    };

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        return fail(file.errorString());
    }
    if (file.write(data) != data.size())
    {
        auto message = file.errorString();
        file.cancelWriting();
        return fail(message);
    }
    if (!file.commit())
    {
        return fail(file.errorString());
    }
    return true;
}
