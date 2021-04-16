#pragma once


#include <QPointer>
#include <QUdpSocket>

#include "traffic/TrafficDataSource_Abstract.h"


namespace Traffic {

/*! \brief Traffic receiver: TCP connection to FLARM/NMEA source
 *
 *  This class connects to a traffic receiver via a TCP connection. It expects to
 *  find a receiver at the specifed IP-Address and port.
 *
 *  In most use cases, the
 *  connection will be established via the device's WiFi interface.  The class will
 *  therefore try to lock the WiFi once a heartbeat has been detected, and release the WiFi at the appropriate time.
 */
class TrafficDataSource_Udp_GDL : public TrafficDataSource_Abstract {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param hostName Name of the host where the traffic receiver is expected
     *
     *  @param port Port at the host where the traffic receiver is expected
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Udp_GDL(quint16 port, QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Udp_GDL() override;

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its superclass.
     *
     *  @returns Property sourceName
     */
    QString sourceName() const override
    {
        return tr("UDP connection to port %1").arg(m_port);
    }

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its superclass.
     */
    void connectToTrafficReceiver() override;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its superclass.
     */
    void disconnectFromTrafficReceiver() override;

private slots:
    // Handle socket errors.
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    // Update the property "errorString" and "connectivityStatus" and emit notification signals
    void onStateChanged(QAbstractSocket::SocketState socketState);

    // Acquire or release WiFi lock
    static void onReceivingHeartbeatChanged(bool receivingHB);

    void onReadyRead();

private:
    quint16 crcCompute(char *block, int length);

    QPointer<QUdpSocket> socket;
    quint16 m_port;

    // GPS altitude of owncraft
    AviationUnits::Distance m_trueAltitude;
    AviationUnits::Distance m_trueAltitude_FOM;
    QTimer m_trueAltitudeTimer;

    // Pressure altitude of owncraft
    AviationUnits::Distance m_pressureAltitude;
    QTimer m_pressureAltitudeTimer;

    // Targets
    Traffic::TrafficFactor factor;

    QVector<quint16> Crc16Table;
};

}

/*
#include "Navigation_Traffic.h"

#include <QUdpSocket>
#include <QTimer>
#include <QGeoPositionInfo>

class SimInterface : public QObject
{
    Q_OBJECT
public:
    SimInterface(QObject *parent);

    ~SimInterface();

    bool start();

    void stop();

    QGeoPositionInfo lastKnownPosition() const;

    QString simName() const { return _simName; };

public slots:
    void readPendingDatagrams();

signals:
    void positionUpdated(const QGeoPositionInfo &update);

    void timeout();

    void trafficUpdated(const Navigation::Traffic &traffic);

private:
    QUdpSocket *_udpSocket = nullptr;
    QTimer _timeoutPosUpdate;
    const int _timeoutThreshold = 60*1000;
    QGeoPositionInfo _geoPos;
    QString _simName;
};
*/
