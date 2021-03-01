#include "SimInterface.h"

#include <QNetworkDatagram>
#include <QByteArray>
#include <QNetworkInterface>

SimInterface::SimInterface(QObject *parent) : QObject(parent)
{
}

SimInterface::~SimInterface()
{
    delete _udpSocket;
}

bool SimInterface::start()
{
    if (!_udpSocket) {
        _udpSocket = new QUdpSocket(this);
    }

    bool bBindSuccess = _udpSocket->bind(49002);
    connect(_udpSocket, &QUdpSocket::readyRead, this, &SimInterface::readPendingDatagrams);

    _timeoutPosUpdate.setSingleShot(true);
    connect(&_timeoutPosUpdate, &QTimer::timeout, this, &SimInterface::timeout);

    return bBindSuccess;
}

void SimInterface::stop()
{
    _timeoutPosUpdate.stop();
    _udpSocket->close();
}

QGeoPositionInfo SimInterface::lastKnownPosition() const
{
    return _geoPos;
}

void SimInterface::readPendingDatagrams()
{
    while (_udpSocket->hasPendingDatagrams())
    {
        QString str = _udpSocket->receiveDatagram().data();
        QStringList list = str.split(QLatin1Char(','));

        if (list[0].contains("XGPS")) {
            if (_simName != list[0].remove(0,4)) {
                if ("1" == list[0]) {
                    if (_simName != "X-Plane") {
                        _simName = "X-Plane";
                    }
                }
                else {
                    _simName = list[0];
                }
            }

            _geoPos.setCoordinate(QGeoCoordinate(list[2].toDouble(),
                                                 list[1].toDouble(),
                                                 list[3].toDouble()));
            _geoPos.setAttribute(QGeoPositionInfo::Direction, list[4].toDouble());
            _geoPos.setAttribute(QGeoPositionInfo::GroundSpeed, list[5].toDouble());

            _geoPos.setTimestamp(QDateTime::currentDateTimeUtc());

            _timeoutPosUpdate.start(_timeoutThreshold);
            emit positionUpdated(_geoPos);
        }
        else if (list[0].contains("XTRA")) {
            QString targetID = list[1];
            QGeoPositionInfo geoPositionInfo(QGeoCoordinate(list[2].toDouble(),
                                                            list[3].toDouble(),
                                                            list[4].toDouble()*0.3048),
                                             QDateTime::currentDateTimeUtc());

            geoPositionInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, list[5].toDouble()/0.3048/60.0);
            geoPositionInfo.setAttribute(QGeoPositionInfo::Direction, list[7].toDouble());
            geoPositionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, list[8].toDouble()*1.852/3.6);

            auto traffic = Navigation::Traffic();

            traffic.setData(0,
                            targetID,
                            AviationUnits::Distance::fromM(_geoPos.coordinate().distanceTo(geoPositionInfo.coordinate())),
                            AviationUnits::Distance::fromM(geoPositionInfo.coordinate().altitude() - _geoPos.coordinate().altitude()),
                            AviationUnits::Speed::fromMPS(_geoPos.attribute(QGeoPositionInfo::VerticalSpeed)),
                            Navigation::Traffic::unknown,
                            geoPositionInfo );

            emit trafficUpdated(traffic);
        }
    }
}
