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

#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor_DistanceOnly.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "traffic/Warning.h"


namespace Traffic {

/*! \brief Base class for all traffic receiver data sources
 *
 *  This is an abstract base class for all classes that connect to a traffic
 *  receiver.  In addition to the properties listed below, the class also emits
 *  imporant data via the signals barometricAltitudeUpdated,
 *  factorWithoutPosition, factorWithPosition and warning. It contains methods
 *  to interpret FLARM and GDL90 data streams.
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
     *  This property holds a translated, human-readable string that describes
     *  the last error, or an empty string when there is not error.  The string
     *  is cleared when a new connection attempt is started.
     */
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property errorString
     */
    auto errorString() -> QString
    {
        return m_errorString;
    }

    /*! \brief Connectivity status
     *
     *  This property contains a human-readable, translated string that
     *  describes if the class has established a connection to a traffic
     *  receiver. A typical string could be "Bound to an address and port, but
     *  not connected yet.".  Subclasses shall use the setter function to set
     *  the property content.
     *
     *  The setter method is protected and can be used by subclasses to update
     *  the property content.
     */
    Q_PROPERTY(QString connectivityStatus READ connectivityStatus WRITE setConnectivityStatus NOTIFY connectivityStatusChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connectivityStatus
     */
    [[nodiscard]] auto connectivityStatus() const -> QString
    {
        return m_connectivityStatus;
    }

    /*! \brief Heartbeat indicator
     *
     *  When active, traffic receivers send regular heartbeat messages. These
     *  can be used to verify that the connection to the receiver works, even in
     *  times when no traffic is reported. This property indicates if the class
     *  receives heartbeat messages from at least one of the known receivers.
     *
     *  The setter and resetter methods are protected and can be used by
     *  subclasses to update the property content.
     */
    Q_PROPERTY(bool receivingHeartbeat READ receivingHeartbeat WRITE setReceivingHeartbeat RESET resetReceivingHeartbeat NOTIFY receivingHeartbeatChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property receivingHeartbeat
     */
    auto receivingHeartbeat() -> bool
    {
        return m_heartbeatTimer.isActive();
    }

    /*! \brief Source name
     *
     *  This property contains a short, human-readable and translated
     *  description of the source. A typical string is "TCP connection to
     *  132.168.1.1 port 2000".
     */
    Q_PROPERTY(QString sourceName READ sourceName CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property sourceName
     */
    [[nodiscard]] virtual auto sourceName() const -> QString = 0;

    /*! \brief String describing the last traffic data receiver runtime error
     *
     *  This property holds a translated, human-readable string that describes
     *  the last error reported by the traffic receiver, or an empty string when
     *  there is not error.  The string is cleared when a new connection attempt
     *  is started.
     */
    Q_PROPERTY(QString trafficReceiverRuntimeError READ trafficReceiverRuntimeError WRITE setTrafficReceiverRuntimeError NOTIFY trafficReceiverRuntimeErrorChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property errorString
     */
    auto trafficReceiverRuntimeError() -> QString
    {
        return m_trafficReceiverRuntimeError;
    }

    /*! \brief String describing the last traffic data receiver self-test error
     *
     *  This property holds a translated, human-readable string that describes
     *  the last error reported by the traffic receiver self-test, or an empty
     *  string when there is not error.  The string is cleared when a new
     *  connection attempt is started.
     */
    Q_PROPERTY(QString trafficReceiverSelfTestError READ trafficReceiverSelfTestError WRITE setTrafficReceiverSelfTestError NOTIFY trafficReceiverSelfTestErrorChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property errorString
     */
    auto trafficReceiverSelfTestError() -> QString
    {
        return m_trafficReceiverSelfTestError;
    }

signals:
    /*! \brief Notifier signal */
    void connectivityStatusChanged(QString newStatus);

    /*! \brief Notifier signal */
    void errorStringChanged(QString newError);

    /*! \brief Traffic factor without position
     *
     *  This signal is emitted when the traffic receiver informs this class
     *  about traffic whose position is not known.
     *
     *  \param factor Traffic factor.
     */
    void factorWithoutPosition(const Traffic::TrafficFactor_DistanceOnly &factor);

    /*! \brief Traffic factor with position
     *
     *  This signal is emitted when the traffic receiver informs this class
     *  about traffic whose position is known.
     *
     *  \param factor Traffic factor.
     */
    void factorWithPosition(const Traffic::TrafficFactor_WithPosition& factor);

    /* \brief Password request
     *
     *  This signal is emitted whenever the traffic receiver asks for a
     *  password. Note that this is not the WiFi-Password.
     *
     *  @param SSID Name of the WiFi network that is currently in use.
     */
    void passwordRequest(const QString& SSID);

    /* \brief Password storage request
     *
     *  This signal is emitted whenever the traffic receiver has successfully
     *  connected using a password that was not yet in the database.
     *
     *  @param SSID Name of the WiFi network that is was used in use.
     */
    void passwordStorageRequest(const QString& SSID, const QString& password);

    /*! \brief Pressure altitude
     *
     *  If this class received pressure altitude information from a connected
     *  traffic receiver, this information is emitted here. Pressure altitude is
     *  the altitude shown by your altimeter if the altimeter is set to 1013.2
     *  hPa.
     */
    void pressureAltitudeUpdated(Units::Distance);

    /*! \brief Position info
     *
     *  If this class received position information from a connected traffic
     *  receiver, this information is emitted here.
     */
    void positionUpdated(Positioning::PositionInfo pInfo);

    /*! \brief Notifier signal */
    void receivingHeartbeatChanged(bool);

    /*! \brief Notifier signal */
    void trafficReceiverRuntimeErrorChanged(const QString& message);

    /*! \brief Notifier signal */
    void trafficReceiverSelfTestErrorChanged(const QString& message);

    /*! \brief Traffic receiver hardware version
     *
     *  If this class receives information about the hardware version of a
     *  connected traffic receiver, this information is emitted here.
     *
     *  @param result String that identifies the hardware version
     */
    void trafficReceiverHwVersion(QString result);

    /*! \brief Traffic receiver obstacle database version
     *
     *  If this class receives information about the obstacle database version
     *  of a connected traffic receiver, this information is emitted here.
     *
     *  @param result String that identifies the obstacle database version
     */
    void trafficReceiverObVersion(QString result);

    /*! \brief Traffic receiver software version
     *
     *  If this class receives information about the software version of a
     *  connected traffic receiver, this information is emitted here.
     *
     *  @param result String that identifies the software version
     */
    void trafficReceiverSwVersion(QString result);

    /*! \brief Traffic warning
     *
     *  This signal is emitted when the traffic receiver issues a traffic
     *  warning. An invalid warning (i.e. a warning with alarm level = -1) is
     *  emitted to indicate that the last warning is no longer active and should
     *  be disregarded.
     *
     *  \param warning Traffic warning.
     */
    void warning(const Traffic::Warning& warning);

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  If this class is connected to a traffic receiver, this method does
     *  nothing.  Otherwise, it stops any ongoing connection attempt and starts
     *  a new attempt to connect to a potential receiver.
     */
    virtual void connectToTrafficReceiver() = 0;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method stops any ongoing connection or connection attempt. This
     *  method will not reset the property errorString, so that the error
     *  remains visible even after the class has been disconnected from the
     *  traffic receiver.
     */
    virtual void disconnectFromTrafficReceiver() = 0;

    /*! \brief Set password
     *
     *  If the implementation of the traffic data source supports passwords,
     *  this method checks if the traffic data source is waiting for a password
     *  with key SSID. If so, it will send the password to the traffic data
     *  receiver. If the implementation of the traffic data source does not
     *  support passwords, this method does nothing.
     */
    virtual void setPassword(const QString& SSID, const QString& password)
    {
        Q_UNUSED(SSID)
        Q_UNUSED(password)
    }

protected:
    /*! \brief Process one FLARM/NMEA sentence
     *
     *  This method expects exactly one line containing a valid FLARM/NMEA
     *  sentence. This is a string typically looks like
     *  "$PFLAA,0,1587,1588,40,1,AA1237,225,,37,-1.6,1*7F".  The method
     *  interprets the string and updates the properties and emits signals as
     *  appropriate. Invalid strings are silently ignored.
     *
     *  @param sentence A QString containing a FLARM/NMEA sentence.
     */
    void processFLARMSentence(QString sentence);

    /*! \brief Process one GDL90 message
     *
     *  This method expects exactly one GDL90 message, including starting and
     *  trailing 0x7e bytes.  The method interprets the string and updates the
     *  properties and emits signals as appropriate. Invalid messages are
     *  silently ignored.
     *
     *  @param message A QByteArray containing a GDL90 message.
     */
    void processGDLMessage(const QByteArray& message);

    /*! \brief Process one XGPS string
     *
     *  This method expects exactly XGPS/XTRAFFIC string, as specified in
     *
     *  https://www.foreflight.com/support/network-gps/
     *
     *  The method interprets the string and updates the properties and emits
     *  signals as appropriate. Invalid messages are silently ignored.
     *
     *  @param data A QByteArray containing an XGPS string.
     */
    void processXGPSString(const QByteArray& data);

    /*! \brief Resetter method for the property with the same name
     *
     *  This is equivalent to calling setReceivingHeartbeat(false)
     */
    void resetReceivingHeartbeat();

    /*! \brief Setter function for the property with the same name
     *
     *  @param newConnectivityStatus Property connectivityStatus
     */
    void setConnectivityStatus(const QString& newConnectivityStatus);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newErrorString Property errorString
     */
    void setErrorString(const QString& newErrorString = QString());

    /*! \brief Setter method for the property with the same name
     *
     *  When set to 'true' a timer is stated that will automatically reset the
     *  property to 'false' after 5 seconds of inactivity.
     *
     *  @param newReceivingHeartbeat Property receivingHeartbeat
     */
    void setReceivingHeartbeat(bool newReceivingHeartbeat);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newErrorString Property errorString
     */
    void setTrafficReceiverRuntimeError(const QString& newErrorString);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newErrorString Property errorString
     */
    void setTrafficReceiverSelfTestError(const QString& newErrorString);

private:
    // Property caches
    QString m_connectivityStatus {};
    QString m_errorString {};
    QString m_trafficReceiverRuntimeError {};
    QString m_trafficReceiverSelfTestError {};

    // True altitude of own aircraft. We store these values because the
    // necessary information to compile a PositionInfo class does not always
    // come in one piece.  Whenever a valid altitude is set, the timer should be
    // started. The timer can then be used to check if the altitude information
    // is recent enough to be used. Whenever an invalid altitude is set, the
    // timer should be stopped.
    Units::Distance m_trueAltitude;
    Units::Distance m_trueAltitudeFOM; // Fig. of Merit
    QTimer m_trueAltitudeTimer;

    // Pressure altitude of own aircraft. See the member m_trueAltitude for a
    // description how the timer should be used.
    Units::Distance m_pressureAltitude;
    QTimer m_pressureAltitudeTimer;

    // Heartbeat timer
    QTimer m_heartbeatTimer;
    bool m_hasHeartbeat {false};

    // Targets
    Traffic::TrafficFactor_WithPosition m_factor;
    Traffic::TrafficFactor_DistanceOnly m_factorDistanceOnly;
};

}
