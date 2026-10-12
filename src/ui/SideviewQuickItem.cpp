/***************************************************************************
 *   Copyright (C) 2025 by Simon Schneider, Stefan Kebekus                 *
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
#include "SideviewQuickItem.h"
#include "ui/ProfileSampling.h"
#include "weather/WeatherDataProvider.h"

using namespace Qt::Literals::StringLiterals;

Ui::SideviewQuickItem::SideviewQuickItem(QQuickItem *parent)
    : QQuickItem(parent), m_baroCache(new Navigation::BaroCache(this))
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &Ui::SideviewQuickItem::updateProperties);
    connect(GlobalObject::weatherDataProvider(), &Weather::WeatherDataProvider::QNHInfoChanged, this, &Ui::SideviewQuickItem::updateProperties);

    notifiers.push_back(bindableHeight().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(bindableWidth().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePositionInfo().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePressureAltitude().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePressureAltitudeImplausible().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(m_pixelPer10km.addNotifier([this]() {updateProperties();}));
    updateProperties();   
}

void Ui::SideviewQuickItem::updateProperties()
{
    // Rate-limiting code. Ensure that this expensive method runs at most every minimumUpdateInterval_ms and not more often.
    if (m_timer.isActive())
    {
        return;
    }
    if (m_elapsedTimer.isValid())
    {
        auto elapsed = (int)m_elapsedTimer.elapsed();
        if (elapsed < minimumUpdateInterval_ms)
        {
            m_timer.start((minimumUpdateInterval_ms + 10) - elapsed);
            return;
        }
    }
    m_elapsedTimer.start();

    //
    // Set all properties to default values. We update those depending on the data we have available.
    //
    const QScopedPropertyUpdateGroup updateLock;
    m_fiveMinuteBar = {0, 0};
    m_ownshipPosition = {-100, -100};
    m_track = QString();
    m_error = QString();
    m_terrain = QPolygonF();

    m_airspaces = Ui::ProfileSampling::emptyAirspacePolygons();

    // If the side view is really small, safe CPU cycles by doing nothing
    if (height() < 5.0)
    {
        return;
    }

    // If zoom is totally out of range, then exit
    if (!qIsFinite(m_pixelPer10km) || (m_pixelPer10km < 5.0))
    {
        m_error = tr("Unable to show side view: Zoom value out of range.");
        return;
    }

    //
    // Get data required in the drawing
    //
    auto ownshipCoordinate = Positioning::PositionProvider::lastValidCoordinate();
    if (!ownshipCoordinate.isValid())
    {
        m_error = tr("Unable to show side view: No valid position data.");
        return;
    }
    if (GlobalObject::positionProvider()->pressureAltitudeImplausible())
    {
        m_error = tr("Unable to show side view: Pressure altitude and GNSS altitude differ by an unrealistic amount. "
                     "This can happen when the device does not measure static pressure, "
                     "for instance in a pressurized cabin or when a flight simulator is used.");
        return;
    }
    auto ownshipTerrainElevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(ownshipCoordinate);
    if (!ownshipTerrainElevation.isFinite())
    {
        m_error = tr("Unable to show side view: No terrain data for current position. Please install the relevant terrain maps.");
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
        if (ownshipTrack.isFinite())
        {
            m_track = u"Direction → %1°"_s.arg( qRound(ownshipTrack.toDEG()));
        }
        else
        {
            m_track = QString();
        }
    }
    if (!ownshipTrack.isFinite())
    {
        m_error = tr("Unable to show side view: No valid track data.");
        m_track = QString();
        return;
    }
    auto ownshipGeometricAltitude = Units::Distance::fromM(ownshipCoordinate.altitude());
    if (!ownshipGeometricAltitude.isFinite())
    {
        m_error = tr("Unable to show side view: No valid altitude data for own aircraft.");
        return;
    }
    auto ownshipPressureAltitude = GlobalObject::positionProvider()->pressureAltitude();
    if (!ownshipPressureAltitude.isFinite())
    {
        ownshipPressureAltitude = m_baroCache->estimatedPressureAltitude(ownshipGeometricAltitude);
    }
    if (!ownshipPressureAltitude.isFinite())
    {
        ownshipPressureAltitude = ownshipGeometricAltitude;
        m_error = tr("Unable to compute sufficiently precise vertical airspace boundaries because barometric altitude information is not available. <a href='xx'>Click here</a> for more information.");
    }
    auto QNH = GlobalObject::weatherDataProvider()->QNH();
    if (!QNH.isFinite())
    {
        m_error = tr("Unable to compute sufficiently precise vertical airspace boundaries because the QNH is not available. Please wait while QNH information is downloaded from the internet.");
        return;
    }
    if (ownshipTerrainElevation.isFinite())
    {
        if (ownshipGeometricAltitude < ownshipTerrainElevation)
        {
            ownshipGeometricAltitude = ownshipTerrainElevation;
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
    // Sample the profile every few pixels along the track
    //
    const int step = 4.0;
    QList<Ui::ProfileSampling::Sample> samples;
    samples.reserve( qRound((width()+20)/step) );
    for(int x = -10; x <= width()+10; x += step)
    {
        auto dist = 10000.0*(x-0.2*width())/(m_pixelPer10km.value());
        samples << Ui::ProfileSampling::Sample{static_cast<double>(x), ownshipCoordinate.atDistanceAndAzimuth(dist, ownshipTrack.toDEG()), {}};
    }
    if (!Ui::ProfileSampling::fillTerrain(samples))
    {
        m_error = tr("Incomplete terrain data. Please install the relevant terrain maps.");
    }
    Units::Distance minElevation = ownshipTerrainElevation;
    Units::Distance maxElevation = ownshipTerrainElevation;
    for(const auto& sample : std::as_const(samples))
    {
        minElevation = qMin(minElevation, sample.terrain);
        maxElevation = qMax(maxElevation, sample.terrain);
    }

    //
    // Compute minimum and maximum altitude shown in the QML item,
    // generate function altToYCoordinate() that computes pixel the coordinate from the altitude.
    //
    auto sideview_minAlt = ownshipGeometricAltitude;
    auto sideview_maxAlt = ownshipGeometricAltitude;
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
    m_ownshipPosition = {width()*0.2, altToYCoordinate(ownshipGeometricAltitude)};

    // 5-Minute-Bar
    if (ownshipVSpeed.isFinite() && ownshipHSpeed.isFinite())
    {
        m_fiveMinuteBar = {m_pixelPer10km.value()*(ownshipHSpeed.toMPS()*5*60)/10000, -height()*ownshipVSpeed.toFPM()*5.0/((sideview_maxAlt - sideview_minAlt).toFeet())};
    }

    // Terrain
    m_terrain = Ui::ProfileSampling::terrainPolygon(samples, altToYCoordinate, width(), height());

    // Airspaces
    m_airspaces = Ui::ProfileSampling::airspacePolygons(samples, altToYCoordinate, QNH, ownshipGeometricAltitude, ownshipPressureAltitude);
    //qWarning() << "SideviewQuickItem full" << m_elapsedTimer.elapsed();
}
