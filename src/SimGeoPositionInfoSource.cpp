#include "SimGeoPositionInfoSource.h"

SimGeoPositionInfoSource::SimGeoPositionInfoSource(QObject * parent)
    : QGeoPositionInfoSource(parent)
{
}

SimGeoPositionInfoSource::~SimGeoPositionInfoSource()
{
    delete _simInterface;
}

QGeoPositionInfo SimGeoPositionInfoSource::lastKnownPosition(bool) const
{
    return _simInterface->lastKnownPosition();
}

QGeoPositionInfoSource::PositioningMethods SimGeoPositionInfoSource::supportedPositioningMethods() const
{
    return PositioningMethod::SatellitePositioningMethods;
}

int SimGeoPositionInfoSource::minimumUpdateInterval() const
{
    return 1000;
}

QString SimGeoPositionInfoSource::simName() const
{
    return _simInterface->simName();
}

QGeoPositionInfoSource::Error SimGeoPositionInfoSource::error() const
{
    return _error;
}

void SimGeoPositionInfoSource::startUpdates()
{
    if (!_simInterface) {
        _simInterface = new SimInterface(this);
    }

    connect(_simInterface, &SimInterface::positionUpdated, this, &SimGeoPositionInfoSource::positionUpdated);
    connect(_simInterface, &SimInterface::timeout, this, &SimGeoPositionInfoSource::updateTimeout);

    connect(_simInterface, &SimInterface::trafficUpdated, this, &SimGeoPositionInfoSource::trafficUpdated);

    _simInterface->start();
}

void SimGeoPositionInfoSource::stopUpdates()
{
    _simInterface->stop();
}

void SimGeoPositionInfoSource::requestUpdate(int)
{
}
