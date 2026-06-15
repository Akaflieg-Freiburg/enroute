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

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQmlEngine>
#include <QStringList>

namespace Weather {

/*! \brief Downloads and exposes Météo-France forecast PNG maps from a local HTTP server.
 *
 * On refresh(), fetches index.json from the configured server, downloads any
 * new PNGs into the app cache, then scans the cache so QML can display them.
 * Falls back to whatever is already in cache on connection failure.
 */
class ForecastMapProvider : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum class Status {
        Idle,        ///< Ready, showing cached data
        Refreshing,  ///< Fetching index or downloading files
        Error        ///< Last refresh failed; showing stale cache
    };
    Q_ENUM(Status)

    explicit ForecastMapProvider(QObject* parent = nullptr);
    static Weather::ForecastMapProvider* create(QQmlEngine*, QJSEngine*);

    /*! \brief Application-wide singleton instance, for C++ callers */
    static Weather::ForecastMapProvider* instance() { return create(nullptr, nullptr); }

    //
    // Map data properties
    //

    Q_PROPERTY(QStringList timestamps READ timestamps NOTIFY timestampsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentTimestampLabel READ currentTimestampLabel NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentRainMap READ currentRainMap NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentCloudbaseMap READ currentCloudbaseMap NOTIFY currentIndexChanged)

    //
    // Layer metadata (from index.json)
    //

    /*! \brief Model run time that produced the current maps, e.g. "Thu 11 Jun 12:00 UTC" */
    Q_PROPERTY(QString referenceTimeLabel READ referenceTimeLabel NOTIFY metadataChanged)

    /*! \brief Units string for the rain layer, e.g. "mm/h" */
    Q_PROPERTY(QString rainUnits READ rainUnits NOTIFY metadataChanged)
    /*! \brief Color stops for the rain legend, as "#RRGGBBAA" strings, low→high */
    Q_PROPERTY(QStringList rainColors READ rainColors NOTIFY metadataChanged)
    /*! \brief vmin for rain color scale */
    Q_PROPERTY(double rainVmin READ rainVmin NOTIFY metadataChanged)
    /*! \brief vmax for rain color scale */
    Q_PROPERTY(double rainVmax READ rainVmax NOTIFY metadataChanged)

    /*! \brief Units string for the cloudbase layer, e.g. "m" */
    Q_PROPERTY(QString cloudbaseUnits READ cloudbaseUnits NOTIFY metadataChanged)
    /*! \brief Color stops for the cloudbase legend */
    Q_PROPERTY(QStringList cloudbaseColors READ cloudbaseColors NOTIFY metadataChanged)
    /*! \brief vmin for cloudbase color scale */
    Q_PROPERTY(double cloudbaseVmin READ cloudbaseVmin NOTIFY metadataChanged)
    /*! \brief vmax for cloudbase color scale */
    Q_PROPERTY(double cloudbaseVmax READ cloudbaseVmax NOTIFY metadataChanged)

    //
    // Sync / connectivity properties
    //

    /*! \brief Base URL of the forecast server, e.g. "http://192.168.1.10:8765/meteo" */
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)

    /*! \brief Current sync state */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /*! \brief Human-readable time since last successful refresh, e.g. "42 min ago" */
    Q_PROPERTY(QString lastRefreshLabel READ lastRefreshLabel NOTIFY lastRefreshLabelChanged)


    //
    // Getters
    //

    [[nodiscard]] QStringList timestamps() const { return m_timestamps; }
    [[nodiscard]] int currentIndex() const { return m_currentIndex; }
    [[nodiscard]] QString currentTimestampLabel() const;
    [[nodiscard]] QString currentRainMap() const;
    [[nodiscard]] QString currentCloudbaseMap() const;
    [[nodiscard]] QString serverUrl() const { return m_serverUrl; }
    [[nodiscard]] Status status() const { return m_status; }
    [[nodiscard]] QString lastRefreshLabel() const;

    [[nodiscard]] QString referenceTimeLabel() const;
    [[nodiscard]] QString rainUnits() const      { return m_rainUnits; }
    [[nodiscard]] QStringList rainColors() const { return m_rainColors; }
    [[nodiscard]] double rainVmin() const        { return m_rainVmin; }
    [[nodiscard]] double rainVmax() const        { return m_rainVmax; }
    [[nodiscard]] QString cloudbaseUnits() const      { return m_cloudbaseUnits; }
    [[nodiscard]] QStringList cloudbaseColors() const { return m_cloudbaseColors; }
    [[nodiscard]] double cloudbaseVmin() const        { return m_cloudbaseVmin; }
    [[nodiscard]] double cloudbaseVmax() const        { return m_cloudbaseVmax; }


    //
    // Setters
    //

    void setCurrentIndex(int idx);
    void setServerUrl(const QString& url);

    /*! \brief Fetch index.json from the server and download new maps. No-op if already refreshing. */
    Q_INVOKABLE void refresh();


signals:
    void timestampsChanged();
    void currentIndexChanged();
    void serverUrlChanged();
    void statusChanged();
    void lastRefreshLabelChanged();
    void metadataChanged();


private:
    void scan();
    void fetchIndex();
    void startDownloads(const QStringList& filenames, const QStringList& serverFiles);
    void finalizeRefresh(const QStringList& serverFiles);
    void abortRefresh();
    void parseMetadata(const QJsonObject& root);

    void setStatus(Status s);
    [[nodiscard]] QString cacheDir() const;

    QNetworkAccessManager m_nam;

    QString m_serverUrl;
    QString m_localScanDir;   ///< non-empty when serverUrl is a file:// URL
    Status  m_status          {Status::Idle};
    QDateTime m_lastRefreshTime;

    // Tracks in-flight downloads during a refresh cycle
    int         m_pendingDownloads {0};
    QStringList m_serverFileList;

    // Map data
    QStringList m_timestamps;
    int         m_currentIndex {0};
    QMap<QString, QString>                  m_rainMaps;
    QMap<QString, QString>                  m_cloudbaseMaps;

    // Layer metadata from index.json
    QString     m_referenceTime;
    QString     m_rainUnits       {QStringLiteral("mm/h")};
    QStringList m_rainColors;
    double      m_rainVmin        {0.1};
    double      m_rainVmax        {10.0};
    QString     m_cloudbaseUnits  {QStringLiteral("m")};
    QStringList m_cloudbaseColors;
    double      m_cloudbaseVmin   {0.0};
    double      m_cloudbaseVmax   {4000.0};
};

} // namespace Weather
