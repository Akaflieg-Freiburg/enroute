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
    const QScopedPropertyUpdateGroup updateLock;

    m_fiveMinuteBar = {0, 0};
    m_ownshipPosition = {-100, -100};
    m_track = QString();
    m_error = QString();
    m_terrain = QPolygonF();

    if (height() == 0.0)
    {
        return;
    }

    auto positionInfo = GlobalObject::positionProvider()->positionInfo();
    if (!positionInfo.isValid())
    {
        m_error = tr("No valid position data.");
        return;
    }

    Units::Distance ownShipAltitude = positionInfo.trueAltitudeAMSL();
    if (!ownShipAltitude.isFinite())
    {
        m_error = tr("No valid altitude data.");
        return;
    }
    if (ownShipAltitude < positionInfo.terrainElevationAMSL())
    {
        ownShipAltitude = positionInfo.terrainElevationAMSL();
    }

    auto track = positionInfo.trueTrack();
    if (!track.isFinite())
    {
        track = Positioning::PositionProvider::lastValidTT();
        m_track = u"Direction → %1°"_s.arg(qRound(track.toDEG()));
    }

    // Compute elevations
    QVector<Units::Distance> elevations;
    Units::Distance minElevation = ownShipAltitude;
    Units::Distance maxElevation = ownShipAltitude;
    const int step = 1;
    elevations.reserve(qRound(width()/step)+1);
    for(int x = 0; x <= width()+step; x += step)
    {
        auto dist = 10000.0*(x-0.2*width())/(m_pixelPer10km.value());
        auto position = positionInfo.coordinate().atDistanceAndAzimuth(dist, track.toDEG());
        auto elevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(position);
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

    auto sideview_minAlt = ownShipAltitude;
    auto sideview_maxAlt = ownShipAltitude;
    if (positionInfo.verticalSpeed().isFinite())
    {
        sideview_maxAlt += qMax(Units::Distance::fromFT(3000.0),
                                Units::Distance::fromFT(positionInfo.verticalSpeed().toFPM()*7.5));
        sideview_minAlt += qMin(Units::Distance::fromFT(-3000.0),
                                Units::Distance::fromFT(positionInfo.verticalSpeed().toFPM()*7.5));
    }
    else
    {
        sideview_maxAlt += Units::Distance::fromFT(3000.0);
        sideview_minAlt += Units::Distance::fromFT(-3000.0);
    }
    sideview_minAlt = qMax(sideview_minAlt, minElevation - Units::Distance::fromFT(100) );


    auto altToYCoordinate = [this, sideview_minAlt, sideview_maxAlt](Units::Distance alt)
    {
        return ((double)height())*(sideview_maxAlt - alt) / (sideview_maxAlt - sideview_minAlt);
    };

    m_ownshipPosition = {width()*0.2, altToYCoordinate(ownShipAltitude) };

    // Show 5-Minute-Bar, but only is groundspeed is known, and at least 10 kts
    if (positionInfo.groundSpeed().isFinite() && (positionInfo.groundSpeed() > Units::Speed::fromKN(10)))
    {
        m_fiveMinuteBar = {m_pixelPer10km.value()*(positionInfo.groundSpeed().toMPS()*5*60)/10000, -height()*positionInfo.verticalSpeed().toFPM()*5.0/((sideview_maxAlt - sideview_minAlt).toFeet())};
    }

    // Compute Terrain
    QPolygonF polygon;
    polygon.reserve( elevations.size()+4 );
    for(int i = 0; i < elevations.size(); i++)
    {
        auto elevation = elevations[i];
        auto y = altToYCoordinate(elevation);
        polygon << QPointF(i*step, y);
    }
    polygon  << QPointF(width(), height()+20) << QPointF(0, height()+20);
    m_terrain = polygon;

    //    m_error = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint obcaecat cupiditat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
}


