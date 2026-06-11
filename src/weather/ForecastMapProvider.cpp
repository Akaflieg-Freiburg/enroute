/***************************************************************************
 *   Copyright (C) 2026 by Quentin Bossard                                 *
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

#include "weather/ForecastMapProvider.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>

using namespace Qt::Literals::StringLiterals;

static const QRegularExpression re_rain(
    u"^rain_map_(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z?)\\.png$"_s);
static const QRegularExpression re_cloudbase(
    u"^cloudbase_map_(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z?)\\.png$"_s);
static const QRegularExpression re_wind(
    u"^wind_map_(\\d+(?:\\.\\d+)?)hPa_(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z?)\\.png$"_s);


// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Weather::ForecastMapProvider::ForecastMapProvider(QObject* parent)
    : QObject(parent)
{
    QSettings s;
    m_serverUrl       = s.value(u"ForecastMapProvider/serverUrl"_s).toString();
    m_lastRefreshTime = s.value(u"ForecastMapProvider/lastRefreshTime"_s).toDateTime();

    QDir().mkpath(cacheDir());
    scan();
}

Weather::ForecastMapProvider* Weather::ForecastMapProvider::create(QQmlEngine*, QJSEngine*)
{
    static ForecastMapProvider instance;
    return &instance;
}

QString Weather::ForecastMapProvider::cacheDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppCacheLocation)
           + u"/meteo_france"_s;
}


// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::refresh()
{
    if (m_status == Status::Refreshing)
        return;
    if (m_serverUrl.isEmpty()) {
        setStatus(Status::Error);
        return;
    }
    setStatus(Status::Refreshing);
    fetchIndex();
}


// ---------------------------------------------------------------------------
// Network: fetch index
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::fetchIndex()
{
    auto* reply = m_nam.get(QNetworkRequest(QUrl(m_serverUrl + u"/index.json"_s)));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            abortRefresh();
            return;
        }

        const auto doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) {
            abortRefresh();
            return;
        }

        QStringList serverFiles;
        for (const auto& v : doc.object()[u"files"_s].toArray())
            serverFiles << v.toString();

        // Only download files not already in cache
        const QDir dir(cacheDir());
        QStringList toDownload;
        for (const auto& fname : std::as_const(serverFiles))
            if (!dir.exists(fname))
                toDownload << fname;

        if (toDownload.isEmpty()) {
            finalizeRefresh(serverFiles);
            return;
        }

        startDownloads(toDownload, serverFiles);
    });
}


// ---------------------------------------------------------------------------
// Network: download individual files
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::startDownloads(
    const QStringList& filenames,
    const QStringList& serverFiles)
{
    m_pendingDownloads = filenames.size();
    m_serverFileList   = serverFiles;

    for (const auto& fname : filenames) {
        auto* reply = m_nam.get(QNetworkRequest(QUrl(m_serverUrl + u"/"_s + fname)));

        connect(reply, &QNetworkReply::finished, this, [this, reply, fname]() {
            reply->deleteLater();

            if (reply->error() == QNetworkReply::NoError) {
                // Atomic write: tmp → final, so a partial download never appears in scan()
                const QString tmp   = cacheDir() + u"/"_s + fname + u".tmp"_s;
                const QString final = cacheDir() + u"/"_s + fname;
                QFile f(tmp);
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(reply->readAll());
                    f.close();
                    QFile::rename(tmp, final);
                }
            }
            // Count down regardless — a missing file just won't appear in scan()
            if (--m_pendingDownloads == 0)
                finalizeRefresh(m_serverFileList);
        });
    }
}


// ---------------------------------------------------------------------------
// Post-refresh bookkeeping
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::finalizeRefresh(const QStringList& serverFiles)
{
    // Purge cache entries no longer on the server — only on a successful full refresh
    const QSet<QString> keep(serverFiles.begin(), serverFiles.end());
    QDir dir(cacheDir());
    for (const auto& name : dir.entryList(QStringList{u"*.png"_s}, QDir::Files))
        if (!keep.contains(name))
            dir.remove(name);
    // Clean up any leftover .tmp files from crashed downloads
    for (const auto& name : dir.entryList(QStringList{u"*.tmp"_s}, QDir::Files))
        dir.remove(name);

    m_lastRefreshTime = QDateTime::currentDateTimeUtc();
    QSettings().setValue(u"ForecastMapProvider/lastRefreshTime"_s, m_lastRefreshTime);

    setStatus(Status::Idle);
    emit lastRefreshLabelChanged();
    scan();
}

void Weather::ForecastMapProvider::abortRefresh()
{
    // Clean up any .tmp files from the aborted attempt
    QDir dir(cacheDir());
    for (const auto& name : dir.entryList(QStringList{u"*.tmp"_s}, QDir::Files))
        dir.remove(name);

    setStatus(Status::Error);
    scan(); // expose whatever is in cache
}


// ---------------------------------------------------------------------------
// Directory scan
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::scan()
{
    m_rainMaps.clear();
    m_cloudbaseMaps.clear();
    m_windMaps.clear();

    const QDir dir(cacheDir());
    for (const auto& name : dir.entryList(QStringList{u"*.png"_s}, QDir::Files)) {
        auto m = re_rain.match(name);
        if (m.hasMatch()) {
            m_rainMaps[m.captured(1)] = dir.absoluteFilePath(name);
            continue;
        }
        m = re_cloudbase.match(name);
        if (m.hasMatch()) {
            m_cloudbaseMaps[m.captured(1)] = dir.absoluteFilePath(name);
            continue;
        }
        m = re_wind.match(name);
        if (m.hasMatch())
            m_windMaps[m.captured(1)][m.captured(2)] = dir.absoluteFilePath(name);
    }

    // Union of all timestamps across all map types
    QSet<QString> tsSet;
    for (const auto& k : m_rainMaps.keys())      tsSet << k;
    for (const auto& k : m_cloudbaseMaps.keys()) tsSet << k;
    for (const auto& level : std::as_const(m_windMaps))
        for (const auto& k : level.keys())       tsSet << k;

    m_timestamps = tsSet.values();
    m_timestamps.sort();

    // Wind pressure levels sorted ascending (700 first = highest altitude)
    QStringList levels = m_windMaps.keys();
    std::sort(levels.begin(), levels.end(),
              [](const QString& a, const QString& b) { return a.toDouble() < b.toDouble(); });
    m_windPressureLevels = levels;

    if (m_currentWindPressureLevel.isEmpty() && !levels.isEmpty())
        m_currentWindPressureLevel = levels.first();

    m_currentIndex = qBound(0, m_currentIndex, qMax(0, int(m_timestamps.size()) - 1));

    emit timestampsChanged();
    emit currentIndexChanged();
    emit currentWindMapChanged();
}


// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------

void Weather::ForecastMapProvider::setStatus(Status s)
{
    if (m_status == s) return;
    m_status = s;
    emit statusChanged();
}

void Weather::ForecastMapProvider::setServerUrl(const QString& url)
{
    if (m_serverUrl == url) return;
    m_serverUrl = url;
    QSettings().setValue(u"ForecastMapProvider/serverUrl"_s, url);
    emit serverUrlChanged();
}

void Weather::ForecastMapProvider::setCurrentIndex(int idx)
{
    idx = qBound(0, idx, qMax(0, int(m_timestamps.size()) - 1));
    if (m_currentIndex == idx) return;
    m_currentIndex = idx;
    emit currentIndexChanged();
    emit currentWindMapChanged();
}

void Weather::ForecastMapProvider::setCurrentWindPressureLevel(const QString& level)
{
    if (m_currentWindPressureLevel == level) return;
    m_currentWindPressureLevel = level;
    emit currentWindMapChanged();
}


// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

QString Weather::ForecastMapProvider::lastRefreshLabel() const
{
    if (!m_lastRefreshTime.isValid())
        return u"Never"_s;
    const auto secs = m_lastRefreshTime.secsTo(QDateTime::currentDateTimeUtc());
    if (secs < 60)   return u"Just now"_s;
    if (secs < 3600) return QString::number(secs / 60) + u" min ago"_s;
    return QString::number(secs / 3600) + u" h ago"_s;
}

QString Weather::ForecastMapProvider::currentTimestampLabel() const
{
    if (m_timestamps.isEmpty() || m_currentIndex >= m_timestamps.size()) return {};
    const auto dt = QDateTime::fromString(m_timestamps[m_currentIndex], Qt::ISODate);
    if (!dt.isValid()) return m_timestamps[m_currentIndex];
    return dt.toUTC().toString(u"ddd dd MMM HH:mm"_s) + u" UTC"_s;
}

QString Weather::ForecastMapProvider::currentRainMap() const
{
    if (m_timestamps.isEmpty() || m_currentIndex >= m_timestamps.size()) return {};
    return m_rainMaps.value(m_timestamps[m_currentIndex]);
}

QString Weather::ForecastMapProvider::currentCloudbaseMap() const
{
    if (m_timestamps.isEmpty() || m_currentIndex >= m_timestamps.size()) return {};
    return m_cloudbaseMaps.value(m_timestamps[m_currentIndex]);
}

QString Weather::ForecastMapProvider::currentWindMap() const
{
    if (m_timestamps.isEmpty() || m_currentIndex >= m_timestamps.size()
            || m_currentWindPressureLevel.isEmpty()) return {};
    return m_windMaps.value(m_currentWindPressureLevel).value(m_timestamps[m_currentIndex]);
}
