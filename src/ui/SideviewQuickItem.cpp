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
#include "weather/WeatherDataProvider.h"

using namespace Qt::Literals::StringLiterals;

QStringList airspaceCategories = {"ATZ", "TMZ", "RMZ", "TIA", "TIZ", "NRA", "DNG", "D", "C", "B", "A", "CTR", "R", "P", "PJE", "SUA"};


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

    QVariantMap newAirspaces;
    const QVector<QPolygonF> empty;
    newAirspaces[u"A"_s] = QVariant::fromValue(empty);
    newAirspaces[u"CTR"_s] = QVariant::fromValue(empty);
    newAirspaces[u"R"_s] = QVariant::fromValue(empty);
    newAirspaces[u"RMZ"_s] = QVariant::fromValue(empty);
    newAirspaces[u"NRA"_s] = QVariant::fromValue(empty);
    newAirspaces[u"PJE"_s] = QVariant::fromValue(empty);
    newAirspaces[u"TMZ"_s] = QVariant::fromValue(empty);
    m_airspaces = newAirspaces;

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
        m_track = u"Direction → %1°"_s.arg( qRound(ownshipTrack.toDEG()));
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
    // Compute array of x-coordinates in pixels.
    //
    const int step = 4.0;
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
    QPolygonF polygon;
    polygon.reserve( elevations.size()+4 );
    for(qsizetype i = 0; i < elevations.size(); i++)
    {
        polygon << QPointF(xCoordinates[i], altToYCoordinate(elevations[i]));
    }
    polygon  << QPointF(width(), height()+2000) << QPointF(-20, height()+2000);
    m_terrain = polygon;
    qWarning() << "SideviewQuickItem terrain" << m_elapsedTimer.elapsed();

    // Airspaces
    auto airspaces = GlobalObject::geoMapProvider()->airspaces();
    QVector<QPolygonF> airspacePolygonsA;
    QVector<QPolygonF> airspacePolygonsCTR;
    QVector<QPolygonF> airspacePolygonsR;
    QVector<QPolygonF> airspacePolygonsRMZ;
    QVector<QPolygonF> airspacePolygonsNRA;
    QVector<QPolygonF> airspacePolygonsPJE;
    QVector<QPolygonF> airspacePolygonsTMZ;
    QList<QPointF> upper;
    QList<QPointF> lower;
    for(const auto& airspace : std::as_const(airspaces))
    {
        if (!airspaceCategories.contains(airspace.CAT()))
        {
            continue;
        }
        auto airspacePolygon = airspace.polygon();
        for(int i=0; i < xCoordinates.size(); i++)
        {
            auto x = xCoordinates[i];
            const auto& geoCoordinate = geoCoordinates[i];
            if ((i != xCoordinates.size()-1) && airspacePolygon.contains(geoCoordinate))
            {
                upper << QPointF(x, altToYCoordinate(airspace.estimatedUpperBoundMSL(elevations[i], QNH, ownshipGeometricAltitude, ownshipPressureAltitude)));
                lower << QPointF(x, altToYCoordinate(airspace.estimatedLowerBoundMSL(elevations[i], QNH, ownshipGeometricAltitude, ownshipPressureAltitude)));
            }
            else
            {
                if (!upper.isEmpty())
                {
                    std::reverse(lower.begin(), lower.end());
                    QPolygonF polygon(upper + lower);
                    polygon << upper[0];
                    if ((airspace.CAT() == u"A"_s) || (airspace.CAT() == u"B"_s) || (airspace.CAT() == u"C"_s) || (airspace.CAT() == u"D"_s))
                    {
                        airspacePolygonsA << polygon;
                    }
                    if (airspace.CAT() == u"CTR"_s)
                    {
                        airspacePolygonsCTR << polygon;
                    }
                    if ((airspace.CAT() == u"R"_s) || (airspace.CAT() == u"P"_s) || (airspace.CAT() == u"DNG"_s))
                    {
                        airspacePolygonsR << polygon;
                    }
                    if ((airspace.CAT() == u"ATZ"_s) || (airspace.CAT() == u"RMZ"_s) || (airspace.CAT() == u"TIA"_s) || (airspace.CAT() == u"TIZ"_s))
                    {
                        airspacePolygonsRMZ << polygon;
                    }
                    if (airspace.CAT() == u"NRA"_s)
                    {
                        airspacePolygonsNRA << polygon;
                    }
                    if ((airspace.CAT() == u"PJE"_s) || (airspace.CAT() == u"SUA"_s))
                    {
                        airspacePolygonsPJE << polygon;
                    }
                    if (airspace.CAT() == u"TMZ"_s)
                    {
                        airspacePolygonsTMZ << polygon;
                    }
                    upper.clear();
                    lower.clear();
                }
            }
        }
    }

    newAirspaces[u"A"_s] = QVariant::fromValue(airspacePolygonsA);
    newAirspaces[u"CTR"_s] = QVariant::fromValue(airspacePolygonsCTR);
    newAirspaces[u"R"_s] = QVariant::fromValue(airspacePolygonsR);
    newAirspaces[u"RMZ"_s] = QVariant::fromValue(airspacePolygonsRMZ);
    newAirspaces[u"NRA"_s] = QVariant::fromValue(airspacePolygonsNRA);
    newAirspaces[u"PJE"_s] = QVariant::fromValue(airspacePolygonsPJE);
    newAirspaces[u"TMZ"_s] = QVariant::fromValue(airspacePolygonsTMZ);

    m_airspaces = newAirspaces;
    qWarning() << "SideviewQuickItem full" << m_elapsedTimer.elapsed();
}
