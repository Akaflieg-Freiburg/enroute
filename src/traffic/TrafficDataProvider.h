/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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

#include <QNetworkDatagram>
#include <QStandardPaths>
#include <QUdpSocket>

#include "GlobalObject.h"
#include "positioning/PositionInfoSource_Abstract.h"
#include "traffic/ConnectionInfo.h"
#include "traffic/TrafficDataSource_Abstract.h"

namespace Traffic {

/*! \brief Traffic receiver
 *
 *  This class manages multiple TrafficDataSources. It combines the data
 *  streams, and passes data from the most relevant (if any) traffic data source
 *  on to the consumers of this class.
 *
 *  By default, it watches the following data channels:
 *
 *  - TCP connection to 192.168.1.1, port 2000
 *  - TCP connection to 192.168.10.1, port 2000
 *
 *  This class also acts as a PositionInfoSource, and passes position data (that
 *  some traffic receivers provide) on to the the consumers of this class.
 *
 *  Following the standards established by the app ForeFlight, this classEnroute
 *  broadcasts a UDP message on port 63093 every 5 seconds while the app is
 *  running in the foreground. This message allows devices to discover Enroute’s
 *  IP address, which can be used as the target of UDP unicast messages.
 */
class TrafficDataProvider : public Positioning::PositionInfoSource_Abstract {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficDataProvider(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit TrafficDataProvider() = delete;

    // factory function for QML singleton
    static Traffic::TrafficDataProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::trafficDataProvider();
    }



    //
    // Properties
    //

    /*! \brief Traffic data sources
     *
     * This property holds a list of all traffic data sources currently in use.
     * The items are owned by this class.
     */
    Q_PROPERTY(QList<Traffic::TrafficDataSource_Abstract*> dataSources READ dataSources NOTIFY dataSourcesChanged)

    /*! \brief Heartbeat indicator
     *
     *  When active, traffic receivers send regular heartbeat messages. These
     *  can be used to verify that the connection to the receiver works, even in
     *  times when no traffic is reported. This property indicates if the class
     *  receives heartbeat messages from at least one of the known receivers.
     */
    Q_PROPERTY(bool receivingHeartbeat READ receivingHeartbeat WRITE setReceivingHeartbeat NOTIFY receivingHeartbeatChanged)

    /*! \brief Traffic objects whose position is known
     *
     *  This property holds a list of the most relevant traffic objects, as a
     *  QQmlListProperty for better cooperation with QML. Note that only the
     *  valid items in this list pertain to actual traffic. Invalid items should
     *  be ignored. The list is not sorted in any way. The items themselves are
     *  owned by this class.
     */
    Q_PROPERTY(QList<Traffic::TrafficFactor_WithPosition*> trafficObjects READ trafficObjects CONSTANT)

    /*! \brief Most relevant traffic object whose position is not known
     *
     *  This property holds a pointer to the most relevant traffic object whose
     *  position is not known.  This item should be ignored if invalid. The item
     *  is owned by this class.
     */
    Q_PROPERTY(Traffic::TrafficFactor_DistanceOnly* trafficObjectWithoutPosition READ trafficObjectWithoutPosition CONSTANT)

    /*! \brief String describing the current traffic data receiver errors
     *
     *  This property holds a translated, human-readable string that describes
     *  the current errors reported by the traffic receiver, or an empty string
     *  when there is no error.  The string is cleared when a new connection
     *  attempt is started.
     */
    Q_PROPERTY(QString trafficReceiverRuntimeError READ trafficReceiverRuntimeError NOTIFY trafficReceiverRuntimeErrorChanged)

    /*! \brief String describing the traffic data receiver errors found in self-test
     *
     *  This property holds a translated, human-readable string that describes
     *  the errors reported by the traffic receiver during self-test, or an
     *  empty string when there is no error.  The string is cleared when a new
     *  connection attempt is started.
     */
    Q_PROPERTY(QString trafficReceiverSelfTestError READ trafficReceiverSelfTestError NOTIFY trafficReceiverSelfTestErrorChanged)

    /*! \brief Current traffic warning
     *
     *  This property holds the current traffic warning.  The traffic warning is
     *  updated regularly and set to an invalid warning (i.e. one with
     *  alarmLevel == -1) after a certain period.
     */
    Q_PROPERTY(Traffic::Warning warning READ warning NOTIFY warningChanged)



    //
    // Getter Methods
    //

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property dataSources
     */
    [[nodiscard]] QList<Traffic::TrafficDataSource_Abstract*> dataSources() const;

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receiving
     */
    [[nodiscard]] bool receivingHeartbeat() const
    {
        return m_receivingHeartbeat;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjects
     */
    [[nodiscard]] QList<Traffic::TrafficFactor_WithPosition*> trafficObjects() const
    {
        return m_trafficObjects;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property trafficObjectWithoutPosition
     */
    [[nodiscard]] Traffic::TrafficFactor_DistanceOnly* trafficObjectWithoutPosition() const
    {
        return m_trafficObjectWithoutPosition;
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property trafficReceiverRuntimeError
     */
    [[nodiscard]] QString trafficReceiverRuntimeError() const
    {
        return m_trafficReceiverRuntimeError;
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property trafficReceiverSelfTestError
     */
    [[nodiscard]] QString trafficReceiverSelfTestError() const
    {
        return m_trafficReceiverSelfTestError;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property Warning
     */
    [[nodiscard]] Traffic::Warning warning() const
    {
        return m_Warning;
    }



    //
    // Methods
    //

    /*! \brief Add an additional data source
     *
     *  This method adds an additional data source to this TrafficDataProvider,
     *  typically a simulator source used for debugging purposes. The
     *  TrafficDataProvider takes ownership of the source.
     *
     *  @param source New TrafficDataSource that is to be added.
     */
    void addDataSource(Traffic::TrafficDataSource_Abstract* source);

    /*! \brief Add an additional data source
     *
     * This method adds an additional data source to this TrafficDataProvider,
     * typically a Bluetooth device.
     *
     * \param connectionInfo Description of the new connection.
     *
     * \returns An empty string on success, and a human-readable, translated error message on failure.
     */
    Q_INVOKABLE QString addDataSource(const Traffic::ConnectionInfo &connectionInfo);

    /*! \brief Add an additional data source
     *
     * This method adds an additional UDP data source to this TrafficDataProvider.
     *
     * \param port UDP Port.
     *
     * \returns An empty string on success, and a human-readable, translated error message on failure.
     */
    Q_INVOKABLE QString addDataSource_UDP(quint16 port);

    /*! \brief Add an additional data source
     *
     * This method adds an additional serial port data source to this TrafficDataProvider.
     *
     * \param portName Name of the serial port
     *
     * \returns An empty string on success, and a human-readable, translated error message on failure.
     */
    Q_INVOKABLE QString addDataSource_SerialPort(const QString& portName);

    /*! \brief Add an additional data source
     *
     * This method adds an additional TCP data source to this TrafficDataProvider.
     *
     * \param host Internet address
     *
     * \param port TCP Port.
     *
     * \returns An empty string on success, and a human-readable, translated error message on failure.
     */
    Q_INVOKABLE QString addDataSource_TCP(const QString& host, quint16 port);

    /*! \brief Remove data sources
     *
     * This method removes a data source from this TrafficDataProvider.
     * Canonical data sources cannot be removed
     *
     * \param source New TrafficDataSource that is to be removed.
     */
    Q_INVOKABLE void removeDataSource(Traffic::TrafficDataSource_Abstract* source);



    //
    // Constants
    //


    /*! \brief Maximal vertical distance for relevant traffic
     *
     *  Traffic whose vertical distance to the own aircraft is larger than this
     *  number will be ignored.
     */
    static constexpr Units::Distance maxVerticalDistance = Units::Distance::fromM(1500.0);

    /*! \brief Maximal horizontal distance for relevant traffic
     *
     *  Traffic whose horizontal distance to the own aircraft is larger than
     *  this number will be ignored.
     */
    static constexpr Units::Distance maxHorizontalDistance = Units::Distance::fromNM(20.0);

signals:
    /*! \brief Notifier signal */
    void dataSourcesChanged();

    /*! \brief Password request
     *
     *  This signal is emitted whenever one of the traffic data sources requires
     *  asks for a password. Note that this is not the WiFi-Password.
     *
     *  @param SSID Name of the WiFi network that is currently in use.
     */
    void passwordRequest(const QString& SSID);

    /*! \brief Password storage request
     *
     *  This signal is emitted whenever one of the traffic data sources has
     *  verified a password that was not yet in the database. The GUI should
     *  connect to this signal and open a "Store Password …?" dialogn.
     */
    void passwordStorageRequest(const QString& SSID, const QString& password);

    /*! \brief Notifier signal */
    void receivingHeartbeatChanged(bool);

    /*! \brief Notifier signal */
    void trafficReceiverRuntimeErrorChanged();

    /*! \brief Notifier signal */
    void trafficReceiverSelfTestErrorChanged();

    /*! \brief Notifier signal */
    void warningChanged(const Traffic::Warning&);

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     * If this class is connected to a traffic receiver, this method does
     * nothing.  Otherwise, it stops any ongoing connection attempt and starts a
     * new attempt to connect to a potential receiver, via all available
     * channels simultaneously.
     */
    void connectToTrafficReceiver();

    /*! \brief Disconnect from traffic receiver
     *
     * This method stops any ongoing connection or connection attempt.
     */
    void disconnectFromTrafficReceiver();

    /*! \brief Send password to the traffic data sources
     *
     *  This method will send a password/ssid combination to all traffic data
     *  source. If a source is waiting for a password with the given SSID, then
     *  it will send the password to the traffic data receiver.
     *
     *  @param SSID Network where the receiver should be connected
     *
     *  @param password Password that is sent to the receiver
     */
    void setPassword(const QString& SSID, const QString &password);

private slots:
    // Identical to addDataSource, but handles Bluetooth Classic connections only.
    QString addDataSource_BluetoothClassic(const Traffic::ConnectionInfo &connectionInfo);

    // Identical to addDataSource, but handles Bluetooth Low Energy connections only.
    static QString addDataSource_BluetoothLowEnergy(const Traffic::ConnectionInfo &connectionInfo);

    // Clear all data sources
    void clearDataSources();

    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of constructors in Global.
    void deferredInitialization();

    // Sends out foreflight broadcast message See
    // https://www.foreflight.com/connect/spec/
    void foreFlightBroadcast();

    // Load connection infos from file and create connections
    void loadConnectionInfos();

    // Called if one of the sources indicates a heartbeat change
    void onSourceHeartbeatChanged();

    // Called if one of the sources reports traffic (position unknown)
    void onTrafficFactorWithPosition(const Traffic::TrafficFactor_WithPosition& factor);

    // Called if one of the sources reports traffic (position known)
    void onTrafficFactorWithoutPosition(const Traffic::TrafficFactor_DistanceOnly& factor);

    // Called if one of the sources reports or clears an error string
    void onTrafficReceiverSelfTestError();

    // Called if one of the sources reports or clears an error string
    void onTrafficReceiverRuntimeError();

    // Resetter method
    void resetWarning();

    // Save connection infos to file
    void saveConnectionInfos();

    // Setter method
    void setReceivingHeartbeat(bool newReceivingHeartbeat);

    // Setter method
    void setWarning(const Traffic::Warning& warning);

    // Updates the property statusString that is inherited from
    // Positioning::PositionInfoSource_Abstract
    void updateStatusString();

private:
    // UDP Socket for ForeFlight Broadcast messages.
    // See https://www.foreflight.com/connect/spec/
    QNetworkDatagram foreFlightBroadcastDatagram {R"({"App":"Enroute Flight Navigation","GDL90":{"port":4000}})", QHostAddress::Broadcast, 63093};
    QUdpSocket foreFlightBroadcastSocket;
    QTimer foreFlightBroadcastTimer;

    // Targets
    QList<Traffic::TrafficFactor_WithPosition *> m_trafficObjects;
    QPointer<Traffic::TrafficFactor_DistanceOnly> m_trafficObjectWithoutPosition;

    // TrafficData Sources
    QList<QPointer<Traffic::TrafficDataSource_Abstract>> m_dataSources;
    QPointer<Traffic::TrafficDataSource_Abstract> m_currentSource;

    // Property cache
    Traffic::Warning m_Warning;
    QTimer m_WarningTimer;
    QString m_trafficReceiverRuntimeError {};
    QString m_trafficReceiverSelfTestError {};

    // Reconnect
    QTimer reconnectionTimer;

    // Property Cache
    bool m_receivingHeartbeat {false};

    // Standard file name for saveConnectionInfos()
    QString stdFileName{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/connectionInfos.data"};
};

} // namespace Traffic
