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
#include "Navigation_Traffic.h"

namespace Navigation {

/*! \brief Traffic receiver
 *
 *  This class connects to a traffic receiver via the network. It expects to find a receiver at the IP-Address
 *  192.168.1.1, port 2000.  Once connected, it continuously reads data from the device, and exposes
 *  position and traffic information to the user, as well as barometric altitude.
 *
 *  By modifying the source code, developers can also start the class in a mode where it connects
 *  to a file with simulator data.
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

    /*! \brief Connectivity status
     *
     * This property contains a bool that indicates if the class is currently connected
     * to a traffic receiver. A connection might either exists to a real traffic receiver
     * via the IP-Address 192.168.1.1, port 2000, or a connection could exist to a file
     * containing synthetic simulator data.
     *
     * Being connected to a device does not imply that data is actually flowing.
     * Use the propery receiving for this.
     */
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connected
     */
    bool connected() const
    {
        return _connected;
    }

    /*! \brief Traffic receiver heatbeat
     *
     * This property contains a bool that indicates if the class has received valid NMEA sentences in the last 5 seconds.
     */
    Q_PROPERTY(bool receiving READ receiving NOTIFY receivingChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property heartbeat
     */
    bool receiving() const
    {
        return connected() && receivingTimer.isActive();
    }


    // =================================================

    /*! \brief Status codes */
    enum Status
    {
        OK, /*!< A FLARM device is connected and receives data. */
        WaitingForData, /*!< A FLARM device is connected, but no data has been received for more than 5 seconds.  */
        InOp /*!< No FLARM device connected */
    };
    Q_ENUM(Status)


    //
    // Methods
    //

    /*! \brief Last position information
     *
     *  This method differs from positionInfo() in that it always returns the last report, even
     *  if the report is older than 3 seconds.
     *
     *  @returns Last position information
     */
    QGeoPositionInfo lastPositionInfo() const
    {
        return _positionInfo;
    }


    //
    // Properties
    //

    /*! \brief Activity
     *
     * This property contains a translated, human-readable string that explains what this class is currently
     * doing.
     */
    Q_PROPERTY(QString activity READ activity NOTIFY activityChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property activity
     */
    QString activity() const
    {
        return _activity;
    }

    /*! \brief Barometric altitude
     *
     * This property holds the barometric altitude, as reported by the NMEA source, or NaN if no barometric altitude is available.
     * An altitude report is considered valid for 3 seconds after it is reported. After that time, the content of this property
     * goes back to NaN.
     */
    Q_PROPERTY(AviationUnits::Distance barometricAltitude READ barometricAltitude NOTIFY barometricAltitudeChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property barometricAltitude
     */
    AviationUnits::Distance barometricAltitude() const;

    /*! \brief Can connect
     *
     * This property holds a bool. If true, then it makes senss to try and initiate new connection to the
     * traffic receiver. If false, then it makes sense to try and disconnect a traffic receiver.
     */
    Q_PROPERTY(bool canConnect READ canConnect NOTIFY canConnectChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property canConnect
     */
    bool canConnect() const;

    /*! \brief lastError
     *
     * This property holds a translated, human-readable string that describes the last error.
     */
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property lastError
     */
    QString lastError() const
    {
        return _lastError;
    }

    /*! \brief Position information for own aircraft
     *
     *  This property holds the own position, as reported by the NMEA source, or an invalid QGeoPositionInfo if no current data is available.
     *  A position report is considered valid for 3 seconds after it is reported. After that time, the content of this property
     *  goes back to an invalid QGeoPositionInfo.
     */
    Q_PROPERTY(QGeoPositionInfo positionInfo READ positionInfo NOTIFY positionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    QGeoPositionInfo positionInfo() const
    {
        if (_positionInfoTimer.isActive()) {
            return _positionInfo;
        }
        return QGeoPositionInfo();
    }

    /*! \brief Traffic objects whose position is known
     *
     *  This property holds a list of the most relevant traffic objects, as a QQmlListProperty for better cooperation with QML. Note that only the valid items in this list pertain to actual
     *  traffic. Invalid items should be ignored. The list is not sorted in any way. The items themselves are owned by this class.
     */
    Q_PROPERTY(QQmlListProperty<Navigation::Traffic> trafficObjects4QML READ trafficObjects4QML CONSTANT)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjects4QML
     */
    QQmlListProperty<Navigation::Traffic> trafficObjects4QML()
    {
        return QQmlListProperty(this, &_trafficObjects);
    }

    /*! \brief Most relevant traffic objects whose position is not known
     *
     *  This property holds a pointer to the most relevant traffic object whose position is not known.
     *  This item should be ignored if invalid. The item is owned by this class.
     */
    Q_PROPERTY(Navigation::Traffic *trafficObjectWithoutPosition READ trafficObjectWithoutPosition CONSTANT)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjectWithoutPosition
     */
    Navigation::Traffic *trafficObjectWithoutPosition()
    {
        return _trafficObjectWithoutPosition;
    }



// =================



    Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

    QString statusString() const
    {
        return _statusString;
    }

    /*! \brief Status of the FLARM adaptor

    This property holds the status of the FLARM adaptor class.
    */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property status
     */
    Status status() const
    {
        return _status;
    };

    void setSimulatorFile(const QString& fileName = QString() );

signals:
    /*! \brief Notifier signal */
    void connectedChanged(bool);

    /*! \brief Notifier signal */
    void receivingChanged();

    void activityChanged();
    void barometricAltitudeChanged();
    void canConnectChanged();
    void lastErrorChanged();
    void statusStringChanged();
    void statusChanged();
    void positionInfoChanged();

    /*! \brief Result of traffic receiver self test
     *
     * If this class receives self-test information from a connected
     * traffic receiver, this information is emitted here.
     *
     * @param message Result of self-test as a human-readable, translated error message
     */
    void trafficReceiverSelfTest(QString message);

    void trafficReceiverHwVersion(QString result);
    void trafficReceiverSwVersion(QString result);
    void trafficReceiverObVersion(QString result);


public slots:
    /*! \brief Start attempt to connect to device
     *
     * If this class is connected to a traffic receiver, this method does nothing.
     * Otherwise, it stops any ongoing connection attempt and starts a new attempt
     * to connect to a potential receiver.
     */
    void connectToTrafficReceiver();

private slots:

    void receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError);

    void readFromStream();

    void readFromSimulatorStream();

    void processFLARMMessage(QString msg);

    // Sets the barometric altitude to NaN and emits the notifier signal when appropriate
    void clearBarometricAltitude();

    // Stops the _positionInfoTimer and emits the notifier signal when appropriate
    void clearPositionInfo();

    // Sets the barometric altitude and emits the notifier signal when appropriate
    void setBarometricAltitude(AviationUnits::Distance newBarometricAltitude);

    // Sets the position info and emits the notifier signal when appropriate
    void setPositionInfo(const QGeoPositionInfo& newPositionInfo);

    void setActivity();
    void setError(const QString &newError);
    void setConnected();
    void setStatus();
    void setStatusString();

private:
    // Property connected. Should only be written to by setConnected()
    bool _connected {false};

    // ==========================

    QTimer connectTimer;
    QTimer receivingTimer;

    QPointer<QTcpSocket> socket;
    QTextStream stream;

    // Property activity. Should only be written to by setActivity()
    QString _activity;

    // Property lastError.
    QString _lastError;

    // General status info. Should only be written to by setStatus()
    Status _status {InOp};

    // Detailed status info. Should only be written to by setStatusString()
    QString _statusString;

    // Barometric height information
    AviationUnits::Distance _barometricAltitude;
    QTimer _barometricAltitudeTimer;

    // PositionInfo
    QGeoPositionInfo _positionInfo;
    QTimer _positionInfoTimer;

    // GPS altitude and position information
    AviationUnits::Distance _altitude;
    QDateTime _altitudeTimeStamp;

    // Simulator related members
    QFile simulatorFile;
    QTextStream simulatorStream;
    QTimer simulatorTimer;
    int lastTime {0};
    QString lastPayload;

    // Targets
#warning should use QPointer here
    QList<Navigation::Traffic *> _trafficObjects;
    Navigation::Traffic * _trafficObjectWithoutPosition;
};

}
