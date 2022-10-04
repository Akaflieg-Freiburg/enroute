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

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>

#include "geomaps/MBTILES.h"
#include "GlobalObject.h"
#include "Downloadable_SingleFile.h"
#include "Settings.h"


DataManagement::Downloadable_SingleFile::Downloadable_SingleFile(QUrl url, const QString &fileName, QObject *parent)
    : Downloadable_Abstract(parent), m_url(std::move(url))
{

    // Paranoid safety checks
    Q_ASSERT(!fileName.isEmpty());

    QFileInfo info(fileName);
    m_fileName = info.absoluteFilePath();

    connect(this, &Downloadable_SingleFile::remoteFileDateChanged, this, &Downloadable_SingleFile::infoTextChanged);
    connect(this, &Downloadable_SingleFile::remoteFileSizeChanged, this, &Downloadable_SingleFile::infoTextChanged);
    connect(this, &Downloadable_SingleFile::hasFileChanged, this, &Downloadable_SingleFile::infoTextChanged);
    connect(this, &Downloadable_SingleFile::hasFileChanged, this, &Downloadable_SingleFile::filesChanged);
    connect(this, &Downloadable_SingleFile::fileContentChanged, this, &Downloadable_SingleFile::infoTextChanged);
    connect(this, &Downloadable_SingleFile::downloadingChanged, this, &Downloadable_SingleFile::infoTextChanged);
    connect(this, &Downloadable_SingleFile::downloadProgressChanged, this, &Downloadable_SingleFile::infoTextChanged);

    // Wire up signals
    connect(this, &Downloadable_SingleFile::fileContentChanged, this, &Downloadable_Abstract::descriptionChanged);

    // set m_contentType
    {
        QString tmpName = m_url.path();
        if (tmpName.isEmpty())
        {
            tmpName = m_fileName;
        }

        if (tmpName.endsWith(QLatin1String("geojson")))
        {
            m_contentType = AviationMap;
        }
        else if (tmpName.endsWith(QLatin1String("mbtiles")))
        {
            m_contentType = BaseMapVector;
        }
        else if (tmpName.endsWith(QLatin1String("raster")))
        {
            m_contentType = BaseMapRaster;
        }
        else if (tmpName.endsWith(QLatin1String("terrain")))
        {
            m_contentType = TerrainMap;
        }
    }
}


DataManagement::Downloadable_SingleFile::~Downloadable_SingleFile()
{
    // Free all ressources
    delete m_networkReplyDownloadFile;
    delete m_networkReplyDownloadHeader;
    delete m_saveFile;
}



//
// Getter Methods
//

auto DataManagement::Downloadable_SingleFile::description() -> QString
{
    QFileInfo fi(m_fileName);
    if (!fi.exists())
    {
        return tr("No information available.");
    }
    QString result = QStringLiteral("<table><tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr><tr><td><strong>%3 :&nbsp;&nbsp;</strong></td><td>%4</td></tr></table>")
                         .arg(tr("Installed"),
                              fi.lastModified().toUTC().toString(),
                              tr("File Size"),
                              QLocale::system().formattedDataSize(fi.size(), 1, QLocale::DataSizeSIFormat));

    // Extract infomation from GeoJSON
    if (m_fileName.endsWith(u".geojson"))
    {
        QLockFile lockFile(m_fileName + ".lock");
        lockFile.lock();
        QFile file(m_fileName);
        file.open(QIODevice::ReadOnly);
        auto document = QJsonDocument::fromJson(file.readAll());
        file.close();
        lockFile.unlock();
        QString concatInfoString = document.object()[QStringLiteral("info")].toString();
        if (!concatInfoString.isEmpty())
        {
            result += "<p>" + tr("The map data was compiled from the following sources.") + "</p><ul>";
            auto infoStrings = concatInfoString.split(QStringLiteral(";"));
            foreach (auto infoString, infoStrings)
                result += "<li>" + infoString + "</li>";
            result += u"</ul>";
        }
    }

    // Extract infomation from MBTILES
    if (m_fileName.endsWith(u".mbtiles") || m_fileName.endsWith(u".raster") || m_fileName.endsWith(u".terrain"))
    {
        GeoMaps::MBTILES mbtiles(m_fileName);
        result += "<p>" + mbtiles.info() + "</p>";
    }

    // Extract infomation from text file - this is simply the first line
    if (m_fileName.endsWith(u".txt"))
    {
        // Open file and read first line
        QFile dataFile(m_fileName);
        dataFile.open(QIODevice::ReadOnly);
        auto description = dataFile.readLine();
        result += QStringLiteral("<p>%1</p>").arg(QString::fromLatin1(description));
    }

    return result;
}


auto DataManagement::Downloadable_SingleFile::fileContent() const -> QByteArray
{
    // Paranoid safety checks
    Q_ASSERT(!m_fileName.isEmpty());

    QFile file(m_fileName);
    if (!file.exists())
    {
        return {};
    }

    QLockFile lockFile(m_fileName + ".lock");
    lockFile.lock();
    file.open(QIODevice::ReadOnly);
    QByteArray result = file.readAll();
    file.close();
    lockFile.unlock();

    return result;
}


auto DataManagement::Downloadable_SingleFile::files() -> QStringList
{
    if (!hasFile())
    {
        return {};
    }
    return {m_fileName};
}


auto DataManagement::Downloadable_SingleFile::infoText() -> QString
{
    if (downloading())
    {
        return tr("downloading … %1% complete").arg(m_downloadProgress);
    }

    QString displayText;
    if (hasFile())
    {
        QFileInfo info(m_fileName);
        displayText = tr("installed • %1")
                          .arg(QLocale::system().formattedDataSize(info.size(), 1,
                                                                   QLocale::DataSizeSIFormat));

        if (updateSize() != 0)
        {
            displayText += " • " + tr("update available");
        }
        if (!url().isValid())
        {
            displayText += " • " + tr("manually imported");
        }
    }
    else
    {
        displayText += tr("not installed") + " • ";
        if (remoteFileSize() >= 0)
        {
            displayText += QStringLiteral("%1").arg(QLocale::system().formattedDataSize(
                remoteFileSize(), 1, QLocale::DataSizeSIFormat));
        }
        else
        {
            displayText += tr("file size unknown");
        }
    }
    return displayText;
}


auto DataManagement::Downloadable_SingleFile::updateSize() -> qint64
{
    if (downloading())
    {
        return 0;
    }
    if (!QFile::exists(m_fileName))
    {
        return 0;
    }

    QFileInfo info(m_fileName);
    if (m_remoteFileDate.isValid() && (info.lastModified() < m_remoteFileDate))
    {
        return m_remoteFileSize;
    }
    return 0;
}



//
// Setter Methods
//

void DataManagement::Downloadable_SingleFile::setRemoteFileDate(const QDateTime &date)
{
    // Do nothing if old and new data agrees
    if (date == m_remoteFileDate)
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();

    m_remoteFileDate = date;

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
    emit remoteFileDateChanged();
}


void DataManagement::Downloadable_SingleFile::setRemoteFileSize(qint64 size)
{
    // Paranoid safety checks
    Q_ASSERT(size >= -1);
    if (size < -1)
    {
        size = -1;
    }

    // Do nothing if old and new data agrees
    if (size == m_remoteFileSize)
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();

    m_remoteFileSize = size;

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
    emit remoteFileSizeChanged();
}



//
// Methods
//

void DataManagement::Downloadable_SingleFile::deleteFiles()
{
    // If the local file does not exist, there is nothing to do
    if (!QFile::exists(m_fileName))
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();

    emit aboutToChangeFile(m_fileName);
    QLockFile lockFile(m_fileName + ".lock");
    lockFile.lock();
    QFile::remove(m_fileName);
    lockFile.unlock();
    emit hasFileChanged();
    emit fileContentChanged();

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
}


void DataManagement::Downloadable_SingleFile::startDownload()
{

    // Do not begin a new download if one is already running
    if (downloading())
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();
    auto oldDownloadProgress = m_downloadProgress;
    auto oldIsDownloading = downloading();

    // Clear temporary file
    delete m_saveFile;

    // Create directory that will hold the local file, if it does not yet exist
    QDir dir(QFileInfo(m_fileName).dir());
    if (!dir.exists())
    {
        dir.mkpath(QStringLiteral("."));
    }

    // Copy the temporary file to the local file
    m_saveFile = new QSaveFile(m_fileName, this);
    m_saveFile->open(QIODevice::WriteOnly);

    // Start download
    QNetworkRequest request(m_url);
    m_networkReplyDownloadFile = GlobalObject::networkAccessManager()->get(request);
    connect(m_networkReplyDownloadFile, &QNetworkReply::finished, this, &Downloadable_SingleFile::downloadFileFinished);
    connect(m_networkReplyDownloadFile, &QNetworkReply::readyRead, this, &Downloadable_SingleFile::downloadFilePartialDataReceiver);
    connect(m_networkReplyDownloadFile, &QNetworkReply::downloadProgress, this, &Downloadable_SingleFile::downloadFileProgressReceiver);
    connect(m_networkReplyDownloadFile, &QNetworkReply::errorOccurred, this, &Downloadable_SingleFile::downloadFileErrorReceiver);
    m_downloadProgress = 0;

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
    if (m_downloadProgress != oldDownloadProgress)
    {
        emit downloadProgressChanged(m_downloadProgress);
    }
    if (downloading() != oldIsDownloading)
    {
        emit downloadingChanged();
    }
}


void DataManagement::Downloadable_SingleFile::startInfoDownload()
{

    // Do not start a new check if an old one is still running
    if (!m_networkReplyDownloadHeader.isNull())
    {
        return;
    }

    // Start the download process for the remote file info
    m_networkReplyDownloadHeader = GlobalObject::networkAccessManager()->head(QNetworkRequest(m_url));
    connect(m_networkReplyDownloadHeader, &QNetworkReply::finished, this,
            &Downloadable_SingleFile::downloadHeaderFinished);
}


void DataManagement::Downloadable_SingleFile::stopDownload()
{

    // Do stop a new download if none is already running
    if (!downloading())
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();

    // Stop the download
    m_networkReplyDownloadFile->deleteLater();
    m_networkReplyDownloadFile = nullptr;
    delete m_saveFile;

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
    emit downloadingChanged();
}


void DataManagement::Downloadable_SingleFile::update()
{
    if (updateSize() != 0)
    {
        startDownload();
    }
}



//
// Private Methods
//

void DataManagement::Downloadable_SingleFile::downloadFileErrorReceiver(QNetworkReply::NetworkError code)
{

    // Do nothing if there is no error
    if (code == QNetworkReply::NoError)
    {
        return;
    }

    // Stop the download
    stopDownload();

    // Do not do anything about SSL errors; this has already been handled by the SSLErrorHandler
    if ((code == QNetworkReply::SslHandshakeFailedError) &&
        !GlobalObject::settings()->ignoreSSLProblems())
    {
        return;
    }

    // Come up with a message…
    QString message;
    switch (code)
    {
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
        message += tr("the SSL/TLS handshake failed and the encrypted channel could not be established.");
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


void DataManagement::Downloadable_SingleFile::downloadFileFinished()
{
    // Paranoid safety checks
    //  Q_ASSERT(!_networkReplyDownloadFile.isNull() && !_tmpFile.isNull());
    if (m_networkReplyDownloadFile.isNull() || m_saveFile.isNull())
    {
        stopDownload();
        return;
    }
    if (m_networkReplyDownloadFile->error() != QNetworkReply::NoError)
    {
        stopDownload();
        return;
    }

    // Read the last remaining bits of data, then close the temporary file
    downloadFilePartialDataReceiver();

    // Download is now finished to 100%
    if (m_downloadProgress != 100)
    {
        m_downloadProgress = 100;
        emit downloadProgressChanged(m_downloadProgress);
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();
    bool oldHasLocalFile = hasFile();

    // Copy the temporary file to the local file
    emit aboutToChangeFile(m_fileName);
    QLockFile lockFile(m_fileName + ".lock");
    lockFile.lock();
    m_saveFile->commit();
    lockFile.unlock();
    emit fileContentChanged();

    // Delete the data structures for the download
    delete m_saveFile;
    m_networkReplyDownloadFile->deleteLater();
    m_networkReplyDownloadFile = nullptr;

    // Emit signals as appropriate
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
    if (oldHasLocalFile != hasFile())
    {
        emit hasFileChanged();
    }
    emit downloadingChanged();
}


void DataManagement::Downloadable_SingleFile::downloadFileProgressReceiver(qint64 bytesReceived, qint64 bytesTotal)
{
    auto oldDownloadProgress = m_downloadProgress;

    // If the content is compressed, then Qt does not know the total size and will set 'bytesTotal' to -1. In that case, the number _remoteFileSize might be a better estimate.
    if ((bytesTotal < 0) && (m_remoteFileSize > 0))
    {
        bytesTotal = m_remoteFileSize;
    }
    if (bytesTotal <= 0)
    {
        m_downloadProgress = 0;
    }
    else
    {
        m_downloadProgress = qRound((100.0 * static_cast<double>(bytesReceived)) / static_cast<double>(bytesTotal));
    }

    // Whatever, make sure that the number computed is between 0 and 100
    m_downloadProgress = qBound(0, m_downloadProgress, 100);

    if (m_downloadProgress != oldDownloadProgress)
    {
        emit downloadProgressChanged(m_downloadProgress);
    }
}


void DataManagement::Downloadable_SingleFile::downloadFilePartialDataReceiver()
{
    // Paranoid safety checks
    if (m_networkReplyDownloadFile.isNull() || m_saveFile.isNull())
    {
        stopDownload();
        return;
    }
    if (m_networkReplyDownloadFile->error() != QNetworkReply::NoError)
    {
        return;
    }

    // Write all available data to the temporary file
    m_saveFile->write(m_networkReplyDownloadFile->readAll());
}


void DataManagement::Downloadable_SingleFile::downloadHeaderFinished()
{
    // Paranoid safety checks
    Q_ASSERT(!m_networkReplyDownloadHeader.isNull());
    if (m_networkReplyDownloadHeader.isNull())
    {
        return;
    }
    if (m_networkReplyDownloadHeader->error() != QNetworkReply::NoError)
    {
        return;
    }

    // Save old value to see if anything changed
    auto oldUpdateSize = updateSize();

    // Update remote file information
    auto old_remoteFileDate = m_remoteFileDate;
    auto old_remoteFileSize = m_remoteFileSize;
    m_remoteFileDate =
        m_networkReplyDownloadHeader->header(QNetworkRequest::LastModifiedHeader).toDateTime();
    m_remoteFileSize =
        m_networkReplyDownloadHeader->header(QNetworkRequest::ContentLengthHeader).toLongLong();

    // Emit signals as appropriate
    if (m_remoteFileDate != old_remoteFileDate)
    {
        emit remoteFileDateChanged();
    }
    if (m_remoteFileSize != old_remoteFileSize)
    {
        emit remoteFileSizeChanged();
    }
    if (oldUpdateSize != updateSize())
    {
        emit updateSizeChanged();
    }
}
