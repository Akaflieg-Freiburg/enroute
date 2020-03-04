/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#ifndef DOWNLOADABLE_H
#define DOWNLOADABLE_H

#include <QFile>
#include <QNetworkReply>
#include <QPointer>
#include <QSaveFile>


/*! \brief Base class for all downloadable objects
  
  This class represents a downloadable item, such as an aviation map file.  The
  class is able to do the following.

  - Download the file asynchronously from the server.

  - Check if a newer version of the file is available at the URL and update the
    file if desired.

  The URL and the name of the local file are given in the constructor and cannot
  be changed. See the description of the method startFileDownload() to see how
  downloads work.
*/

class Downloadable : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor

    @param url The address in the internet where the newest version of the item
    can always be found. This URL should be http or https because data about the
    remote file (such as modification time) can be retrieved with this protocol.
    
    @param localFileName Name of a local file where the download will be
    stored. If the file already exists, the constructor assumes that the file
    contains a previously downloaded version of the item, and that the
    modification time of the file is the download time.
    
    @param networkAccessManager Pointer to a QNetworkAccessManager that will be
    used for network access. The QNetworkAccessManager needs to survive the
    lifetime of this object.
    
    @param parent The standard QObject parent pointer.

    After construction, size and modification time of the file on the server are
    not known and set to -1 and an invalid QDateTime, respectively.  To obtain
    these pieces of data from the server, use the method
    checkForUpdate(). Alternatively, you can write to the properties
    remoteFileDate and remoteFileSize directly, e.g. to restore cached data when
    no internet connection is available.

    Use the method startFileDownload() to initiate the download process.
  */
    explicit Downloadable(QUrl url, QString localFileName, QNetworkAccessManager *networkAccessManager, QObject *parent=nullptr);

    // No copy constructor
    Downloadable(Downloadable const&) = delete;

    // No assign operator
    Downloadable& operator =(Downloadable const&) = delete;

    // No move constructor
    Downloadable(Downloadable&&) = delete;

    // No move assignment operator
    Downloadable& operator=(Downloadable&&) = delete;

    /*! \brief Standard destructor

    This destructor will stop all running downloads and delete all temporary
    files.  It will not delete the local file.
  */
    ~Downloadable() override;

    /*! \brief Indicates whether a download process is currently running

    @see startFileDownload(), stopFileDownload()
  */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter function for the property with the same name */
    bool downloading() const { return !_networkReplyDownloadFile.isNull(); }

    /*! \brief Download progress

    This property holds the download progress, if a download is ongoing. If no
    download is taking place, this property is undefined.
  */
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)

    /*! \brief Getter function for the property with the same name */
    int downloadProgress() const { return _downloadProgress; }

    /*! \brief File name, as set in the constructor */
    Q_PROPERTY(QString fileName READ fileName CONSTANT)

    /*! \brief Getter function for the property with the same name */
    QString fileName() const { return _fileName; }

    /*! \brief Convenience property, returns 'true' if a local file exists

    @warning The notification signal is emitted whenever this class changes the
    local file. The signal is not emitted when another process touches the file.
  */
    Q_PROPERTY(bool hasLocalFile READ hasLocalFile NOTIFY localFileChanged)

    /*! \brief Getter function for the property with the same name */
    bool hasLocalFile() const { return QFile::exists(_fileName); }

    /*! \brief Short info text describing the state of the downloadable

    The text is typically of the form

    - "downloading … 47% complete"

    - "installed • 203 kB • update available"

    It might be translated to the local language.
  */
    Q_PROPERTY(QString infoText READ infoText NOTIFY infoTextChanged)

    /*! \brief Getter function for the property with the same name */
    QString infoText() const;

    /*! \brief Content of the local file

    This convenience property that holds the content of the local file, or a
    null QByteArray, if no local file exists */
    Q_PROPERTY(QByteArray localFileContent READ localFileContent NOTIFY localFileChanged)

    /*! \brief Getter function for the property with the same name */
    QByteArray localFileContent() const;

    /*! \brief Modification date of the remote file

    If the modification date of the remote file is not known, the property holds
    an invalid QDateTime. The property can be set with the method
    checkForUpdate(), or directly written to.
   */
    Q_PROPERTY(QDateTime remoteFileDate READ remoteFileDate WRITE setRemoteFileDate NOTIFY remoteFileInfoChanged)

    /*! \brief Setter function for the property with the same name */
    void setRemoteFileDate(const QDateTime& dt);
    
    /*! \brief Getter function for the property with the same name */
    QDateTime remoteFileDate() const { return _remoteFileDate; }

    /*! \brief Size of the remote file

    If the size date of the remote file is not known, the property holds an
    -1. The property can be set with the method checkForUpdate(), or directly
    written to.
   */
    Q_PROPERTY(qint64 remoteFileSize READ remoteFileSize WRITE setRemoteFileSize NOTIFY remoteFileInfoChanged)

    /*! \brief Getter function for the property with the same name */
    qint64 remoteFileSize() const { return _remoteFileSize; }

    /*! \brief Setter function for the property with the same name */
    void setRemoteFileSize(qint64 size);

    /*! \brief Name of the directory containing the file on the server
   *
   * This property is a convenience for accessing the part of the URL that
   * contains the directory name of the downloadable file. This is used e.g. for
   * files containing aviation maps, where the directory name is the name of the
   * associated geographic continent.  The GUI uses this to generate section
   * headings in the list of downloadable aviation maps.
   */
    Q_PROPERTY(QString section READ section CONSTANT)

    /*! \brief Getter function for the property with the same name */
    QString section() const { return _url.path().section("/", -2, -2); }

    /*! \brief Indicates if the local file is known to be updatable

    This property is true if all of the following conditions are met.

    - No download is in progress

    - The local file exists

    - The file on the remote server is known to differ from the local file in
      file size, or if the modification date of the file on the remote server
      is newer than the modification date of the local file.

    @warning The notification signal is not emitted when another process touches
    the local file.
  */
    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    /*! \brief Getter function for the property with the same name */
    bool updatable() const;

    /*! \brief URL, as set in the constructor */
    Q_PROPERTY(QUrl url READ url CONSTANT)

    /*! \brief Getter function for the property with the same name */
    QUrl url() const { return _url; }

public slots:
    /*! \brief The convenience method deletes the local file.

    This convenience method deletes the local file. The singals
    aboutToChangeLocalFile() and localFileChanged() are emitted appropriately.
   */
    void deleteLocalFile();

    /*! \brief Initiate a download

    Initiate an asynchronous download of the remote file. If a download is
    already in progress, nothing will happen.  Otherwise, the following will
    take place.
    
    -# Data is retrieved from the remote server and stored in temporary
       file. The signal downloadProgress() will be emitted regularly.

    -# In case of an error, the signal error() is emitted and the download
       stops.

    -# Optionally, the download can be stopped using the method
       stopFileDownload().

    Once all data has been downloaded successfully to the temporary file, the
    process continues as follows.
    
    -# The signal aboutToChangeLocalFile() is emitted. As the name suggests,
       this indicates that the local file is about to change and that it should
       not be used anymore.

    -# The local file is overwritten by the newly downloaded data.

    -# The signal downloadFinished() is emitted to indicate that the file is
       again ready to be used.
  */
    void startFileDownload();

    /*! \brief Contacts the server and downloads information about the remote file

    This method contacts the remote server, retrieves information about the
    remote file and updates the properties remoteFileDate and remoteFileSize
    accordingly.  This method runs asynchronously, and emits the signal
    remoteFileInfoChanged() if the data really changes.  If a request is already
    running, this method does nothing.

    @warning This method fails silently if the server cannot be contacted or if
    the server is unable to provide the requested data.
  */
    void startRemoteFileInfoDownload();

    /*! \brief Stops download process

    This method stops the currenly running download process gracefully and
    deletes any partially downloaded data. No signal will be emitted.  If no
    download is in progress, nothing will happen.
   */
    void stopFileDownload();

signals:
    /*! \brief Warning that local file is about to change

    This signal is emitted once the download finished, just before the local
    file is overwritten with new data. It indicates that all users should stop
    using the file immediately. This signal is always followed by the signal
    localFileChanged(), which indicates that the local file can be used again.

    @see localFileChanged()
   */
    void aboutToChangeLocalFile();

    /*! \brief Notifier signal for property downloading */
    void downloadingChanged();

    /*! \brief Download progress

    While the download process is running, this signal is emitted at regular
    intervals, so the user can learn about the progress of the operation.
    
    @param percentage An integer between 0 and 100
  */
    void downloadProgressChanged(int percentage);

    /*! \brief Download error

    This signal is emitted if the download process fails for whatever
    reason. Once the signal is emitted, the download process is deleted and no
    further actions will take place. The local file will not be touched.
    
    @param objectName Name of this QObject, as obtained by the method
    objectName()

    @param message A brief error message of the form "the requested resource is
    no longer available at the server", possibly translated.
   */
    void error(QString objectName, QString message);

    /*! \brief Notifier signal for the property infoText */
    void infoTextChanged();

    /*! \brief Notifier signal for the properties hasLocalFile and localFileContent

    This signal indicates that the local file has changed, and is ready to be
    used by clients if it exists.

    @see aboutToChangeLocalFile()
   */
    void localFileChanged();

    /*! \brief Notifier signal for the properties remoteFileDate and remoteFileSize

    This signal is emitted once one of the properties remoteFileDate and
    remoteFileSize changes, either in response to a use of the setter methods,
    or because downloadRemoteFileData() has been called and data has been
    retrieved from the server.
   */
    void remoteFileInfoChanged();

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged();

private slots:
    // Called when an error occurs during the download of the remote file, this
    // method deletes QTemporaryFile _tmpFile, emits the signal error() and
    // deletes _networkReplyDownload by calling deleteLater.  Connected to
    // &QNetworkReply::error of _networkReplyDownload.
    void downloadFileErrorReceiver(QNetworkReply::NetworkError code);

    // Called once download of the remote file is finished, this method overwrites
    // the local file. It deletes the QTemporaryFile _tmpFile, and deletes
    // _networkReplyDownload by calling deleteLater. Connected to
    // &QNetworkReply::finished of _networkReplyDownload.
    void downloadFileFinished();

    // Called during the download of the remote file, this method emits the signal
    // downloadProgress().  Connected to &QNetworkReply::downloadProgress of
    // _networkReplyDownload.
    void downloadFileProgressReceiver(qint64 bytesReceived, qint64 bytesTotal);

    // Called during the download of the remote file, this method reads all the
    // data that has been downloaded so far and stores it in the QTemporaryFile
    // _tmpFile.  Connected to &QNetworkReply::readyRead of _networkReplyDownload.
    void downloadFilePartialDataReceiver();

    // Called once download of the remote file header data is finished, this
    // method updates the properties remoteFileDate and remoteFileSize, and
    // _networkReplyDownloadHeader by calling deleteLater. Connected to
    // &QNetworkReply::finished of _networkReplyDownloadHeader.
    void downloadHeaderFinished();

private:
    // Pointer the QNetworkAccessManager that will be used for all the downloading
    QPointer<QNetworkAccessManager> _networkAccessManager;

    // This member holds the download progress.
    int _downloadProgress {0};

    // NetworkReply for downloading of remote file data. Set to nullptr when no
    // download is in progress.
    QPointer<QNetworkReply> _networkReplyDownloadFile;

    // NetworkReply for downloading of remote file header. Set to nullptr when no
    // download is in progress.
    QPointer<QNetworkReply> _networkReplyDownloadHeader;

    // Temporary file for storing partiall data when downloading the remote
    // file. Set to nullptr when no download is in progress.
    QPointer<QSaveFile> _saveFile;

    // URL of the remote file, as set in the constructor
    QUrl _url;

    // Name of the local file, as set in the constructor
    QString _fileName;

    // Modification date of the remote file, set directly via a setter method or
    // by calling downloadRemoteFileInfo().
    QDateTime _remoteFileDate;

    // Size of the remote file, set directly via a setter method or by calling
    // downloadRemoteFileInfo().
    qint64   _remoteFileSize {-1};
};

#endif // DOWNLOADABLE_H
