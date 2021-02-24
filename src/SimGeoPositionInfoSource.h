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

    QString sourceName() const;

    QGeoPositionInfoSource::Error error() const;

public slots:
    void startUpdates();

    void stopUpdates();

    void requestUpdate(int timeout = 0);

private slots:
    void setSourceName(const QString &sourceName);

Q_SIGNALS:
    void positionUpdated(const QGeoPositionInfo &update);

    void updateTimeout();

    void error(QGeoPositionInfoSource::Error);

    void supportedPositioningMethodsChanged();

private:
    QString _sourceName = "n/a";
    SimInterface *_simInterface = nullptr;
    QGeoPositionInfoSource::Error _error = QGeoPositionInfoSource::Error::NoError;
};
