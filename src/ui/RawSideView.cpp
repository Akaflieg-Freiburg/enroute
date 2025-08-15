/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#include "GeoMapProvider.h"
#include "PositionProvider.h"
#include "RawSideView.h"

using namespace Qt::Literals::StringLiterals;

Ui::RawSideView::RawSideView(QQuickItem *parent)
    : QQuickItem(parent)
{
    notifiers.push_back(bindableHeight().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(bindableWidth().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePositionInfo().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(m_pixelPer10km.addNotifier([this]() {updateProperties();}));
    updateProperties();
}

void Ui::RawSideView::updateProperties()
{
    //
    // Set all properties to default values. We update those depending on the data we have available.
    //
    const QScopedPropertyUpdateGroup updateLock;
    m_fiveMinuteBar = {0, 0};
    m_ownshipPosition = {-100, -100};
    m_track = QString();
    m_error = QString();
    m_terrain = QPolygonF();

    // If the side view is really small, safe CPU cycles by doing nothing
    if (height() < 5.0)
    {
        return;
    }

    //
    // Get data required in the drawing
    //
    auto ownshipCoordinate = Positioning::PositionProvider::lastValidCoordinate();
    if (!ownshipCoordinate.isValid())
    {
        m_error = tr("No valid position data.");
        return;
    }
    auto ownshipTerrainElevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(ownshipCoordinate);
    if (!ownshipTerrainElevation.isFinite())
    {
        m_error = tr("No terrain data for current position. Please install the relevant terrain maps.");
        return;
    }
    auto ownshipPositionInfo = GlobalObject::positionProvider()->positionInfo();

    Units::Angle ownshipTrack;
    if (ownshipPositionInfo.isValid() && ownshipPositionInfo.trueTrack().isFinite())
    {
        ownshipTrack = ownshipPositionInfo.trueTrack();
    }
    else
    {
        ownshipTrack = Positioning::PositionProvider::lastValidTT();
        m_track = u"Direction → %1°"_s.arg( qRound(ownshipTrack.toDEG()));
    }
    if (!ownshipTrack.isFinite())
    {
        m_error = tr("No valid track data.");
        m_track = QString();
        return;
    }
    auto ownshipAltitude = Units::Distance::fromM(ownshipCoordinate.altitude());
    if (!ownshipAltitude.isFinite())
    {
        m_error = tr("No valid altitude data.");
        return;
    }
    if (ownshipTerrainElevation.isFinite())
    {
        if (ownshipAltitude < ownshipTerrainElevation)
        {
            ownshipAltitude = ownshipTerrainElevation;
        }
    }
    Units::Speed ownshipHSpeed;
    Units::Speed ownshipVSpeed;
    if (ownshipPositionInfo.isValid())
    {
        ownshipHSpeed = ownshipPositionInfo.groundSpeed();
        ownshipVSpeed = ownshipPositionInfo.verticalSpeed();
        if (!ownshipHSpeed.isFinite() || (ownshipHSpeed < Units::Speed::fromKN(10)))
        {
            // If horizontal speed is low, vertical speed info is typically too jittery to be
            // useful. We just ignore it then.
            ownshipVSpeed = {};
        }
    }

    //
    // Compute array of x-coordinates in pixels.
    //
    const int step = 5.0;
    QList<int> xCoordinates;
    xCoordinates.reserve( qRound((width()+20)/step) );
    for(int x = -10; x <= width()+10; x += step)
    {
        xCoordinates << x;
    }

    //
    // For each x-coordinate, compute the associated QGeoCoordinate
    //
    QList<QGeoCoordinate> geoCoordinates;
    geoCoordinates.reserve(xCoordinates.size());
    for(auto x : std::as_const(xCoordinates))
    {
        auto dist = 10000.0*(x-0.2*width())/(m_pixelPer10km.value());
        auto geoCoordinate = ownshipCoordinate.atDistanceAndAzimuth(dist, ownshipTrack.toDEG());
        geoCoordinates << geoCoordinate;
    }

    // For each geoCoordinate, compute the elevation
    QList<Units::Distance> elevations;
    elevations.reserve(geoCoordinates.size());
    Units::Distance minElevation = ownshipTerrainElevation;
    Units::Distance maxElevation = ownshipTerrainElevation;
    for(const auto& geoCoordinate : std::as_const(geoCoordinates))
    {
        auto elevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(geoCoordinate);
        if (!elevation.isFinite())
        {
            elevation = Units::Distance::fromM(0.0);
            m_error = tr("Incomplete terrain data. Please install the relevant terrain maps.");
        }
        elevations << elevation;
        if (elevation < minElevation)
        {
            minElevation = elevation;
        }
        if (elevation > maxElevation)
        {
            maxElevation = elevation;
        }
    }

    //
    // Compute minimum and maximum altitude shown in the QML item,
    // generate function altToYCoordinate() that computes pixel coordinate from altitude
    //
    auto sideview_minAlt = ownshipAltitude;
    auto sideview_maxAlt = ownshipAltitude;
    if (ownshipPositionInfo.verticalSpeed().isFinite())
    {
        sideview_maxAlt += qMax(Units::Distance::fromFT(3000.0),
                                Units::Distance::fromFT(ownshipPositionInfo.verticalSpeed().toFPM()*7.5));
        sideview_minAlt += qMin(Units::Distance::fromFT(-3000.0),
                                Units::Distance::fromFT(ownshipPositionInfo.verticalSpeed().toFPM()*7.5));
    }
    else
    {
        sideview_maxAlt += Units::Distance::fromFT(3000.0);
        sideview_minAlt += Units::Distance::fromFT(-3000.0);
    }
    sideview_minAlt = qMax(sideview_minAlt, minElevation - Units::Distance::fromFT(100));
    auto altToYCoordinate = [this, sideview_minAlt, sideview_maxAlt](Units::Distance alt)
    {
        return ((double)height())*(sideview_maxAlt - alt) / (sideview_maxAlt - sideview_minAlt);
    };

    //
    // Compute graphics data
    //

    // Ownship position
    m_ownshipPosition = {width()*0.2, altToYCoordinate(ownshipAltitude)};

    // 5-Minute-Bar
    if (ownshipVSpeed.isFinite() && ownshipHSpeed.isFinite())
    {
        m_fiveMinuteBar = {m_pixelPer10km.value()*(ownshipHSpeed.toMPS()*5*60)/10000, -height()*ownshipVSpeed.toFPM()*5.0/((sideview_maxAlt - sideview_minAlt).toFeet())};
    }

    // Terrain
    QPolygonF polygon;
    polygon.reserve( elevations.size()+4 );
    for(int i = 0; i < elevations.size(); i++)
    {
        polygon << QPointF(xCoordinates[i], altToYCoordinate(elevations[i]));
    }
    polygon  << QPointF(width(), height()+20) << QPointF(-20, height()+20);
    m_terrain = polygon;

    //m_error = tr("Unable to compute vertical airspace boundaries because static pressure information is not available.");
}


