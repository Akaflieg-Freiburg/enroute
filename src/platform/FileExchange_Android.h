/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit FileExchange(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit FileExchange() = delete;

    // factory function for QML singleton
    static Platform::FileExchange* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::fileExchange();
    }

    ~FileExchange() override = default;


    //
    // Methods
    //

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    void importContent() override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract
     *
     *  @param content see documentation for FileExchange_Abstract
     *
     *  @param mimeType see documentation for FileExchange_Abstract
     *
     *  @param fileNameSuffix see documentation for FileExchange_Abstract
     *
     *  @param fileNameTemplate see documentation for FileExchange_Abstract
     *
     *  @returns see documentation for FileExchange_Abstract
     */
    QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameSuffix, const QString& fileNameTemplate) override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract
     *
     *  @param content see documentation for FileExchange_Abstract
     *
     *  @param mimeType see documentation for FileExchange_Abstract
     *
     *  @param fileNameSuffix see documentation for FileExchange_Abstract
     *
     *  @param fileNameTemplate see documentation for FileExchange_Abstract
     *
     *  @returns see documentation for FileExchange_Abstract
     */
    QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameSuffix, const QString& fileNameTemplate) override;

    /*! \brief Opens the native Android file picker
     *
     *  This method facilitates a workaround against
     *  https://bugreports.qt.io/browse/QTBUG-118154 which renders the file
     *  dialog effectively unusable under Android. This method returns
     *  immediately. It display the file picker on top of the app. Once a file
     *  is chosen, the standart import mechanism is invoked.
     *
     *  @param mimeType A space-separated list of mime types for that that
     *  should be selectable
     */
    Q_INVOKABLE static void openFilePicker(const QString& mimeType);

public slots:
    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    void onGUISetupCompleted() override;

    /*! \brief Implements pure virtual method from FileExchange_Abstract */
    void processFileOpenRequest(const QString& path, const QString& unmingledFilename) override;


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
    QString pendingReceiveOpenFileRequest;
    QString pendingReceiveOpenFileRequestUnmingledName;
};

} // namespace Platform
