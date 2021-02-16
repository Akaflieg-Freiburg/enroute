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

    /*! \brief Status codes */
    enum Status
    {
        /*! \brief Not connected to any device */
        Disconnected,

        /*! \brief Connecting to a device, but not connected yet */
        Connecting,

        /*! \brief Connected to a device, but no data flowing */
        Connected,

        /*! \brief Connected to a device and receiving data */
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


    // =================================================


    //
    // Methods
    //



    //
    // Properties
    //

    /*! \brief lastError
     *
     * This property holds a translated, human-readable string that describes the last error.
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property lastError
     */
    QString errorString() const
    {
        return _lastError;
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


    void setSimulatorFile(const QString& fileName = QString() );

signals:
    /*! \brief Emitted whenever a connection to a device is established */
    void connected();

    /*! \brief Emitted whenever a connection to a device is lost */
    void disconnected();

    /*! \brief Notifier signal */
    void statusChanged(Status);

    // =========================

    /*! \brief Barometric altitude
     *
     * If this class received barometric altitude information from a connected
     * traffic receiver, this information is emitted here.
     */
    void barometricAltitude(AviationUnits::Distance);

    void errorStringChanged();

    /*! \brief Position info
     *
     * If this class received position information from a connected
     * traffic receiver, this information is emitted here.
     */
    void positionInfo(QGeoPositionInfo);

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
    void connectToDevice();

    /*! \brief Disconnect from traffic receiver
     *
     * If this class is connected to a traffic receiver, this method does nothing.
     * Otherwise, it stops any ongoing connection attempt and starts a new attempt
     * to connect to a potential receiver.
     */
    void disconnectFromDevice();

private slots:

    void receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError);

    void readFromStream();

    void readFromSimulatorStream();

    void processFLARMMessage(QString msg);

    void setError(const QString &newError);

    void updateStatus();

private:
    // Property connected. Should only be written to by setConnected()
    Status _status {Disconnected};

    // Try to connect every five minutes
    QTimer connectTimer;

    // Timer: the property 'receiving' is updated when no new data is coming is for five seconds
    QTimer receivingTimer;

    QPointer<QTcpSocket> socket;
    QTextStream stream;

    // Property lastError.
    QString _lastError;

    // GPS altitude information
    AviationUnits::Distance _altitude;
    QDateTime _altitudeTimeStamp;

    // Simulator related members
    QFile simulatorFile;
    QTextStream simulatorStream;
    QTimer simulatorTimer;
    int lastTime {0};
    QString lastPayload;

    // Targets
    QList<Navigation::Traffic *> _trafficObjects;
    QPointer<Navigation::Traffic> _trafficObjectWithoutPosition;
};

}
