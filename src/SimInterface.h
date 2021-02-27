#pragma once


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

private:
    QUdpSocket *_udpSocket = nullptr;
    QTimer _timeoutPosUpdate;
    const int _timeoutThreshold = 60*1000;
    QGeoPositionInfo _geoPos;
    QString _simName;
};
