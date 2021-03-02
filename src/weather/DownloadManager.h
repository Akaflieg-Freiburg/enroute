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

#pragma once

#include <QMap>
#include <QPointer>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

#include "Navigation_SatNav.h"
#include "weather/Station.h"

class Clock;
class FlightRoute;
class GeoMapProvider;
class GlobalSettings;

namespace Weather {

/*! \brief DownloadManager, weather service manager
 *
 * This class retrieves METAR/TAF weather reports from the "Aviation Weather
 * Center" at aviationweather.com, for all weather stations that are within 75nm
 * from the last-known user position or current route.  The reports can then be
 * accessed via the property "weatherStations" and the method
 * findWeatherStation.  The DownloadManager class honors
 * GlobalSettings::acceptedWeatherTerms() and will initiate a download only if
 * the user agreed to the privacy warning.
 *
 * Once constructed, the DownloadManager class will regularly perform background
 * updates to retrieve up-to-date information. It will update the list of known
 * weather stations and also the METAR/TAF reports for the weather stations.
 * The class checks regularly for outdated METAR and TAF reports and deletes
 * them automatically, along with those WeatherStations that no longer contain
 * any report.
 *
 * In order to avoid loss of data when the app is accidently closed in-flight,
 * the class stores all weather data at destruction and at regular intervals,
 * and reads the data back in on construction.
 *
 * This class also contains a number or convenience methods and properties
 * pertaining to sunrise/sunset
 */
class DownloadManager : public QObject {
    Q_OBJECT

public:
    class WeatherStation;

    /*! \brief Standard constructor
     *
     * @param route A pointer to a FlightRoute object. This is used to find
     * WeatherStations near the planned route.
     *
     * @param geoMapProvider Pointer to GeoMapProvider
     * 
     * @param networkAccessManager The manager for network requests
     * 
     * @param parent The standard QObject parent pointer
     */
    explicit DownloadManager(FlightRoute *route,
                           GeoMapProvider *geoMapProvider,
                           QNetworkAccessManager *networkAccessManager,
                           QObject *parent = nullptr);

    /*! \brief Destructor */
    ~DownloadManager() override;


    /*! \brief Background update flag
     *
     * Indicates if the last download process was started as a background update.
     */
    Q_PROPERTY(bool backgroundUpdate READ backgroundUpdate NOTIFY backgroundUpdateChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property backgroundUpdate
     */
    bool backgroundUpdate() const { return _backgroundUpdate; };

    /*! \brief Downloading flag
     *
     * Indicates that the DownloadManager is currently downloading METAR/TAF
     * information from the internet.
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property downloading
     */
    bool downloading() const;

    /*! \brief Find WeatherStation by ICAO code
     *
     * This method returns a pointer to the WeatherStation with the given ICAO
     * code, or a nullptr if no WeatherStation with the given code is known to
     * the DownloadManager.
     *
     * @warning The WeatherStation objects are owned by the DownloadManager and
     * can be deleted anytime.  Store it in a QPointer to avoid dangling
     * pointers.
     *
     * @param ICAOCode ICAO code name of the WeatherStation, such as "EDDF"
     *
     * @returns Pointer to WeatherStation
     */
    Weather::Station *findWeatherStation(const QString &ICAOCode) const
    {
        return _weatherStationsByICAOCode.value(ICAOCode, nullptr);
    }

    /*! \brief QNHInfo
     *
     * This property holds a human-readable, translated, rich-text string with
     * information about the QNH of the nearest weather station.  This could
     * typically read like "QNH: 1019 hPa in LFGA, 4min ago".  If no information
     * is available, the property holds an empty string.
     */
    Q_PROPERTY(QString QNHInfo READ QNHInfo NOTIFY QNHInfoChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property QNHInfo
     */
    QString QNHInfo() const;

    /*! \brief sunInfo
     *
     * This property holds a human-readable, translated, rich-text string with 
     * information about the next sunset or sunrise at the current position. This
     * could typically read like "SS 17:01, in 3h and 5min" or "Waiting for exact
     * position …"
     */
    Q_PROPERTY(QString sunInfo READ sunInfo NOTIFY sunInfoChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property infoString
     */
    static QString sunInfo() ;

    /*! \brief Update method
     *
     * If the global settings indicate that connections to aviationweather.com
     * are not allowed, this method does nothing and returns immediately.
     * Otherwise, this method initiates the asynchronous download of weather
     * information from the internet. It generates the necessary network queries
     * and sends them to aviationweather.com.
     *
     * - If an error occurred while downloading, the signal "error" will be emitted.
     *
     * - If the download completes successfully, the notifier signal for the
     *   property weatherStations will be emitted.
     *
     * @param isBackgroundUpdate This is a simple flag that can be set and later
     * retrieved in the "backgroundUpdate" property. This is a little helper for
     * the GUI that might want to wish to make a distinction between
     * automatically triggered background updates (which should not be shown to
     * the user) and those that are explicitly started by the user.
     */
    Q_INVOKABLE void update(bool isBackgroundUpdate=true);

    /*! \brief List of weather stations
     *
     * This property holds a list of all weather stations that are currently
     * known to this instance of the DownloadManager class, sorted according to
     * the distance to the last known position.  The list can change at any
     * time.
     *
     * @warning The WeatherStation objects are owned by the DownloadManager and
     * can be deleted anytime. Store it in a QPointer to avoid dangling
     * pointers.
     */
    Q_PROPERTY(QList<Weather::Station *> weatherStations READ weatherStations NOTIFY weatherStationsChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property weatherStations
     */
    QList<Weather::Station *> weatherStations() const;

signals:
    /*! \brief Notifier signal */
    void backgroundUpdateChanged();

    /*! \brief Notifier signal */
    void downloadingChanged();

    /*! \brief Signal emitted when a network error occurs
     *
     * This signal is emitted to indicate that the DownloadManager failed to
     * download weather data.
     *
     * @param message A human-readable, translated error message
     */
    void error(QString message);

    /*! \brief Notifier signal */
    void QNHInfoChanged();

    /*! \brief Notifier signal */
    void sunInfoChanged();

    /*! \brief Signal emitted when the list of weather reports changes */
    void weatherStationsChanged();

private slots:
    // Called when a download is finished
    void downloadFinished();

    // Check for expired METARs and TAFs and delete them.
    // This also deletes weather stations if they are no longer in use.
    void deleteExpiredMesages();

    // Name says it all. This method is called from the constructor,
    // but with a little lag to avoid conflicts in the initialisation of
    // static objects.
    void setupConnections() const;

private:
    Q_DISABLE_COPY_MOVE(DownloadManager)

    // Update interval is 30 mins, or 5 mins if update failed
    static const int updateIntervalNormal_ms  = 30*60*1000;
    static const int updateIntervalOnError_ms =  5*60*1000;

    // Similar to findWeatherStation, but will create a weather station if no
    // station with the given code is known
    Weather::Station *findOrConstructWeatherStation(const QString &ICAOCode);

    // This method loads METAR/TAFs from a file "weather.dat" in
    // QStandardPaths::AppDataLocation.  There is locking to ensure that no two
    // processes access the file. The method will fail silently on error.
    // Returns true on success and false on failure.
    bool load();

    // This method saves all METAR/TAFs that are valid and not yet expired to a
    // file "weather.dat" in QStandardPaths::AppDataLocation.  There is locking
    // to ensure that no two processes access the file. The method will fail
    // silently on error.
    void save();

    // Pointer to the flight route
    QPointer<FlightRoute> _flightRoute;

    // Pointer to global settings
    QPointer<GeoMapProvider> _geoMapProvider;

    // Pointer to the network access manager
    QPointer<QNetworkAccessManager> _networkAccessManager;

    // List of replies from aviationweather.com
    QList<QPointer<QNetworkReply>> _networkReplies;

    // A timer used for auto-updating the weather reports every 30 minutes
    QTimer _updateTimer;

    // A timer used for deleting expired weather reports ever 11 minutes
    QTimer _deleteExiredMessagesTimer;

    // Flag, as set by the update() method
    bool _backgroundUpdate {true};

    // List of weather stations, accessible by ICAO code
    QMap<QString, QPointer<Weather::Station>> _weatherStationsByICAOCode;

    // Date and Time of last update
    QDateTime _lastUpdate;
};


}
