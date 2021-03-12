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
#include <QTcpSocket>
#include <QTimer>
#include <QObject>
#include <QQmlListProperty>

#include "AviationUnits.h"
#include "GlobalSettings.h"
#include "traffic/Factor.h"

namespace Navigation {

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
class FLARMAdaptor : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit FLARMAdaptor(QObject *parent = nullptr);

    // Standard destructor
    ~FLARMAdaptor() override = default;

    //
    // Methods
    //

    /*! \brief Pointer to static instance of this class
     *
     *  @returns Pointer to global instance
     */
    static FLARMAdaptor *globalInstance();


    //
    // Properties
    //

    /*! \brief String describin the last socket error
     *
     * This property holds a translated, human-readable string that describes
     * the last error, or an empty string when there is not error.  The string
     * is cleared when a new connection attempt is started.
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property errorString
     */
    QString errorString() const
    {
        return _errorString;
    }

    /*! \brief Indicates that traffic receiver sends position data.
     *
     *  This property is set to true if position data has been received from the traffic receiver in the last five seconds.
     */
    Q_PROPERTY(bool receivingBarometricAltData READ receivingBarometricAltData NOTIFY receivingBarometricAltDataChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property receivingPositionData
     */
    bool receivingBarometricAltData() const
    {
        return _receivingBarometricAltData;
    }

    /*! \brief Indicates that traffic receiver sends position data.
     *
     *  This property is set to true if position data has been received from the traffic receiver in the last five seconds.
     */
    Q_PROPERTY(bool receivingPositionData READ receivingPositionData NOTIFY receivingPositionDataChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property receivingPositionData
     */
    bool receivingPositionData() const
    {
        return _receivingPositionData;
    }

    /*! \brief Status codes */
    enum Status
    {
        /*! \brief Not connected to any device */
        Disconnected,

        /*! \brief Connecting to a device, but not connected yet */
        Connecting,

        /*! \brief Connected to a device, but no data flowing */
        Connected,

        /*! \brief Connected to a device and receiving FLARM heartbeat */
        Receiving
    };
    Q_ENUM(Status)

    /*! \brief Connectivity status */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connected
     */
    Status status() const
    {
        return _status;
    }

    /*! \brief Traffic objects whose position is known
     *
     *  This property holds a list of the most relevant traffic objects, as a
     *  QQmlListProperty for better cooperation with QML. Note that only the
     *  valid items in this list pertain to actual traffic. Invalid items should
     *  be ignored. The list is not sorted in any way. The items themselves are
     *  owned by this class.
     */
    Q_PROPERTY(QQmlListProperty<Traffic::Factor> trafficObjects4QML READ trafficObjects4QML CONSTANT)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjects4QML
     */
    QQmlListProperty<Traffic::Factor> trafficObjects4QML()
    {
        return QQmlListProperty(this, &_trafficObjects);
    }

    /*! \brief Most relevant traffic object whose position is not known
     *
     *  This property holds a pointer to the most relevant traffic object whose
     *  position is not known.  This item should be ignored if invalid. The item
     *  is owned by this class.
     */
    Q_PROPERTY(Traffic::Factor *trafficObjectWithoutPosition READ trafficObjectWithoutPosition CONSTANT)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjectWithoutPosition
     */
    Traffic::Factor *trafficObjectWithoutPosition()
    {
        return _trafficObjectWithoutPosition;
    }


signals:
    /*! \brief Barometric altitude
     *
     * If this class received barometric altitude information from a connected
     * traffic receiver, this information is emitted here.
     */
    void barometricAltitude(AviationUnits::Distance);

    /*! \brief Emitted whenever a connection to a device is established */
    void connected();

    /*! \brief Emitted whenever a connection to a device is lost */
    void disconnected();

    /*! \brief Notifier signal */
    void errorStringChanged();

    /*! \brief Position info
     *
     * If this class received position information from a connected traffic
     * receiver, this information is emitted here.
     */
    void positionInfo(QGeoPositionInfo);

    /*! \brief Notifier signal */
    void receivingPositionDataChanged(bool);

    /*! \brief Notifier signal */
    void receivingBarometricAltDataChanged(bool);

    /*! \brief Notifier signal */
    void statusChanged(Navigation::FLARMAdaptor::Status);

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
    void connectToTrafficReceiver();

    /*! \brief Disconnect from traffic receiver
     *
     * This method stops any ongoing connection or connection attempt.
     */
    void disconnectFromTrafficReceiver();

private slots:
    // Handle socket errors.
    void receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError);

    // Read one line from the socket's text stream and passes the string on to
    // processFLARMMessage.
    void readFromStream();

    // Read one line from the simulator file's text stream and passes the string
    // on to processFLARMMessage.  Sets up times to read the next line in due
    // time.
    void readFromSimulatorStream();

    // Read, understand and process one NMEA sentence
    void processFLARMMessage(QString msg);

    // Update the property "error" and emit notification signals
    void setErrorString(const QString &newErrorString);

    // Update the property "receivingPositionData" and emit notification signals
    void updateReceivingBarometricAltData();

    // Update the property "receivingPositionData" and emit notification signals
    void updateReceivingPositionData();

    // Update the property "status" and emit notification signals
    void updateStatus();

private:
    // Property connected. Should only be written to by setConnected()
    Status _status {Disconnected};

    // Try to connect every five minutes
    QTimer connectTimer;

    // Timers. Properties are reset when no new data comes in for five seconds.
    QTimer receivingBarometricAltDataTimer;
    QTimer receivingHeartbeatTimer;
    QTimer receivingPositionDataTimer;

    QPointer<QTcpSocket> socket;
    QTextStream textStream;

    // Property caches
    QString _errorString;
    bool _receivingBarometricAltData {false};
    bool _receivingPositionData {false};

    // GPS altitude information
    AviationUnits::Distance _altitude;
    QDateTime _altitudeTimeStamp;

    // Simulator related members
    QFile simulatorFile;
    QTextStream simulatorTextStream;
    QTimer simulatorTimer;
    int lastTime {0};
    QString lastPayload;

    // Targets
    QList<Traffic::Factor *> _trafficObjects;
    QPointer<Traffic::Factor> _trafficObjectWithoutPosition;
};

}
