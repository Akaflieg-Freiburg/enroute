#pragma once

#include "SimInterface.h"

#include <QGeoPositionInfoSource>
#include <QUdpSocket>


class SimGeoPositionInfoSource : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    SimGeoPositionInfoSource(QObject * partent);

    ~SimGeoPositionInfoSource();

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const;

    PositioningMethods supportedPositioningMethods() const;

    int minimumUpdateInterval() const;

    QString simName() const;

    QGeoPositionInfoSource::Error error() const;

public slots:
    void startUpdates();

    void stopUpdates();

    void requestUpdate(int timeout = 0);

private slots:

Q_SIGNALS:
    void positionUpdated(const QGeoPositionInfo &update);

    void updateTimeout();

    void error(QGeoPositionInfoSource::Error);

    void supportedPositioningMethodsChanged();

    void trafficUpdated(const Navigation::Traffic &traffic);

private:
    QString _simName = "n/a";
    SimInterface *_simInterface = nullptr;
    QGeoPositionInfoSource::Error _error = QGeoPositionInfoSource::Error::NoError;
};
