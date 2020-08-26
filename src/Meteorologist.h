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

#include "SatNav.h"
#include "FlightRoute.h"
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
     * @param sat The satellite navigation system
     * 
     * @param route The flight route
     * 
     * @param networkAccessManager The manager for network requests
     * 
     * @param parent The standard QObject parent pointer
     */
    explicit Meteorologist(SatNav *sat, FlightRoute *route,
                           QNetworkAccessManager *networkAccessManager,
                           QObject *parent = nullptr);

    /*! \brief Destructor */
    ~Meteorologist() override;

    /*! \brief Time of last succesful METAR/TAF update
     *
     * This property holds an empty string if there has not been any update yet
     */
    Q_PROPERTY(QString timeOfLastUpdateAsString READ timeOfLastUpdateAsString NOTIFY timeOfLastUpdateAsStringChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property reports
     */
    QString timeOfLastUpdateAsString() const {
        if (!_lastUpdate.isValid())
            return QString();
        return _lastUpdate.toString("HH:mm");
    };

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
     * Indicates if the Meteorologist is currently downloading (return
     * true), or not (return false)
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property processing
     */
    bool downloading() const;

    /*! \brief Update method
     *
     * Gets the last-known user location and the current route, generates the
     * network queries and send them to aviationweather.com
     */
    Q_INVOKABLE void update();

signals:
    /*! \brief Notifyer signal */
    void timeOfLastUpdateAsStringChanged();

    /*! \brief Signal emitted when the list of weather reports changes */
    void reportsChanged();

    /*! \brief Signal emitted when the processing flag changes */
    void downloadingChanged();

    /*! \brief Signal emitted when a network error occurs */
    void error(QString message);

private:
    Q_DISABLE_COPY_MOVE(Meteorologist)

    /*! \brief Pointer to satellite navigation system */
    QPointer<SatNav> _sat;

    /*! \brief Pointer to route */
    QPointer<FlightRoute> _route;

    /*! \brief Pointer to network manager */
    QPointer<QNetworkAccessManager> _networkAccessManager;

    /*! \brief List of replies that will be fetched from aviationweather.com */
    QList<QPointer<QNetworkReply>> _replies;

    /*! \brief A timer used for auto-updating the weather reports every 30 minutes */
    QTimer _timer;

    QDateTime _lastUpdate {};

    /*! \brief List of weather reports */
    QList<QPointer<WeatherReport>> _reports;

    /*! \brief Slot activated when a download is finished */
    void downloadFinished();

    /*! \brief Process the replies from aviationweather.com and generates the reports */
    void process();

    /*! \brief Reads an XML reply from aviationweather.com and create a map subsequently used to generate a report */
    QMultiMap<QString, QVariant> readReport(QXmlStreamReader &xml, const QString &type);
};
