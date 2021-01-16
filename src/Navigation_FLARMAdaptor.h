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
 *  This class connects via WIFI to a traffic receiver. Once connected, it exposes position and traffic information.
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

    /*! \brief Pointer to static instance of this class
     *
     *  @returns Pointer to global instance
     */
    static FLARMAdaptor *globalInstance();

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

    Q_PROPERTY(QString FLARMHwVersion READ FLARMHwVersion NOTIFY FLARMHwVersionChanged)

    QString FLARMHwVersion() const
    {
        return _FLARMHwVersion;
    }

    Q_PROPERTY(QString FLARMSwVersion READ FLARMSwVersion NOTIFY FLARMSwVersionChanged)

    QString FLARMSwVersion() const
    {
        return _FLARMSwVersion;
    }

    Q_PROPERTY(QString FLARMObstVersion READ FLARMObstVersion NOTIFY FLARMObstVersionChanged)

    QString FLARMObstVersion() const
    {
        return _FLARMObstVersion;
    }

    Q_PROPERTY(QString FLARMSelfTest READ FLARMSelfTest NOTIFY FLARMSelfTestChanged)

    QString FLARMSelfTest() const
    {
        return _FLARMSelfTest;
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
    void barometricAltitudeChanged();
    void flarmSelfTestFailed();
    void FLARMSelfTestChanged();
    void FLARMHwVersionChanged();
    void FLARMSwVersionChanged();
    void FLARMObstVersionChanged();
    void receivingChanged();
    void statusStringChanged();
    void statusChanged();
    void positionInfoChanged();

public slots:
    void connectToFLARM();

private slots:

    void receiveSocketConnected();
    void receiveSocketDisconnected();
    void receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError);

    void readFromStream();

    void readFromSimulatorStream();

    void processFLARMMessage(QString msg);

    void clearFlarmDeviceInfo();

    // Sets the barometric altitude to NaN and emits the notifier signal when appropriate
    void clearBarometricAltitude();

    // Stops the _positionInfoTimer and emits the notifier signal when appropriate
    void clearPositionInfo();

    // Sets the barometric altitude and emits the notifier signal when appropriate
    void setBarometricAltitude(AviationUnits::Distance alt);

    // Sets the position info and emits the notifier signal when appropriate
    void setPositionInfo(const QGeoPositionInfo& newPositionInfo);

    void setStatus();
    void setStatusString();

private:
    QTimer connectTimer;
    QTimer heartBeatTimer;

    QTcpSocket socket;
    QTextStream stream;

    // General status info. Should only been written to by setStatus()
    Status _status {InOp};

    // Detailed status info. Should only been written to by setStatusString()
    QString _statusString;

    // Barometric height information
    AviationUnits::Distance _barometricAltitude;
    QTimer _barometricAltitudeTimer;

    // PositionInfo
    QGeoPositionInfo _positionInfo;
    QTimer _positionInfoTimer;

    // FLARM information
    QString _FLARMHwVersion; // Hardware version
    QString _FLARMSwVersion; // Software version
    QString _FLARMObstVersion; // Name of obstacle database
    QString _FLARMSelfTest;

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
    QList<Navigation::Traffic *> _trafficObjects;
    Navigation::Traffic * _trafficObjectWithoutPosition;
};

}
