/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QLockFile>
#include <utility>

#include "Downloadable.h"

GeoMaps::Downloadable::Downloadable(QUrl url, const QString &fileName,
                           QNetworkAccessManager *networkAccessManager, QObject *parent)
    : QObject(parent), _networkAccessManager(networkAccessManager), _url(std::move(url)) {
    // Paranoid safety checks
    Q_ASSERT(networkAccessManager != nullptr);
    Q_ASSERT(!fileName.isEmpty());

    QFileInfo info(fileName);
    _fileName = info.absoluteFilePath();

    connect(this, &Downloadable::remoteFileDateChanged, this, &Downloadable::infoTextChanged);
    connect(this, &Downloadable::remoteFileSizeChanged, this, &Downloadable::infoTextChanged);
    connect(this, &Downloadable::hasFileChanged, this, &Downloadable::infoTextChanged);
    connect(this, &Downloadable::fileContentChanged, this, &Downloadable::infoTextChanged);
    connect(this, &Downloadable::downloadingChanged, this, &Downloadable::infoTextChanged);
    connect(this, &Downloadable::downloadProgressChanged, this, &Downloadable::infoTextChanged);
}


GeoMaps::Downloadable::~Downloadable() {
    // Free all ressources
    delete _networkReplyDownloadFile;
    delete _networkReplyDownloadHeader;
    delete _saveFile;
}


auto GeoMaps::Downloadable::infoText() const -> QString {
    if (downloading()) {
        return tr("downloading … %1% complete").arg(_downloadProgress);
    }

    QString displayText;
    if (hasFile()) {
        QFileInfo info(_fileName);
        displayText = tr("installed • %1")
                .arg(QLocale::system().formattedDataSize(info.size(), 1,
                                                         QLocale::DataSizeSIFormat));

        if (updatable()) {
            displayText += " • " + tr("update available");
        }
        if (!url().isValid()) {
            displayText += " • " + tr("no longer supported");
        }
    } else {
        displayText += tr("not installed") + " • ";
        if (remoteFileSize() >= 0) {
            displayText += QString("%1").arg(QLocale::system().formattedDataSize(
                                                 remoteFileSize(), 1, QLocale::DataSizeSIFormat));
        } else {
            displayText += tr("file size unknown");
        }
    }
    return displayText;
}


auto GeoMaps::Downloadable::fileContent() const -> QByteArray {
    // Paranoid safety checks
    Q_ASSERT(!_fileName.isEmpty());

    QFile file(_fileName);
    if (!file.exists()) {
        return QByteArray();
    }

    QLockFile lockFile(_fileName + ".lock");
    lockFile.lock();
    file.open(QIODevice::ReadOnly);
    QByteArray result = file.readAll();
    file.close();
    lockFile.unlock();

    return result;
}


void GeoMaps::Downloadable::setRemoteFileDate(const QDateTime &date) {
    // Do nothing if old and new data agrees
    if (date == _remoteFileDate) {
        return;
    }

    // Save old value to see if anything changed
    bool oldUpdatable = updatable();

    _remoteFileDate = date;

    // Emit signals as appropriate
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
    emit remoteFileDateChanged();
}


void GeoMaps::Downloadable::setRemoteFileSize(qint64 size) {
    // Paranoid safety checks
    Q_ASSERT(size >= -1);
    if (size < -1) {
        size = -1;
    }

    // Do nothing if old and new data agrees
    if (size == _remoteFileSize) {
        return;
    }

    // Save old value to see if anything changed
    bool oldUpdatable = updatable();

    _remoteFileSize = size;

    // Emit signals as appropriate
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
    emit remoteFileSizeChanged();
}


void GeoMaps::Downloadable::setSection(const QString& sectionName)
{
    if (sectionName == _section) {
        return;
    }
    _section = sectionName;
    emit sectionChanged();
}


auto GeoMaps::Downloadable::updatable() const -> bool {
    if (downloading()) {
        return false;
    }
    if (!QFile::exists(_fileName)) {
        return false;
    }

    QFileInfo info(_fileName);
    return _remoteFileDate.isValid() && (info.lastModified() < _remoteFileDate);
}


void GeoMaps::Downloadable::deleteFile() {
    // If the local file does not exist, there is nothing to do
    if (!QFile::exists(_fileName)) {
        return;
    }

    // Save old value to see if anything changed
    bool oldUpdatable = updatable();

    emit aboutToChangeFile(_fileName);
    QLockFile lockFile(_fileName + ".lock");
    lockFile.lock();
    QFile::remove(_fileName);
    lockFile.unlock();
    emit hasFileChanged();
    emit fileContentChanged();

    // Emit signals as appropriate
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
}


void GeoMaps::Downloadable::startInfoDownload() {
    // Paranoid safety checks
    Q_ASSERT(!_networkAccessManager.isNull());
    if (_networkAccessManager.isNull()) {
        return;
    }

    // Do not start a new check if an old one is still running
    if (!_networkReplyDownloadHeader.isNull()) {
        return;
    }

    // Start the download process for the remote file info
    _networkReplyDownloadHeader = _networkAccessManager->head(QNetworkRequest(_url));
    connect(_networkReplyDownloadHeader, &QNetworkReply::finished, this,
            &Downloadable::downloadHeaderFinished);
}


void GeoMaps::Downloadable::startFileDownload() {
    // Paranoid safety checks
    Q_ASSERT(!_networkAccessManager.isNull());
    if (_networkAccessManager.isNull()) {
        return;
    }

    // Do not begin a new download if one is already running
    if (downloading()) {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdatable = updatable();
    auto oldDownloadProgress =_downloadProgress;
    auto oldIsDownloading = downloading();

    // Clear temporary file
    delete _saveFile;

    // Create directory that will hold the local file, if it does not yet exist
    QDir dir(QFileInfo(_fileName).dir());
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Copy the temporary file to the local file
    _saveFile = new QSaveFile(_fileName, this);
    _saveFile->open(QIODevice::WriteOnly);

    // Start download
    QNetworkRequest request(_url);
    _networkReplyDownloadFile = _networkAccessManager->get(request);
    connect(_networkReplyDownloadFile, &QNetworkReply::finished, this, &Downloadable::downloadFileFinished);
    connect(_networkReplyDownloadFile, &QNetworkReply::readyRead, this, &Downloadable::downloadFilePartialDataReceiver);
    connect(_networkReplyDownloadFile, &QNetworkReply::downloadProgress, this, &Downloadable::downloadFileProgressReceiver);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(_networkReplyDownloadFile, &QNetworkReply::errorOccurred, this, &Downloadable::downloadFileErrorReceiver);
#else
    connect(_networkReplyDownloadFile, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Downloadable::downloadFileErrorReceiver);
#endif
    _downloadProgress = 0;

    // Emit signals as appropriate
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
    if (_downloadProgress != oldDownloadProgress) {
        emit downloadProgressChanged(_downloadProgress);
    }
    if (downloading() != oldIsDownloading) {
        emit downloadingChanged();
    }
}


void GeoMaps::Downloadable::stopFileDownload() {
    // Paranoid safety checks
    Q_ASSERT(!_networkAccessManager.isNull());
    if (_networkAccessManager.isNull()) {
        return;
    }

    // Do stop a new download if none is already running
    if (!downloading()) {
        return;
    }

    // Save old value to see if anything changed
    bool oldUpdatable = updatable();

    // Stop the download
    _networkReplyDownloadFile->deleteLater();
    _networkReplyDownloadFile = nullptr;
    delete _saveFile;

    // Emit signals as appropriate
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
    emit downloadingChanged();
}


void GeoMaps::Downloadable::downloadFileErrorReceiver(QNetworkReply::NetworkError code) {
    // Do nothing if there is no error
    if (code == QNetworkReply::NoError) {
        return;
    }

    // Stop the download
    stopFileDownload();

    // Come up with a message…
    QString message;
    switch (code) {
    case QNetworkReply::ConnectionRefusedError:
        message +=
                tr("the remote server refused the connection (the server is not accepting requests)");
        break;

    case QNetworkReply::RemoteHostClosedError:
        message += tr("the remote server closed the connection prematurely, before the entire "
                      "reply was received and processed");
        break;

    case QNetworkReply::HostNotFoundError:
        message += tr("the remote host name was not found (invalid hostname)");
        break;

    case QNetworkReply::TimeoutError:
        message += tr("the connection to the remote server timed out");
        break;

    case QNetworkReply::OperationCanceledError:
        message +=
                tr("the operation was canceled via calls to abort() or close() before it was finished");
        break;

    case QNetworkReply::SslHandshakeFailedError:
        message += tr("the SSL/TLS handshake failed and the encrypted channel could not be "
                      "established. The sslErrors() signal should have been emitted");
        break;

    case QNetworkReply::TemporaryNetworkFailureError:
        message += tr("the connection was broken due to disconnection from the network");
        break;

    case QNetworkReply::NetworkSessionFailedError:
        message += tr("the connection was broken due to disconnection from the network or failure "
                      "to start the network");
        break;

    case QNetworkReply::BackgroundRequestNotAllowedError:
        message += tr("the background request is not currently allowed due to platform policy");
        break;

    case QNetworkReply::TooManyRedirectsError:
        message += tr("while following redirects, the maximum limit was reached");
        break;

    case QNetworkReply::InsecureRedirectError:
        message += tr("while following redirects, the network access API detected a redirect from "
                      "a encrypted protocol (https) to an unencrypted one (http)");
        break;

    case QNetworkReply::ProxyConnectionRefusedError:
        message += tr("the connection to the proxy server was refused (the proxy server is not "
                      "accepting requests)");
        break;

    case QNetworkReply::ProxyConnectionClosedError:
        message += tr("the proxy server closed the connection prematurely, before the entire reply "
                      "was received and processed");
        break;

    case QNetworkReply::ProxyNotFoundError:
        message += tr("the proxy host name was not found (invalid proxy hostname)");
        break;

    case QNetworkReply::ProxyTimeoutError:
        message += tr("the connection to the proxy timed out or the proxy did not reply in time to "
                      "the request sent");
        break;

    case QNetworkReply::ProxyAuthenticationRequiredError:
        message += tr("the proxy requires authentication in order to honour the request but did "
                      "not accept any credentials offered (if any)");
        break;

    case QNetworkReply::ContentAccessDenied:
        message += tr("the access to the remote content was denied (similar to HTTP error 403)");
        break;

    case QNetworkReply::ContentOperationNotPermittedError:
        message += tr("the operation requested on the remote content is not permitted");
        break;

    case QNetworkReply::ContentNotFoundError:
        message += tr("the remote content was not found at the server (similar to HTTP error 404)");
        break;

    case QNetworkReply::AuthenticationRequiredError:
        message += tr("the remote server requires authentication to serve the content but the "
                      "credentials provided were not accepted (if any)");
        break;

    case QNetworkReply::ContentReSendError:
        message += tr("the request needed to be sent again, but this failed for example because "
                      "the upload data could not be read a second time");
        break;

    case QNetworkReply::ContentConflictError:
        message += tr("the request could not be completed due to a conflict with the current state "
                      "of the resource");
        break;

    case QNetworkReply::ContentGoneError:
        message += tr("the requested resource is no longer available at the server");
        break;

    case QNetworkReply::InternalServerError:
        message += tr("the server encountered an unexpected condition which prevented it from "
                      "fulfilling the request");
        break;

    case QNetworkReply::OperationNotImplementedError:
        message +=
                tr("the server does not support the functionality required to fulfill the request");
        break;

    case QNetworkReply::ServiceUnavailableError:
        message += tr("the server is unable to handle the request at this time");
        break;

    case QNetworkReply::ProtocolUnknownError:
        message +=
                tr("the Network Access API cannot honor the request because the protocol is not known");
        break;

    case QNetworkReply::ProtocolInvalidOperationError:
        message += tr("the requested operation is invalid for this protocol");
        break;

    case QNetworkReply::UnknownNetworkError:
        message += tr("an unknown network-related error was detected");
        break;

    case QNetworkReply::UnknownProxyError:
        message += tr("an unknown proxy-related error was detected");
        break;

    case QNetworkReply::UnknownContentError:
        message += tr("an unknown error related to the remote content was detected");
        break;

    case QNetworkReply::ProtocolFailure:
        message += tr("a breakdown in protocol was detected (parsing error, invalid or unexpected "
                      "responses, etc.)");
        break;

    case QNetworkReply::UnknownServerError:
        message += tr("an unknown error related to the server response was detected");
        break;

    default:
        message += tr("unknown");
        break;
    }

    // emit the message and return
    emit error(objectName(), message);
}


void GeoMaps::Downloadable::downloadFileFinished() {
    // Paranoid safety checks
    //  Q_ASSERT(!_networkReplyDownloadFile.isNull() && !_tmpFile.isNull());
    if (_networkReplyDownloadFile.isNull() || _saveFile.isNull()) {
        stopFileDownload();
        return;
    }
    if (_networkReplyDownloadFile->error() != QNetworkReply::NoError) {
        stopFileDownload();
        return;
    }

    // Read the last remaining bits of data, then close the temporary file
    downloadFilePartialDataReceiver();

    // Download is now finished to 100%
    if (_downloadProgress != 100) {
        _downloadProgress = 100;
        emit downloadProgressChanged(_downloadProgress);
    }

    // Save old value to see if anything changed
    bool oldIsUpdatable = updatable();
    bool oldHasLocalFile = hasFile();

    // Copy the temporary file to the local file
    emit aboutToChangeFile(_fileName);
    QLockFile lockFile(_fileName + ".lock");
    lockFile.lock();
    _saveFile->commit();
    lockFile.unlock();
    emit fileContentChanged();

    // Delete the data structures for the download
    delete _saveFile;
    _networkReplyDownloadFile->deleteLater();
    _networkReplyDownloadFile = nullptr;

    // Emit signals as appropriate
    if (oldIsUpdatable != updatable()) {
        emit updatableChanged();
    }
    if (oldHasLocalFile != hasFile()) {
        emit hasFileChanged();
    }
    emit downloadingChanged();
}


void GeoMaps::Downloadable::downloadFileProgressReceiver(qint64 bytesReceived, qint64 bytesTotal) {
    auto oldDownloadProgress = _downloadProgress;

    // If the content is compressed, then Qt does not know the total size and will set 'bytesTotal' to -1. In that case, the number _remoteFileSize might be a better estimate.
    if ((bytesTotal < 0) && (_remoteFileSize > 0)) {
        bytesTotal = _remoteFileSize;
    }
    if (bytesTotal <= 0) {
        _downloadProgress = 0;
    } else {
        _downloadProgress = qRound((100.0 * bytesReceived) / bytesTotal);
    }

    // Whatever, make sure that the number computed is between 0 and 100
    _downloadProgress = qBound( 0, _downloadProgress, 100);

    if (_downloadProgress != oldDownloadProgress) {
        emit downloadProgressChanged(_downloadProgress);
    }
}


void GeoMaps::Downloadable::downloadFilePartialDataReceiver() {
    // Paranoid safety checks
    if (_networkReplyDownloadFile.isNull() || _saveFile.isNull()) {
        stopFileDownload();
        return;
    }
    if (_networkReplyDownloadFile->error() != QNetworkReply::NoError) {
        return;
    }

    // Write all available data to the temporary file
    _saveFile->write(_networkReplyDownloadFile->readAll());
}


void GeoMaps::Downloadable::downloadHeaderFinished() {
    // Paranoid safety checks
    Q_ASSERT(!_networkReplyDownloadHeader.isNull());
    if (_networkReplyDownloadHeader.isNull()) {
        return;
    }
    if (_networkReplyDownloadHeader->error() != QNetworkReply::NoError) {
        return;
    }

    // Save old value to see if anything changed
    bool oldUpdatable = updatable();

    // Update remote file information
    auto old_remoteFileDate = _remoteFileDate;
    auto old_remoteFileSize = _remoteFileSize;
    _remoteFileDate =
            _networkReplyDownloadHeader->header(QNetworkRequest::LastModifiedHeader).toDateTime();
    _remoteFileSize =
            _networkReplyDownloadHeader->header(QNetworkRequest::ContentLengthHeader).toLongLong();

    // Emit signals as appropriate
    if (_remoteFileDate != old_remoteFileDate) {
        emit remoteFileDateChanged();
    }
    if (_remoteFileSize != old_remoteFileSize) {
        emit remoteFileSizeChanged();
    }
    if (oldUpdatable != updatable()) {
        emit updatableChanged();
    }
}
