/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QtGlobal>
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)


#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>

#include "platform/PlatformAdaptor_Linux.h"



Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    ;
}



//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    Q_UNUSED(this);
    return QStringLiteral("<unknown ssid>");
}


auto Platform::PlatformAdaptor::hasMissingPermissions() -> bool
{
    Q_UNUSED(this);
    return false;
}


void Platform::PlatformAdaptor::importContent()
{
    Q_UNUSED(this);

    auto fileNameX = QFileDialog::getOpenFileName(nullptr, tr("Import data"), QDir::homePath(), tr("All files (*)"));
    if (!fileNameX.isEmpty())
    {
        processFileOpenRequest(fileNameX);
    }
}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    Q_UNUSED(lock)
}


auto Platform::PlatformAdaptor::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)
    Q_UNUSED(this);

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


void Platform::PlatformAdaptor::vibrateBrief()
{
    Q_UNUSED(this);
}


auto Platform::PlatformAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    Q_UNUSED(mimeType)
    Q_UNUSED(this);

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


// -------- Methods of questionable value start here.

void Platform::PlatformAdaptor::hideSplashScreen()
{

}


auto Platform::PlatformAdaptor::manufacturer() -> QString
{
    return {};
}

#endif // defined(Q_OS_LINUX)
