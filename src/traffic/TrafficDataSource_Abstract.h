/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QGeoPositionInfo>
#include <QTimer>
#include <QObject>
#include <QQmlListProperty>

#include "GlobalSettings.h"
#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor.h"
#include "traffic/FLARMWarning.h"
#include "units/Distance.h"

namespace Traffic {

/*! \brief Traffic receiver
 *
 *  This class connects to a traffic receiver via the network. It expects to
 *  find a receiver at the IP-Address 192.168.1.1, port 2000.  Once connected,
 *  it continuously reads data from the device, and exposes position and traffic
 *  information to the user, as well as barometric altitude.
 *
 *  By modifying the source code, developers can also start the class in a mode
 *  where it connects to a file with simulator data.
 */
class TrafficDataSource_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Abstract(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Abstract() override = default;

    //
    // Properties
    //

    /*! \brief String describing the last socket error
     *
     * This property holds a translated, human-readable string that describes
     * the last error, or an empty string when there is not error.  The string
     * is cleared when a new connection attempt is started.
     */
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property errorString
     */
    QString errorString()
    {
        return _errorString;
    }

    /*! \brief Connectivity status */
    Q_PROPERTY(QString connectivityStatus READ connectivityStatus WRITE setConnectivityStatus NOTIFY connectivityStatusChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connectivityStatus
     */
    QString connectivityStatus() const
    {
        return _connectivityStatus;
    }

    /*! \brief FLARM heartbeat
     *
     *  This properts is true if a FLARM heartbeat has been received within the last 5 seconds.
     */
    Q_PROPERTY(bool hasHeartbeat READ hasHeartbeat NOTIFY hasHeartbeatChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property hasHeartbeat
     */
    bool hasHeartbeat()
    {
        return heartbeatTimer.isActive();
    }

    /*! \brief Source name */
    Q_PROPERTY(QString sourceName READ sourceName CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property sourceName
     */
    virtual QString sourceName() const = 0;


signals:
    /*! \brief Barometric altitude
     *
     * If this class received barometric altitude information from a connected
     * traffic receiver, this information is emitted here.
     */
    void barometricAltitudeUpdated(AviationUnits::Distance);

    /*! \brief Notifier signal */
    void connectivityStatusChanged();

    /*! \brief Notifier signal */
    void errorStringChanged();

    /*! \brief Traffic factor without position
     *
     * \param factor Pointer to traffic factor. This element is owned by this class and might change without notice.
     */
    void factorWithoutPosition(const Traffic::TrafficFactor &factor);

    /*! \brief Traffic factor with position
     *
     * \param factor Pointer to traffic factor. This element is owned by this class and might change without notice.
     */
    void factorWithPosition(const Traffic::TrafficFactor &factor);

//#warning need to document
    void flarmWarning(const Traffic::FLARMWarning &warning);

    /*! \brief Notifier signal */
    void hasHeartbeatChanged();

    /*! \brief Position info
     *
     * If this class received position information from a connected traffic
     * receiver, this information is emitted here.
     */
    void positionUpdated(Positioning::PositionInfo);

    /*! \brief Traffic receiver hardware version
     *
     * If this class receives information about the hardware version of a
     * connected traffic receiver, this information is emitted here.
     *
     * @param result String that identifies the hardware version
     */
    void trafficReceiverHwVersion(QString result);

    /*! \brief Traffic receiver obstacle database version
     *
     * If this class receives information about the obstacle database version of a connected
     * traffic receiver, this information is emitted here.
     *
     * @param result String that identifies the obstacle database version
     */
    void trafficReceiverObVersion(QString result);

    /*! \brief Result of traffic receiver self test
     *
     * If this class receives self-test information from a connected
     * traffic receiver, this information is emitted here.
     *
     * @param result Result of self-test as a human-readable, translated error message
     */
    void trafficReceiverSelfTest(QString result);

    /*! \brief Traffic receiver software version
     *
     * If this class receives information about the software version of a connected
     * traffic receiver, this information is emitted here.
     *
     * @param result String that identifies the software version
     */
    void trafficReceiverSwVersion(QString result);


public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     * If this class is connected to a traffic receiver, this method does nothing.
     * Otherwise, it stops any ongoing connection attempt and starts a new attempt
     * to connect to a potential receiver.
     */
    virtual void connectToTrafficReceiver() = 0;

    /*! \brief Disconnect from traffic receiver
     *
     * This method stops any ongoing connection or connection attempt. This method will not reset the property errorString,
     * so that the error remains visible even after the class has been disconnected from the traffic receiver.
     */
    virtual void disconnectFromTrafficReceiver() = 0;


protected:
    /*! \brief Setter function for the property with the same name
     *
     * @param newConnectivityStatus Property connectivityStatus
     */
    void setConnectivityStatus(const QString& newConnectivityStatus);

    /*! \brief Setter function for the property with the same name
     *
     * @param newErrorString Property errorString
     */
    void setErrorString(const QString& newErrorString = QString());

    void stopHeartbeat();

    // Read, understand and process one NMEA sentence
    void processFLARMMessage(QString msg);

private:
    // This slot is called whenever a heartbeat is reveived. If updates the property
    // hasHeartbeat.
    void onHeartbeat();

    // Property caches
    QString _connectivityStatus {};
    QString _errorString {};

    // GPS altitude information
    AviationUnits::Distance _altitude;
    QDateTime _altitudeTimeStamp;

    // Heartbeat timer
    QTimer heartbeatTimer;

    // Targets
    Traffic::TrafficFactor factor;
};

}
