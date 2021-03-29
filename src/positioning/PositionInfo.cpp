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

#include "positioning/PositionInfo.h"


Positioning::PositionInfo::PositionInfo(const QGeoPositionInfo &info)
{
    _positionInfo = info;
}


auto Positioning::PositionInfo::groundSpeed() const -> AviationUnits::Speed
{
    if (!_positionInfo.isValid()) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        return {};
    }

    return AviationUnits::Speed::fromMPS(_positionInfo.attribute(QGeoPositionInfo::GroundSpeed));
}


auto Positioning::PositionInfo::positionErrorEstimate() const -> AviationUnits::Distance
{
    if (!_positionInfo.isValid()) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        return {};
    }

    return AviationUnits::Distance::fromM(_positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy));
}


auto Positioning::PositionInfo::trueAltitude() const -> AviationUnits::Distance
{
    if (_positionInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return {};
    }

    return AviationUnits::Distance::fromM(_positionInfo.coordinate().altitude());

}


auto Positioning::PositionInfo::trueAltitudeErrorEstimate() const -> AviationUnits::Distance
{
    if (!_positionInfo.isValid()) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
        return {};
    }

    return AviationUnits::Distance::fromM(_positionInfo.attribute(QGeoPositionInfo::VerticalAccuracy));
}


auto Positioning::PositionInfo::trueTrack() const -> AviationUnits::Angle
{
    if (!groundSpeed().isFinite()) {
        return {};
    }
    if (groundSpeed().toKN() < 4) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        return {};
    }

    return AviationUnits::Angle::fromDEG(_positionInfo.attribute(QGeoPositionInfo::Direction));
}


auto Positioning::PositionInfo::variation() const -> AviationUnits::Angle
{
    if (!_positionInfo.isValid()) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::MagneticVariation)) {
        return {};
    }

    return AviationUnits::Angle::fromDEG(_positionInfo.attribute(QGeoPositionInfo::MagneticVariation));
}


auto Positioning::PositionInfo::verticalSpeed() const -> AviationUnits::Speed
{
    if (!_positionInfo.isValid()) {
        return {};
    }
    if (!_positionInfo.hasAttribute(QGeoPositionInfo::VerticalSpeed)) {
        return {};
    }

    return AviationUnits::Speed::fromMPS(_positionInfo.attribute(QGeoPositionInfo::VerticalSpeed));
}

