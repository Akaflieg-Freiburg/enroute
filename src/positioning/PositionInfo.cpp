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

#include "GlobalObject.h"
#include "geomaps/GeoMapProvider.h"
#include "positioning/PositionInfo.h"


Positioning::PositionInfo::PositionInfo(const QGeoPositionInfo &info)
{
    m_positionInfo = info;
}


auto Positioning::PositionInfo::groundSpeed() const -> Units::Speed
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        return {};
    }

    return Units::Speed::fromMPS(m_positionInfo.attribute(QGeoPositionInfo::GroundSpeed));
}


auto Positioning::PositionInfo::isValid() const -> bool
{
    if (!m_positionInfo.isValid()) {
        return false;
    }

    auto expiry = m_positionInfo.timestamp().addMSecs( std::chrono::milliseconds(lifetime).count() );
    return expiry >= QDateTime::currentDateTime();
}


auto Positioning::PositionInfo::positionErrorEstimate() const -> Units::Distance
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        return {};
    }

    return Units::Distance::fromM(m_positionInfo.attribute(QGeoPositionInfo::HorizontalAccuracy));
}


auto Positioning::PositionInfo::terrainElevationAMSL() -> Units::Distance
{
    if (m_terrainAMSL.isFinite())
    {
        return m_terrainAMSL;
    }

    m_terrainAMSL = GlobalObject::geoMapProvider()->terrainElevationAMSL(m_positionInfo.coordinate());
    return m_terrainAMSL;
}


auto Positioning::PositionInfo::trueAltitudeAMSL() const -> Units::Distance
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (m_positionInfo.coordinate().type() != QGeoCoordinate::Coordinate3D) {
        return {};
    }

    return Units::Distance::fromM(m_positionInfo.coordinate().altitude());

}

auto Positioning::PositionInfo::trueAltitudeAGL() -> Units::Distance
{
    if (m_trueAltitudeAGL.isFinite())
    {
        return m_trueAltitudeAGL;
    }

    auto tAltAMSL = trueAltitudeAMSL();
    if (!tAltAMSL.isFinite())
    {
        return {};
    }

    if (!m_terrainAMSL.isFinite())
    {
        m_terrainAMSL = GlobalObject::geoMapProvider()->terrainElevationAMSL(m_positionInfo.coordinate());
    }
    if (!m_terrainAMSL.isFinite())
    {
        return {};
    }

    m_trueAltitudeAGL = qMax(tAltAMSL - m_terrainAMSL, Units::Distance::fromM(0));

    return m_trueAltitudeAGL;
}

auto Positioning::PositionInfo::trueAltitudeErrorEstimate() const -> Units::Distance
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
        return {};
    }

    return Units::Distance::fromM(m_positionInfo.attribute(QGeoPositionInfo::VerticalAccuracy));
}


auto Positioning::PositionInfo::trueTrack() const -> Units::Angle
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!groundSpeed().isFinite()) {
        return {};
    }
    if (groundSpeed().toKN() < 4) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        return {};
    }

    return Units::Angle::fromDEG(m_positionInfo.attribute(QGeoPositionInfo::Direction));
}


auto Positioning::PositionInfo::variation() const -> Units::Angle
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::MagneticVariation)) {
        return {};
    }

    return Units::Angle::fromDEG(m_positionInfo.attribute(QGeoPositionInfo::MagneticVariation));
}


auto Positioning::PositionInfo::verticalSpeed() const -> Units::Speed
{
    if (!m_positionInfo.isValid()) {
        return {};
    }
    if (!m_positionInfo.hasAttribute(QGeoPositionInfo::VerticalSpeed)) {
        return {};
    }

    return Units::Speed::fromMPS(m_positionInfo.attribute(QGeoPositionInfo::VerticalSpeed));
}

