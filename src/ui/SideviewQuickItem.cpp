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
#include "navigation/Navigator.h"
#include "weather/WeatherDataProvider.h"
#include "weather/WindFieldProvider.h"

#include <QtMath>

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
    notifiers.push_back(m_renderWidth.addNotifier([this]() { emit renderWidthChanged(); updateProperties(); }));
    notifiers.push_back(m_scaleMaxAltFt.addNotifier([this]() { emit scaleRangeChanged(); }));
    notifiers.push_back(m_scaleMinAltFt.addNotifier([this]() { emit scaleRangeChanged(); }));
    notifiers.push_back(m_scaleTotalDistKm.addNotifier([this]() { emit scaleRangeChanged(); }));

    // React to route changes when in Route mode
    notifiers.push_back(GlobalObject::navigator()->flightRoute()->bindableGeoPath().addNotifier([this]() {
        if (m_mode == Mode::Route) { updateProperties(); }
    }));
    connect(GlobalObject::navigator()->flightRoute(), &Navigation::FlightRoute::plannedAltitudesChanged, this, [this]() {
        if (m_mode == Mode::Route) { updateProperties(); }
    });
    connect(Weather::WindFieldProvider::instance(), &Weather::WindFieldProvider::dataChanged, this, [this]() {
        if (m_mode == Mode::Route) { updateProperties(); }
    });

    updateProperties();
}

void Ui::SideviewQuickItem::setMode(Mode newMode)
{
    if (m_mode == newMode) { return; }
    m_mode = newMode;
    emit modeChanged();
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

    if (m_mode == Mode::Route)
    {
        updatePropertiesRoute();
    }
    else
    {
        updatePropertiesTrack();
    }
}


// ---------------------------------------------------------------------------
// Shared helper: build airspace polygons along a sampled geo-path
// ---------------------------------------------------------------------------
QVariantMap Ui::SideviewQuickItem::buildAirspacePolygons(
    const QList<int>& xCoordinates,
    const QList<QGeoCoordinate>& geoCoordinates,
    const QList<Units::Distance>& elevations,
    Units::Distance ownshipGeometricAltitude,
    Units::Distance ownshipPressureAltitude,
    std::function<double(Units::Distance)> altToY)
{
    const QVector<QPolygonF> empty;
    QVariantMap result;
    result[u"A"_s]   = QVariant::fromValue(empty);
    result[u"CTR"_s] = QVariant::fromValue(empty);
    result[u"R"_s]   = QVariant::fromValue(empty);
    result[u"RMZ"_s] = QVariant::fromValue(empty);
    result[u"NRA"_s] = QVariant::fromValue(empty);
    result[u"PJE"_s] = QVariant::fromValue(empty);
    result[u"TMZ"_s] = QVariant::fromValue(empty);

    auto QNH = GlobalObject::weatherDataProvider()->QNH();
    if (!QNH.isFinite()) { return result; }

    QVector<QPolygonF> polyA, polyCTR, polyR, polyRMZ, polyNRA, polyPJE, polyTMZ;

    auto airspaces = GlobalObject::geoMapProvider()->airspaces();
    QGeoRectangle const viewBBox(QList({geoCoordinates.constFirst(), geoCoordinates.constLast()}));

    for (const auto& airspace : std::as_const(airspaces))
    {
        if (!airspaceCategories.contains(airspace.CAT())) { continue; }
        if (!airspace.polygon().boundingGeoRectangle().intersects(viewBBox)) { continue; }

        QList<QPointF> upper, lower;
        auto airspacePolygon = airspace.polygon();

        for (int i = 0; i < xCoordinates.size(); i++)
        {
            auto x = xCoordinates[i];
            const auto& gc = geoCoordinates[i];
            if ((i != xCoordinates.size()-1) && airspacePolygon.contains(gc))
            {
                auto u = airspace.estimatedUpperBoundMSL(elevations[i], QNH, ownshipGeometricAltitude, ownshipPressureAltitude);
                auto l = airspace.estimatedLowerBoundMSL(elevations[i], QNH, ownshipGeometricAltitude, ownshipPressureAltitude);
                if (l < u)
                {
                    upper << QPointF(x, altToY(u));
                    lower << QPointF(x, altToY(l));
                    continue;
                }
            }

            if (!upper.isEmpty())
            {
                std::reverse(lower.begin(), lower.end());
                QPolygonF poly(upper + lower);
                poly << upper[0];

                auto cat = airspace.CAT();
                if (cat == u"A"_s || cat == u"B"_s || cat == u"C"_s || cat == u"D"_s) { polyA   << poly; }
                if (cat == u"CTR"_s)                                                   { polyCTR << poly; }
                if (cat == u"R"_s  || cat == u"P"_s  || cat == u"DNG"_s)              { polyR   << poly; }
                if (cat == u"ATZ"_s|| cat == u"RMZ"_s|| cat == u"TIA"_s|| cat == u"TIZ"_s) { polyRMZ << poly; }
                if (cat == u"NRA"_s)                                                   { polyNRA << poly; }
                if (cat == u"PJE"_s|| cat == u"SUA"_s)                                { polyPJE << poly; }
                if (cat == u"TMZ"_s)                                                   { polyTMZ << poly; }

                upper.clear();
                lower.clear();
            }
        }
    }

    result[u"A"_s]   = QVariant::fromValue(polyA);
    result[u"CTR"_s] = QVariant::fromValue(polyCTR);
    result[u"R"_s]   = QVariant::fromValue(polyR);
    result[u"RMZ"_s] = QVariant::fromValue(polyRMZ);
    result[u"NRA"_s] = QVariant::fromValue(polyNRA);
    result[u"PJE"_s] = QVariant::fromValue(polyPJE);
    result[u"TMZ"_s] = QVariant::fromValue(polyTMZ);
    return result;
}


// ---------------------------------------------------------------------------
// Mode::Track  — original logic, unchanged except extracted to own method
// ---------------------------------------------------------------------------
void Ui::SideviewQuickItem::updatePropertiesTrack()
{
    const QScopedPropertyUpdateGroup updateLock;
    //
    // Set all properties to default values. We update those depending on the data we have available.
    //
    m_fiveMinuteBar = {0, 0};
    m_ownshipPosition = {-100, -100};
    m_track = QString();
    m_error = QString();
    m_terrain = QPolygonF();
    m_plannedProfile = QPolygonF();
    m_plannedProfilePoints = QVariantList();
    m_windProfile = QVariantList();

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

    const double renderW = rw();
    //
    // Compute array of x-coordinates in pixels.
    //
    const int step = 4.0;
    QList<int> xCoordinates;
    xCoordinates.reserve( qRound((renderW+20)/step) );
    for(int x = -10; x <= renderW+10; x += step)
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
        auto dist = 10000.0*(x-0.2*renderW)/(m_pixelPer10km.value());
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

    m_scaleMinAltFt  = sideview_minAlt.toFeet();
    m_scaleMaxAltFt  = sideview_maxAlt.toFeet();
    m_scaleTotalDistKm = 0.0;

    //
    // Compute graphics data
    //

    // Ownship position
    m_ownshipPosition = {renderW*0.2, altToYCoordinate(ownshipGeometricAltitude)};

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
    polygon << QPointF(renderW, height()+2000) << QPointF(-20, height()+2000);
    m_terrain = polygon;

    m_airspaces = buildAirspacePolygons(xCoordinates, geoCoordinates, elevations,
                                         ownshipGeometricAltitude, ownshipPressureAltitude, altToYCoordinate);
}


// ---------------------------------------------------------------------------
// Mode::Route  — terrain/airspaces along the planned flight route
// ---------------------------------------------------------------------------
void Ui::SideviewQuickItem::updatePropertiesRoute()
{
    const QScopedPropertyUpdateGroup updateLock;
    m_fiveMinuteBar   = {0, 0};
    m_ownshipPosition = {-100, -100};
    m_track           = QString();
    m_error           = QString();
    m_terrain         = QPolygonF();
    m_plannedProfile  = QPolygonF();
    m_plannedProfilePoints = QVariantList();
    m_windProfile     = QVariantList();

    const QVector<QPolygonF> empty;
    QVariantMap newAirspaces;
    newAirspaces[u"A"_s]   = QVariant::fromValue(empty);
    newAirspaces[u"CTR"_s] = QVariant::fromValue(empty);
    newAirspaces[u"R"_s]   = QVariant::fromValue(empty);
    newAirspaces[u"RMZ"_s] = QVariant::fromValue(empty);
    newAirspaces[u"NRA"_s] = QVariant::fromValue(empty);
    newAirspaces[u"PJE"_s] = QVariant::fromValue(empty);
    newAirspaces[u"TMZ"_s] = QVariant::fromValue(empty);
    m_airspaces = newAirspaces;

    if (height() < 5.0) { return; }

    // -----------------------------------------------------------------------
    // 1. Get the planned route
    // -----------------------------------------------------------------------
    auto geoPath = GlobalObject::navigator()->flightRoute()->geoPath();
    if (geoPath.size() < 2)
    {
        m_error = tr("Unable to show side view: No flight route. Please plan a route first.");
        return;
    }

    // -----------------------------------------------------------------------
    // 2. Build cumulative distance table along the route polyline
    //    cumulativeDist[i] = distance from geoPath[0] to geoPath[i], in metres
    // -----------------------------------------------------------------------
    QList<double> cumulativeDist;
    cumulativeDist.reserve(geoPath.size());
    cumulativeDist << 0.0;
    for (qsizetype i = 1; i < geoPath.size(); i++)
    {
        cumulativeDist << cumulativeDist.last() + geoPath[i-1].distanceTo(geoPath[i]);
    }
    const double totalRouteLen = cumulativeDist.last();
    if (totalRouteLen < 1.0)
    {
        m_error = tr("Unable to show side view: Flight route is too short.");
        return;
    }

    // -----------------------------------------------------------------------
    // 3. Helper: given a distance along the route, return the QGeoCoordinate
    //    by linear interpolation between the nearest waypoints.
    // -----------------------------------------------------------------------
    auto coordAtDist = [&](double distM) -> QGeoCoordinate
    {
        distM = qBound(0.0, distM, totalRouteLen);
        // Find the leg that contains distM
        qsizetype seg = 0;
        for (qsizetype i = 1; i < cumulativeDist.size(); i++)
        {
            if (cumulativeDist[i] >= distM)
            {
                seg = i - 1;
                break;
            }
            seg = i - 1; // last segment
        }
        double segLen = cumulativeDist[seg+1] - cumulativeDist[seg];
        if (segLen < 0.01) { return geoPath[seg]; }
        double t = (distM - cumulativeDist[seg]) / segLen;
        double az = geoPath[seg].azimuthTo(geoPath[seg+1]);
        return geoPath[seg].atDistanceAndAzimuth(t * segLen, az);
    };

    // -----------------------------------------------------------------------
    // 4. Sample the route at pixel resolution
    //    Each pixel column x maps to a distance proportional along the route.
    // -----------------------------------------------------------------------
    const double renderW = rw();
    const int step = 4;
    QList<int> xCoordinates;
    xCoordinates.reserve(qRound((renderW+20)/step));
    for (int x = -10; x <= renderW+10; x += step) { xCoordinates << x; }

    QList<QGeoCoordinate> geoCoordinates;
    geoCoordinates.reserve(xCoordinates.size());
    for (auto x : std::as_const(xCoordinates))
    {
        double fraction = (double)x / renderW;
        geoCoordinates << coordAtDist(fraction * totalRouteLen);
    }

    // -----------------------------------------------------------------------
    // 5. Sample terrain elevations
    // -----------------------------------------------------------------------
    QList<Units::Distance> elevations;
    elevations.reserve(geoCoordinates.size());
    Units::Distance minElevation = Units::Distance::fromM(0.0);
    Units::Distance maxElevation = Units::Distance::fromM(0.0);
    bool firstElev = true;
    for (const auto& gc : std::as_const(geoCoordinates))
    {
        auto elev = GlobalObject::geoMapProvider()->terrainElevationAMSL(gc);
        if (!elev.isFinite())
        {
            elev = Units::Distance::fromM(0.0);
            m_error = tr("Incomplete terrain data. Please install the relevant terrain maps.");
        }
        elevations << elev;
        if (firstElev) { minElevation = maxElevation = elev; firstElev = false; }
        else { minElevation = qMin(minElevation, elev); maxElevation = qMax(maxElevation, elev); }
    }

    // -----------------------------------------------------------------------
    // 6. Determine vertical extent of the view
    //    Centre vertically around the terrain + a comfortable margin above.
    // -----------------------------------------------------------------------
    Units::Distance sideview_minAlt = minElevation - Units::Distance::fromFT(200);
    Units::Distance sideview_maxAlt = maxElevation + Units::Distance::fromFT(4000);

    // -----------------------------------------------------------------------
    // 6b. Resolve the planned altitude at each waypoint (stored value, else
    //     aircraft cruise altitude, else terrain + 1000ft) and expand the
    //     vertical extent so the whole profile is visible.
    // -----------------------------------------------------------------------
    auto* flightRoute = GlobalObject::navigator()->flightRoute();
    const auto routeWaypoints = flightRoute->waypoints();
    const Units::Distance defaultCruiseAlt = GlobalObject::navigator()->aircraft().cruiseAltitude();
    const Units::Distance fallbackAlt = maxElevation + Units::Distance::fromFT(1000);

    QList<Units::Distance> plannedAlts;
    plannedAlts.reserve(routeWaypoints.size());
    for (int i = 0; i < routeWaypoints.size(); i++)
    {
        double storedM = flightRoute->plannedAltitude(i);
        Units::Distance alt = !qIsNaN(storedM) ? Units::Distance::fromM(storedM)
                            : defaultCruiseAlt.isFinite() ? defaultCruiseAlt
                            : fallbackAlt;
        plannedAlts << alt;
        sideview_maxAlt = qMax(sideview_maxAlt, alt + Units::Distance::fromFT(200));
        sideview_minAlt = qMin(sideview_minAlt, alt - Units::Distance::fromFT(200));
    }

    auto altToY = [this, sideview_minAlt, sideview_maxAlt](Units::Distance alt) {
        return ((double)height()) * (sideview_maxAlt - alt) / (sideview_maxAlt - sideview_minAlt);
    };

    m_scaleMinAltFt    = sideview_minAlt.toFeet();
    m_scaleMaxAltFt    = sideview_maxAlt.toFeet();
    m_scaleTotalDistKm = totalRouteLen / 1000.0;

    // Build the planned-altitude polyline: one point per waypoint, at the
    // waypoint's x position along the route.
    QPolygonF profile;
    QVariantList profilePoints;
    profile.reserve(plannedAlts.size());
    profilePoints.reserve(plannedAlts.size());
    for (qsizetype i = 0; (i < plannedAlts.size()) && (i < cumulativeDist.size()); i++)
    {
        double x = (cumulativeDist[i] / totalRouteLen) * renderW;
        QPointF pt(x, altToY(plannedAlts[i]));
        profile << pt;
        profilePoints << pt;
    }
    m_plannedProfile = profile;
    m_plannedProfilePoints = profilePoints;

    // -----------------------------------------------------------------------
    // 6c. Wind barbs projected onto the route profile: sample the wind field
    //     along the route at the planned altitude and project onto the track.
    // -----------------------------------------------------------------------
    {
        const auto* wfp = Weather::WindFieldProvider::instance();
        const bool haveField = wfp->isUsable();
        const Weather::Wind manualWind = GlobalObject::navigator()->wind();
        const bool haveManual = manualWind.speed().isFinite() && manualWind.directionFrom().isFinite();

        // Planned altitude (metres) at a distance along the route
        auto plannedAltAtDist = [&](double distM) -> Units::Distance {
            qsizetype seg = 0;
            for (qsizetype i = 1; i < cumulativeDist.size(); i++)
            {
                if (cumulativeDist[i] >= distM) { seg = i - 1; break; }
                seg = i - 1;
            }
            if (seg + 1 >= plannedAlts.size()) { return plannedAlts.isEmpty() ? Units::Distance() : plannedAlts.last(); }
            const double segLen = cumulativeDist[seg+1] - cumulativeDist[seg];
            const double t = (segLen > 0.01) ? (distM - cumulativeDist[seg]) / segLen : 0.0;
            return plannedAlts[seg] + (plannedAlts[seg+1] - plannedAlts[seg]) * qBound(0.0, t, 1.0);
        };

        QVariantList windPts;
        if ((haveField || haveManual) && (totalRouteLen > 1.0))
        {
            // One sample roughly every 80 px, clamped to a sane count
            const int nSamples = qBound(3, int(renderW / 80.0), 16);
            const QDateTime now = QDateTime::currentDateTimeUtc();
            for (int s = 0; s < nSamples; s++)
            {
                const double frac = (s + 0.5) / nSamples;
                const double distM = frac * totalRouteLen;
                const auto coord = coordAtDist(distM);
                const auto alt = plannedAltAtDist(distM);

                Weather::Wind w = haveField
                    ? wfp->windAt(coord.latitude(), coord.longitude(), alt.toFeet(), now)
                    : Weather::Wind();
                if (!w.speed().isFinite() || !w.directionFrom().isFinite())
                {
                    w = manualWind;
                }
                if (!w.speed().isFinite() || !w.directionFrom().isFinite())
                {
                    continue;
                }

                // Track azimuth at this distance
                qsizetype seg = 0;
                for (qsizetype i = 1; i < cumulativeDist.size(); i++)
                {
                    if (cumulativeDist[i] >= distM) { seg = i - 1; break; }
                    seg = i - 1;
                }
                const double trackDeg = geoPath[seg].azimuthTo(geoPath[qMin(seg+1, geoPath.size()-1)]);

                const double spdKn = w.speed().toKN();
                const double dirDeg = w.directionFrom().toDEG();
                // Headwind component (+) = wind blowing from ahead along the track
                const double alongKn = spdKn * qCos(qDegreesToRadians(dirDeg - trackDeg));

                QVariantMap m;
                m[QStringLiteral("x")]          = frac * renderW;
                m[QStringLiteral("y")]          = altToY(alt);
                m[QStringLiteral("speedKn")]    = spdKn;
                m[QStringLiteral("dirFromDeg")] = dirDeg;
                m[QStringLiteral("alongKn")]    = alongKn;
                windPts << m;
            }
        }
        m_windProfile = windPts;
    }

    // -----------------------------------------------------------------------
    // 7. Terrain polygon
    // -----------------------------------------------------------------------
    QPolygonF polygon;
    polygon.reserve(elevations.size() + 4);
    for (qsizetype i = 0; i < elevations.size(); i++)
    {
        polygon << QPointF(xCoordinates[i], altToY(elevations[i]));
    }
    polygon << QPointF(renderW, height()+2000) << QPointF(-10, height()+2000);
    m_terrain = polygon;

    // -----------------------------------------------------------------------
    // 8. Ownship position — project current GPS fix onto the route polyline
    //    by finding the closest point along all legs.
    // -----------------------------------------------------------------------
    auto ownshipCoordinate = Positioning::PositionProvider::lastValidCoordinate();
    auto ownshipGeometricAltitude = Units::Distance::fromM(0.0);
    auto ownshipPressureAltitude  = Units::Distance::fromM(0.0);

    if (ownshipCoordinate.isValid())
    {
        // Find the leg with minimum perpendicular distance to ownship
        double bestDistM = std::numeric_limits<double>::max();
        double bestAlongRouteM = 0.0;

        for (qsizetype i = 0; i+1 < geoPath.size(); i++)
        {
            // Project ownship onto this leg segment
            double legLen = geoPath[i].distanceTo(geoPath[i+1]);
            if (legLen < 1.0) { continue; }

            double az      = geoPath[i].azimuthTo(geoPath[i+1]);
            double azOwn   = geoPath[i].azimuthTo(ownshipCoordinate);
            double distFromStart = geoPath[i].distanceTo(ownshipCoordinate);
            double angleDiff = (azOwn - az) * M_PI / 180.0;
            double along = distFromStart * std::cos(angleDiff);
            along = qBound(0.0, along, legLen);

            QGeoCoordinate projected = geoPath[i].atDistanceAndAzimuth(along, az);
            double perp = projected.distanceTo(ownshipCoordinate);

            if (perp < bestDistM)
            {
                bestDistM = perp;
                bestAlongRouteM = cumulativeDist[i] + along;
            }
        }

        double ownX = (bestAlongRouteM / totalRouteLen) * renderW;

        ownshipGeometricAltitude = Units::Distance::fromM(ownshipCoordinate.altitude());
        if (!ownshipGeometricAltitude.isFinite()) { ownshipGeometricAltitude = minElevation; }

        ownshipPressureAltitude = GlobalObject::positionProvider()->pressureAltitude();
        if (!ownshipPressureAltitude.isFinite())
        {
            ownshipPressureAltitude = m_baroCache->estimatedPressureAltitude(ownshipGeometricAltitude);
        }
        if (!ownshipPressureAltitude.isFinite())
        {
            ownshipPressureAltitude = ownshipGeometricAltitude;
            m_error = tr("Unable to compute sufficiently precise vertical airspace boundaries because barometric altitude information is not available. <a href='xx'>Click here</a> for more information.");
        }

        // Clamp to terrain
        auto ownTerrainElev = GlobalObject::geoMapProvider()->terrainElevationAMSL(ownshipCoordinate);
        if (ownTerrainElev.isFinite() && ownshipGeometricAltitude < ownTerrainElev)
        {
            ownshipGeometricAltitude = ownTerrainElev;
        }

        double ownY = altToY(ownshipGeometricAltitude);
        m_ownshipPosition = {ownX, ownY};

        // 5-minute bar: use current speed/vspeed projected along the route
        auto posInfo = GlobalObject::positionProvider()->positionInfo();
        if (posInfo.isValid())
        {
            auto hSpeed = posInfo.groundSpeed();
            auto vSpeed = posInfo.verticalSpeed();
            if (hSpeed.isFinite() && hSpeed >= Units::Speed::fromKN(10) && vSpeed.isFinite())
            {
                double dxPixels = (hSpeed.toMPS() * 5.0 * 60.0 / totalRouteLen) * renderW;
                double dyPixels = -height() * vSpeed.toFPM() * 5.0 / (sideview_maxAlt - sideview_minAlt).toFeet();
                m_fiveMinuteBar = {dxPixels, dyPixels};
            }
        }
    }

    // -----------------------------------------------------------------------
    // 9. Airspace polygons
    // -----------------------------------------------------------------------
    if (!GlobalObject::weatherDataProvider()->QNH().isFinite())
    {
        m_error = tr("Unable to compute sufficiently precise vertical airspace boundaries because the QNH is not available. Please wait while QNH information is downloaded from the internet.");
        // Still show terrain — don't return early
    }
    else
    {
        m_airspaces = buildAirspacePolygons(xCoordinates, geoCoordinates, elevations,
                                             ownshipGeometricAltitude, ownshipPressureAltitude, altToY);
    }
}
