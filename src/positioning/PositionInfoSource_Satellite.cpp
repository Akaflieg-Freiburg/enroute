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

#include "positioning/PositionInfoSource_Satellite.h"


Positioning::PositionInfoSource_Satellite::PositionInfoSource_Satellite(QObject *parent) : PositionInfoSource_Abstract(parent)
{
    source = QGeoPositionInfoSource::createDefaultSource(this);
    if (source != nullptr) {
        source->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);
        source->setUpdateInterval(1000);

        QString sName = source->sourceName();
        if (sName.isEmpty()) {
            setSourceName( tr("Built-in receiver") );
        } else {
            setSourceName( tr("Built-in receiver/%1").arg( sName.at(0).toUpper()+sName.mid(1) ) );
        }

        connect(source, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(updateStatusString()));
        connect(source, &QGeoPositionInfoSource::positionUpdated, this, &PositionInfoSource_Satellite::onPositionUpdated);
        source->startUpdates();
    } else {
        setSourceName( tr("None") );
    }

    updateStatusString();
}


void Positioning::PositionInfoSource_Satellite::updateStatusString()
{
    if (source == nullptr) {
        setStatusString( tr("Not installed or access denied") );
        return;
    }

    auto sourceStatus = source->error();

    if (sourceStatus == QGeoPositionInfoSource::AccessError) {
        setStatusString( tr("Access denied") );
        return;
    }

    if (sourceStatus == QGeoPositionInfoSource::ClosedError) {
        setStatusString( tr("Connection to satellite system lost") );
        return;
    }

    if (!receivingPositionInfo()) {
        setStatusString( tr("Waiting for signal") );
        return;
    }

    setStatusString( tr("Receiving data") );
}


void Positioning::PositionInfoSource_Satellite::onPositionUpdated(const QGeoPositionInfo &info)
{
    auto correctedInfo = info;
    if (info.coordinate().type() == QGeoCoordinate::Coordinate3D) {
        auto geoidCorrection = geoid.operator()(static_cast<qreal>(info.coordinate().latitude()), static_cast<qreal>(info.coordinate().longitude()));
        correctedInfo.setCoordinate( correctedInfo.coordinate().atDistanceAndAzimuth(0, 0, -geoidCorrection) );
    }

    setPositionInfo( Positioning::PositionInfo(correctedInfo) );
    updateStatusString();
}
