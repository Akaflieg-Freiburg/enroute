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

#include <QSettings>
#include <QVariant>
#include <QtMath>

//#include "AviationUnits.h"
#include "positioning/PositionInfoSource_Satellite.h"
#include "traffic/TrafficDataProvider.h"



Positioning::PositionInfoSource_Satellite::PositionInfoSource_Satellite(QObject *parent) : PositionInfoSource_Abstract(parent)
{
    source = QGeoPositionInfoSource::createDefaultSource(this);

    if (source != nullptr) {
        sourceStatus = source->error();
        connect(source, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(error(QGeoPositionInfoSource::Error)));
//        connect(source, &QGeoPositionInfoSource::updateTimeout, this, &PositionInfoSource_Satellite::timeout);
        connect(source, &QGeoPositionInfoSource::positionUpdated, this, &PositionInfoSource_Satellite::onPositionUpdated_Sat);
    }

    if (source != nullptr) {
        source->startUpdates();
        if ((source->supportedPositioningMethods() & QGeoPositionInfoSource::SatellitePositioningMethods) == QGeoPositionInfoSource::SatellitePositioningMethods) {
            _geoid = new Positioning::Geoid;
        }
    }
}


Positioning::PositionInfoSource_Satellite::~PositionInfoSource_Satellite()
{
    delete source;
    delete _geoid;
}


void Positioning::PositionInfoSource_Satellite::error(QGeoPositionInfoSource::Error newSourceStatus)
{
    // Save old status and set sourceStatus to QGeoPositionInfoSource::NoError
    sourceStatus = newSourceStatus;

    // If there really is an error, reset lastInfo and cancel all counters
    if (newSourceStatus != QGeoPositionInfoSource::NoError) {
/*
 *         _positionInfo = QGeoPositionInfo();
        timeoutCounter.stop();
        */
    }


}


auto Positioning::PositionInfoSource_Satellite::statusString() const -> QString
{
    if (source == nullptr) {
        return tr("Not installed or access denied");
    }

    if (sourceStatus == QGeoPositionInfoSource::AccessError) {
        return tr("Access denied");
    }

    if (sourceStatus == QGeoPositionInfoSource::ClosedError) {
        return tr("Connection to satellite system lost");
    }

    if (sourceStatus != QGeoPositionInfoSource::NoError) {
        return tr("Unknown error");
    }
/*
    if (!timeoutCounter.isActive()) {
        return tr("Waiting for signal");
    }
*/
    return tr("%1 OK").arg(source->sourceName());
}


void Positioning::PositionInfoSource_Satellite::onPositionUpdated_Sat(const QGeoPositionInfo &info)
{
    auto correctedInfo = info;
    if ((_geoid != nullptr) && (info.coordinate().type() == QGeoCoordinate::Coordinate3D)) {
        auto geoidCorrection = _geoid->operator()(static_cast<qreal>(info.coordinate().latitude()), static_cast<qreal>(info.coordinate().longitude()));
        correctedInfo.setCoordinate( correctedInfo.coordinate().atDistanceAndAzimuth(0, 0, -geoidCorrection) );
    }

    setPositionInfo( Positioning::PositionInfo(correctedInfo) );
}
