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

QString SimGeoPositionInfoSource::sourceName() const
{
    return _sourceName;
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

    connect(_simInterface, &SimInterface::simNameChanged, this, &SimGeoPositionInfoSource::setSourceName);
    connect(_simInterface, &SimInterface::positionUpdated, this, &SimGeoPositionInfoSource::positionUpdated);
    connect(_simInterface, &SimInterface::timeout, this, &SimGeoPositionInfoSource::updateTimeout);

    _simInterface->start();
}

void SimGeoPositionInfoSource::stopUpdates()
{
    _simInterface->stop();
}

void SimGeoPositionInfoSource::requestUpdate(int)
{
}

void SimGeoPositionInfoSource::setSourceName(const QString &sourceName)
{
    _sourceName = sourceName;
}
