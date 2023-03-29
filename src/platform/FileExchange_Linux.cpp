/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include "platform/FileExchange_Linux.h"



Platform::FileExchange::FileExchange(QObject *parent)
    : FileExchange_Abstract(parent)
{
}



//
// Methods
//

void Platform::FileExchange::importContent()
{
    auto fileNameX = QFileDialog::getOpenFileName(nullptr, tr("Import data"), QDir::homePath(), tr("All files (*)"));
    if (!fileNameX.isEmpty())
    {
        processFileOpenRequest(fileNameX);
    }
}


auto Platform::FileExchange::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);

    auto fileNameX = QFileDialog::getSaveFileName(nullptr, tr("Export flight route"), QDir::homePath()+"/"+fileNameTemplate+"."+mime.preferredSuffix(), tr("%1 (*.%2);;All files (*)").arg(mime.comment(), mime.preferredSuffix()));
    if (fileNameX.isEmpty())
    {
        return QStringLiteral("abort");
    }
    QFile file(fileNameX);
    if (!file.open(QIODevice::WriteOnly))
    {
        return tr("Unable to open file <strong>%1</strong>.").arg(fileNameX);
    }

    if (file.write(content) != content.size())
    {
        return tr("Unable to write to file <strong>%1</strong>.").arg(fileNameX);
    }
    file.close();
    return {};
}


auto Platform::FileExchange::viewContent(const QByteArray& content, const QString& /*mimeType*/, const QString& fileNameTemplate) -> QString
{
    QTemporaryFile tmpFile(fileNameTemplate.arg(QStringLiteral("XXXXXX")));
    tmpFile.setAutoRemove(false);
    if (!tmpFile.open()) {
        return tr("Unable to open temporary file.");
    }
    tmpFile.write(content);
    tmpFile.close();

    bool success = QDesktopServices::openUrl(QUrl("file://" + tmpFile.fileName(), QUrl::TolerantMode));
    if (success)
    {
        return {};
    }
    return tr("Unable to open data in other app.");
}
