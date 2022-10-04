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

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QPointer>
#include <QSaveFile>

#include "Downloadable_Abstract.h"

namespace DataManagement
{

/*! \brief Downloadable object
 *
 *  This class represents a downloadable item, such as an aviation map file.
 *  The class is able to do the following.
 *
 *  - Download the file asynchronously from the server.
 *
 *  - Check if a newer version of the file is available at the URL and update
 *    the file if desired.
 *
 *  The URL and the name of the local file are given in the constructor and
 *  cannot be changed. See the description of the method startDownload() to see
 *  how downloads work.
 */

class Downloadable_SingleFile : public Downloadable_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param url The address in the internet where the newest version of the
     * item can always be found. This URL should be http or https because data
     * about the remote file (such as modification time) can be retrieved with
     * this protocol.
     *
     * @param localFileName Name of a local file where the download will be
     * stored. If the file already exists, the constructor assumes that the file
     * contains a previously downloaded version of the item, and that the
     * modification time of the file is the download time.
     *
     * @param parent The standard QObject parent pointer.
     *
     * After construction, size and modification time of the file on the server
     * are not known and set to -1 and an invalid QDateTime, respectively.  To
     * obtain these pieces of data from the server, use the method
     * checkForUpdate(). Alternatively, you can write to the properties
     * remoteFileDate and remoteFileSize directly, e.g. to restore cached data
     * when no internet connection is available.
     *
     * Use the method startFileDownload() to initiate the download process.
     */
    explicit Downloadable_SingleFile(QUrl url, const QString &localFileName, QObject *parent = nullptr);

    /*! \brief Standard destructor
     *
     * This destructor will stop all running downloads and delete all temporary
     * files.  It will not delete the local file.
     */
    ~Downloadable_SingleFile() override;



    //
    // Properties
    //

    /*! \brief Download progress
     *
     * This property holds the download progress in percent, if a download is
     * ongoing. If no download is taking place, this property is undefined.
     */
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

    /*! \brief File name, as set in the constructor */
    Q_PROPERTY(QString fileName READ fileName CONSTANT)

    /*! \brief Content of the downloaded file
     *
     * This convenience property holds the content of the downloaded file, or a null
     * QByteArray, if nothing has been downloaded
     */
    Q_PROPERTY(QByteArray fileContent READ fileContent NOTIFY fileContentChanged)

    /*! \brief Modification date of the remote file
     *
     * If the modification date of the remote file is not known, the property
     * holds an invalid QDateTime. The property can be set with the method
     * checkForUpdate(), or directly written to.
     */
    Q_PROPERTY(QDateTime remoteFileDate READ remoteFileDate WRITE setRemoteFileDate NOTIFY remoteFileDateChanged)

    /*! \brief Size of the remote file
     *
     * This property holds the size of the remote file.  If the size date of the
     * remote file is not known, the property holds an -1. The property can be
     * set with the method checkForUpdate(), or directly written to.
     */
    Q_PROPERTY(qint64 remoteFileSize READ remoteFileSize WRITE setRemoteFileSize NOTIFY remoteFileSizeChanged)

    /*! \brief URL, as set in the constructor */
    Q_PROPERTY(QUrl url READ url CONSTANT)



    //
    // Getter Methods
    //

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property description
     */
    [[nodiscard]] auto description() -> QString override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property downloading
     */
    [[nodiscard]] auto downloading() -> bool override { return !m_networkReplyDownloadFile.isNull(); }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property downloadProgress
     */
    [[nodiscard]] auto downloadProgress() const -> int { return m_downloadProgress; }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property fileName
     */
    [[nodiscard]] auto fileName() const -> QString { return m_fileName; }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property fileContent
     */
    [[nodiscard]] auto fileContent() const -> QByteArray;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property files
     */
    [[nodiscard]] auto files() -> QStringList override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property hasFile
     */
    [[nodiscard]] auto hasFile() -> bool override { return QFile::exists(m_fileName); }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property infoText
     */
    [[nodiscard]] auto infoText() -> QString override;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property remoteFileDate
     */
    [[nodiscard]] auto remoteFileDate() const -> QDateTime { return m_remoteFileDate; }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property remoteFileSize
     */
    [[nodiscard]] auto remoteFileSize() -> qint64 override { return m_remoteFileSize; }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property updateSize
     */
    [[nodiscard]] auto updateSize() -> qint64 override;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property url
     */
    [[nodiscard]] auto url() const -> QUrl { return m_url; }



    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     * @param date Property remoteFileDate
     */
    void setRemoteFileDate(const QDateTime &date);

    /*! \brief Setter function for the property with the same name
     *
     * @param size Property remoteFileSize
     */
    void setRemoteFileSize(qint64 size);



    //
    // Methods
    //

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract
     *
     * This method deletes the local file. The singals
     * aboutToChangeLocalFile() and localFileChanged() are emitted
     * appropriately, and a QLockFile is used at fileName()+".lock".
     */
    Q_INVOKABLE void deleteFiles() override;

    /*! \brief Initiate a download
     *
     * Initiate an asynchronous download of the remote file. If a download is
     * already in progress, nothing will happen.  Otherwise, the following will
     * take place.
     *
     * -# Data is retrieved from the remote server and stored in a temporary
     *    file. The signal downloadProgress() will be emitted regularly.
     *
     * -# In case of an error, the signal error() is emitted and the download
     *    stops.
     *
     * -# Optionally, the download can be stopped using the method
     *    stopFileDownload().
     *
     * Once all data has been downloaded successfully to the temporary file, the
     * process continues as follows.
     *
     * -# The signal aboutToChangeLocalFile() is emitted. As the name suggests,
     *    this indicates that the local file is about to change and that it
     *    should not be used anymore.
     *
     * -# A QLockFile is created at fileName()+".lock"
     *
     * -# The local file is overwritten by the newly downloaded data.
     *
     * -# The QLockFile is removed
     *
     * -# The signal fileChanged() is emitted to indicate that the file is
     *    again ready to be used.
     */
    Q_INVOKABLE void startDownload() override;

    /*! \brief Contacts the server and downloads information about the remote
     *  file
     *
     * This method contacts the remote server, retrieves information about the
     * remote file and updates the properties remoteFileDate and remoteFileSize
     * accordingly.  This method runs asynchronously, and emits the signal
     * remoteFileInfoChanged() if the data really changes.  If a request is
     * already running, this method does nothing.
     *
     * @warning This method fails silently if the server cannot be contacted or
     * if the server is unable to provide the requested data.
     */
    Q_INVOKABLE void startInfoDownload();

    /*! \brief Stops download process
     *
     * This method stops the currenly running download process gracefully and
     * deletes any partially downloaded data. No signal will be emitted.  If no
     * download is in progress, nothing will happen.
     */
    Q_INVOKABLE void stopDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void update() override;

signals:
    /*! \brief Warning that local file is about to change
     *
     * This signal is emitted once the download finished, just before the local
     * file is overwritten with new data. It indicates that all users should
     * stop using the file immediately. This signal is always followed by the
     * signal localFileChanged(), which indicates that the local file can be
     * used again.
     *
     * @param localFileName Name of the local file that has will change
     *
     * @see localFileChanged()
     */
    void aboutToChangeFile(QString localFileName);

    /*! \brief Download progress
     *
     * While the download process is running, this signal is emitted at regular
     * intervals, so the user can learn about the progress of the operation.
     *
     * @param percentage An integer between 0 and 100
     */
    void downloadProgressChanged(int percentage);

    /*! \brief Notifier signal for the properties remoteFileDate and remoteFileSize
     *
     * This signal is emitted once one of the property remoteFileDate changes,
     * either in response to a use of the setter methods, or because
     * downloadRemoteFileData() has been called and data has been retrieved from
     * the server.
     */
    void remoteFileDateChanged();

private:
    Q_DISABLE_COPY_MOVE(Downloadable_SingleFile)

    // Called when an error occurs during the download of the remote file, this
    // method deletes QTemporaryFile _tmpFile, emits the signal error() and
    // deletes _networkReplyDownload by calling deleteLater.  Connected to
    // &QNetworkReply::error of _networkReplyDownload.
    void downloadFileErrorReceiver(QNetworkReply::NetworkError code);

    // Called once download of the remote file is finished, this method
    // overwrites the local file. It deletes the QTemporaryFile _tmpFile, and
    // deletes _networkReplyDownload by calling deleteLater. Connected to
    // &QNetworkReply::finished of _networkReplyDownload.
    void downloadFileFinished();

    // Called during the download of the remote file, this method emits the
    // signal downloadProgress().  Connected to &QNetworkReply::downloadProgress
    // of _networkReplyDownload.
    void downloadFileProgressReceiver(qint64 bytesReceived, qint64 bytesTotal);

    // Called during the download of the remote file, this method reads all the
    // data that has been downloaded so far and stores it in the QTemporaryFile
    // _tmpFile.  Connected to &QNetworkReply::readyRead of
    // _networkReplyDownload.
    void downloadFilePartialDataReceiver();

    // Called once download of the remote file header data is finished, this
    // method updates the properties remoteFileDate and remoteFileSize, and
    // _networkReplyDownloadHeader by calling deleteLater. Connected to
    // &QNetworkReply::finished of _networkReplyDownloadHeader.
    void downloadHeaderFinished();

    // This member holds the download progress.
    int m_downloadProgress{0};

    // NetworkReply for downloading of remote file data. Set to nullptr when no
    // download is in progress.
    QPointer<QNetworkReply> m_networkReplyDownloadFile;

    // NetworkReply for downloading of remote file header. Set to nullptr when
    // no download is in progress.
    QPointer<QNetworkReply> m_networkReplyDownloadHeader;

    // Temporary file for storing partiall data when downloading the remote
    // file. Set to nullptr when no download is in progress.
    QPointer<QSaveFile> m_saveFile{};

    // URL of the remote file, as set in the constructor
    QUrl m_url;

    // Name of the local file, as set in the constructor
    QString m_fileName{};

    // Modification date of the remote file, set directly via a setter method or
    // by calling downloadRemoteFileInfo().
    QDateTime m_remoteFileDate;

    // Size of the remote file, set directly via a setter method or by calling
    // downloadRemoteFileInfo().
    qint64 m_remoteFileSize{-1};
};

};
