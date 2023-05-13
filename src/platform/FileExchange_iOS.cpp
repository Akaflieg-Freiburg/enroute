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

#include "platform/FileExchange_iOS.h"

#include "ios/ObjCAdapter.h"
#include "qfileinfo.h"
#include "qmimedatabase.h"

#include <QDesktopServices>


Platform::FileExchange::FileExchange(QObject *parent)
    : FileExchange_Abstract(parent)
{
    // Standard constructor. Recall that the constructor must not call virtual functions.
    // If you need virtual functions, use the methode deferredInitialization below.

    QDesktopServices::setUrlHandler("file", this, "handleFileUrlReceived");
}


/*void Platform::FileExchange::deferredInitialization()
{
    // This method is called immediately after the instance has been constructed.
    // It can be used to implement initialization that calls virtual methods.
#warning Not implemented
}
*/


//
// Methods
//

void Platform::FileExchange::importContent()
{
    // Desktop: open file dialog and call processFileOpenRequest() on the filename.
    // Mobile platforms: do nothing
#warning not implemented
}


auto Platform::FileExchange::shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) -> QString
{
    // Share file content

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    QString fileExtension = mime.preferredSuffix();

    ObjCAdapter::shareContent(content, mimeType, fileNameTemplate, fileExtension);
    return {};
}


auto Platform::FileExchange::viewContent(const QByteArray& content, const QString& /*mimeType*/, const QString& fileNameTemplate) -> QString
{
    // View file content
#warning not implemented
    return {};
}

void Platform::FileExchange::handleFileUrlReceived(const QUrl &url)
{
    QString myUrl = url.toString();

    //Remove the "file://" Prefix
    myUrl.remove(0, 7);
    // … remove "file://" from Url
    // … then check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    if(fileInfo.exists()) {
        Platform::FileExchange_Abstract::processFileOpenRequest(myUrl);
    } else {
        //TODO: Error Handling
    }
}
