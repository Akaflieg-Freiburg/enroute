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

#include <QGeoRectangle>
#include <QProperty>
#include <QTimer>

#include "GlobalObject.h"
#include "geomaps/Waypoint.h"
#include "navigation/Atmosphere.h"
#include "weather/METAR.h"
#include "weather/TAF.h"

class QNetworkReply;


namespace Weather {

/*! \brief WeatherDataProvider, weather service manager
 *
 * This class retrieves METAR/TAF weather reports from the server enroute-data.
 *
 * Once constructed, the WeatherDataProvider class will regularly perform
 * background updates to retrieve up-to-date information for a region around the
 * current position and around the intended flight route. The class checks
 * regularly for outdated METAR and TAF and deletes them automatically.
 *
 * In order to avoid loss of data when the app is accidently closed in-flight,
 * the class stores all weather data at destruction and at regular intervals,
 * and reads the data back in on construction.
 *
 * This class also contains a number or convenience methods and properties
 * pertaining to sunrise and sunset.
 */
class WeatherDataProvider : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    class updateLogEntry;
    friend QDataStream& operator<<(QDataStream& stream, const updateLogEntry& ule);
    friend QDataStream& operator>>(QDataStream& stream, updateLogEntry& ule);

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit WeatherDataProvider(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit WeatherDataProvider() = delete;

    // factory function for QML singleton
    static Weather::WeatherDataProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::weatherDataProvider();
    }


    //
    // Properties
    //

    /*! \brief Downloading flag
     *
     * Indicates that the WeatherDataProvider is currently downloading METAR/TAF
     * information from the internet.
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief QNH
     *
     * This property holds the QNH of the next airfield, if known. If no QNH is
     * known, this property holds QNaN.
     */
    Q_PROPERTY(Units::Pressure QNH READ QNH NOTIFY QNHInfoChanged)

    /*! \brief QNHPressureAltitude
     *
     *  This property holds the altitude in the standard atmosphere which
     *  corresponds to the current QNH value
     */
    Q_PROPERTY(Units::Distance QNHPressureAltitude READ QNHPressureAltitude NOTIFY QNHInfoChanged)

    /*! \brief QNHInfo
     *
     * This property holds a human-readable, translated, rich-text string with
     * information about the QNH of the nearest weather station.  This could
     * typically read like "QNH: 1019 hPa in LFGA, 4min ago".  If no information
     * is available, the property holds an empty string.
     */
    Q_PROPERTY(QString QNHInfo READ QNHInfo NOTIFY QNHInfoChanged)

    /*! \brief sunInfo
     *
     * This property holds a human-readable, translated, rich-text string with
     * information about the next sunset or sunrise at the current position.
     * This could typically read like "SS 17:01, in 3h and 5min" or "Waiting for
     * exact position …"
     */
    Q_PROPERTY(QString sunInfo READ sunInfo NOTIFY sunInfoChanged)

    /*! \brief List of METARs */
    Q_PROPERTY(QMap<QString,Weather::METAR> METARs READ METARs BINDABLE bindableMETARs)

    /*! \brief List of TAFs */
    Q_PROPERTY(QMap<QString,Weather::TAF> TAFs READ TAFs BINDABLE bindableTAFs)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property of the same name
     *
     * @returns Property downloading
     */
    [[nodiscard]] bool downloading() const;

    /*! \brief Getter method for property of the same name
     *
     * @returns Property METARs
     */
    QMap<QString, Weather::METAR> METARs() {return m_METARs.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property METARs
     */
    QBindable<QMap<QString, Weather::METAR>> bindableMETARs() {return &m_METARs;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property QNH
     */
    [[nodiscard]] Units::Pressure QNH() const;

    /*! \brief Getter method for property of the same name
     *
     * @returns Property QNHPressureAltitude
     */
    [[nodiscard]] Units::Distance QNHPressureAltitude() const { return Navigation::Atmosphere::height(QNH()); }

    /*! \brief Getter method for property of the same name
     *
     * @returns Property QNHInfo
     */
    [[nodiscard]] QString QNHInfo() const;

    /*! \brief Getter method for property of the same name
     *
     * @returns Property sunInfo
     */
    static QString sunInfo();

    /*! \brief Getter method for property of the same name
     *
     * @returns Property TAFs
     */
    QMap<QString, Weather::TAF> TAFs() {return m_TAFs.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property TAFs
     */
    QBindable<QMap<QString, Weather::TAF>> bindableTAFs() {return &m_TAFs;}


    //
    // Methods
    //

    /*! \brief Request update
     *
     * This method initiates the asynchronous download of weather information
     * from the internet, for a region around the current position and around
     * the current flight route.  This method quits immediately if data for that
     * region has been downloaded successfully less than five minutes ago.
     *
     * If an error occurred while downloading, the signal "error" will be
     * emitted.
     */
    Q_INVOKABLE void requestUpdate();

    /*! \brief Request update
     *
     * This method initiates the asynchronous download of weather information
     * from the internet, for a region around the waypoint.  This method quits
     * immediately if data for that region is available or if the waypoint does
     * not describe an airfield with METAR/TAF station.
     *
     * If an error occurred while downloading, the signal "error" will be
     * emitted.
     *
     * @param wp Waypoint
     */
    Q_INVOKABLE void requestUpdate4Waypoint(const GeoMaps::Waypoint& wp);

signals:
    /*! \brief Notifier signal */
    void downloadingChanged();

    /*! \brief Signal emitted when a network error occurs
     *
     * This signal is emitted to indicate that the WeatherDataProvider failed to
     * download weather data.
     *
     * @param message A human-readable, translated error message
     */
    void error(QString message);

    /*! \brief Notifier signal */
    void QNHInfoChanged();

    /*! \brief Notifier signal */
    void sunInfoChanged();

private slots:
    // Called when a download is finished
    void downloadFinished();

    // Check for expired METARs and TAFs and delete them. This also deletes
    // weather stations if they are no longer in use.
    void deleteExpiredMesages();

    // Name says it all. This method is called from the constructor, but with a
    // little lag to avoid conflicts in the initialisation of static objects.
    void deferredInitialization();

    // This method is called by requestUpdate and requestUpdate4Waypoint. It
    // does the actual network request.
    void startDownload(const QGeoRectangle& bBox);

private:
    Q_DISABLE_COPY_MOVE(WeatherDataProvider)

    // Update interval is 30 mins, or 5 mins if update failed
    static const int updateIntervalNormal_ms  = 30*60*1000;
    static const int updateIntervalOnError_ms =  5*60*1000;

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

    // List of replies from aviationweather.com
    QList<QPointer<QNetworkReply>> m_networkReplies;

    // A timer used for auto-updating the weather reports every 30 minutes
    QTimer m_updateTimer;

    // A timer used for deleting expired weather reports ever 11 minutes
    QTimer m_deleteExiredMessagesTimer;

    // METARs and TAFs by ICAO Code
    QProperty<QMap<QString, Weather::METAR>> m_METARs;
    QProperty<QMap<QString, Weather::TAF>> m_TAFs;

    // Time and BBox of the last succesful METAR update for the current region and flight route
    struct updateLogEntry
    {
        QDateTime m_time;
        QGeoRectangle m_bBox;
    };
    QList<updateLogEntry> updateLog;
};

QDataStream& operator<<(QDataStream& stream, const WeatherDataProvider::updateLogEntry& ule);

QDataStream& operator>>(QDataStream& stream, WeatherDataProvider::updateLogEntry& ule);

} // namespace Weather
