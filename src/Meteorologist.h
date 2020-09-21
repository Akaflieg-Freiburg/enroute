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

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QNetworkReply>
#include <QXmlStreamReader>

#include "Clock.h"
#include "FlightRoute.h"
#include "GlobalSettings.h"
#include "SatNav.h"
#include "WeatherReport.h"


/*! \brief Meteorologist, weather service manager
 *
 * This class retrieves METAR/TAF weather reports from the "Aviation Weather Center" for all stations that
 * are within 75nm from the last-known user position or current route.  The reports are fetched from aviationweather.com in XML
 * format, and subsequently decoded.
 */
class Meteorologist : public QObject {
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param clock A clock object
     *
     * @param sat The satellite navigation system
     *
     * @param route The flight route
     * 
     * @param networkAccessManager The manager for network requests
     * 
     * @param parent The standard QObject parent pointer
     */
    explicit Meteorologist(Clock *clock,
                           SatNav *sat,
                           FlightRoute *route,
                           GlobalSettings *globalSettings,
                           QNetworkAccessManager *networkAccessManager,
                           QObject *parent = nullptr);

    /*! \brief Destructor */
    ~Meteorologist() override;

    /*! \brief QNHInfo
     *
     * This property holds a richt-text string with global information. A typical result could be "Sunset at 17:01, in 32 minutes.<br>Last METAR/TAF update 21 minutes ago."
     * Depending on availability of information, the string can also be shorter, or altogether empty.
     */
    Q_PROPERTY(QString QNHinfo READ QNHInfo NOTIFY QNHInfoChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property infoString
     */
    QString QNHInfo() const;

    /*! \brief SunInfo
     *
     * This property holds a richt-text string with global information. A typical result could be "Sunset at 17:01, in 32 minutes.<br>Last METAR/TAF update 21 minutes ago."
     * Depending on availability of information, the string can also be shorter, or altogether empty.
     */
    Q_PROPERTY(QString SunInfo READ SunInfo NOTIFY SunInfoChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property infoString
     */
    QString SunInfo() const;

    /*! \brief The list of weather reports
     *
     * Returns the weather reports as a list of QObject for better interraction
     * with QML.
     */
    Q_PROPERTY(QList<QObject*> reports READ reports NOTIFY reportsChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property reports
     */
    QList<QObject*> reports() const;

    /*! \brief Downloading flag
     *
     * Indicates if the Meteorologist is currently downloading METAR/TAF information from the internet
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property downaloading
     */
    bool downloading() const;

    /*! \brief Background update flag
     *
     * Indicates if the last download process was started as a background update, or if it was explicitly started by the user.
     */
    Q_PROPERTY(bool backgroundUpdate READ backgroundUpdate NOTIFY backgroundUpdateChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property processing
     */
    bool backgroundUpdate() const { return _backgroundUpdate; };

    /*! \brief Update method
     *
     * Gets the last-known user location and the current route, generates the
     * network queries and send them to aviationweather.com.  If the global settings indicate
     * that connection to aviationweather.com is not allowed, this method does nothing and returns immediately.
     *
     * @param isBackgroundUpdate This is a simple flag that can be set and later retrieved in the "backgroundUpdate" property. This is a little helper for the GUI that might want to wish
     * to make a distinction between automatically triggered background updates (which should not be shown to the user) and those that are explicitly started by the user.
     */
    Q_INVOKABLE void update(bool isBackgroundUpdate=true);

    Q_INVOKABLE QString briefDescription(QString code) const
    {
        auto rep = report(code);
        if (rep)
            return rep->oneLineDescription();
        return QString();
    }

    WeatherReport *report(QString code) const
    {
        foreach(auto report, _reports) {
            if (report.isNull())
                continue;
            if (report->id() == code)
                return report;
        }
        return nullptr;
    }

    Q_INVOKABLE QString cat(QString code) const
    {
        auto rep = report(code);
        if (rep == nullptr)
            return QString();
        return rep->cat();
    }

signals:
    /*! \brief Notifier signal */
    void backgroundUpdateChanged();

    /*! \brief Signal emitted when the processing flag changes */
    void downloadingChanged();

    /*! \brief Signal emitted when a network error occurs */
    void error(QString message);

    /*! \brief Notifier signal */
    void QNHInfoChanged();

    /*! \brief Notifier signal */
    void SunInfoChanged();

    /*! \brief Signal emitted when the list of weather reports changes */
    void reportsChanged();

private:
    Q_DISABLE_COPY_MOVE(Meteorologist)

    /*! \brief Pointer to satellite navigation system */
    QPointer<SatNav> _sat;

    /*! \brief Pointer to route */
    QPointer<FlightRoute> _route;

    /*! \brief Pointer to global settings */
    QPointer<GlobalSettings> _globalSettings;

    /*! \brief Pointer to network manager */
    QPointer<QNetworkAccessManager> _networkAccessManager;

    /*! \brief List of replies that will be fetched from aviationweather.com */
    QList<QPointer<QNetworkReply>> _replies;

    /*! \brief A timer used for auto-updating the weather reports every 30 minutes */
    QTimer _updateTimer;

    /*! \brief Time of last update */
    QDateTime _lastUpdate {};

    /*! \brief flag, as set by the update() method */
    bool _backgroundUpdate {true};

    /*! \brief List of weather reports */
    QList<QPointer<WeatherReport>> _reports;

    /*! \brief Slot activated when a download is finished */
    void downloadFinished();

    /*! \brief Process the replies from aviationweather.com and generates the reports */
    void process();
};
