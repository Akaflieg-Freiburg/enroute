/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#pragma once

#include "platform/FileExchange_Abstract.h"

namespace Platform {

/*! \brief Implementation of FileExchange for Android devices */


class FileExchange : public Platform::FileExchange_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit FileExchange(QObject *parent = nullptr);

    ~FileExchange() override = default;


    //
    // Methods
    //

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    Q_INVOKABLE void importContent() override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    Q_INVOKABLE QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    Q_INVOKABLE QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;


public slots:
    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    void processFileOpenRequest(const QString& path) override;


protected:
    /*! \brief Implements virtual method from FileExchange_Abstract */
    void deferredInitialization() override;


private:
    Q_DISABLE_COPY_MOVE(FileExchange)

    // Helper function. Saves content to a file in a directory from where
    // sharing to other android apps is possible
    auto contentToTempFile(const QByteArray& content, const QString& fileNameTemplate) -> QString;

    // Name of a subdirectory within the AppDataLocation for sending and
    // receiving files.
    QString fileExchangeDirectoryName;

    // @returns True if an app could be started, false if no app was found
    static bool outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType);

    bool receiveOpenFileRequestsStarted {false};
    QString pendingReceiveOpenFileRequest {};
};

} // namespace Platform
