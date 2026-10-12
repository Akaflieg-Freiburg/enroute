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
#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "navigation/Navigator.h"
#include "ui/ProfileSampling.h"
#include "ui/RouteProfileQuickItem.h"
#include "weather/WeatherDataProvider.h"

using namespace Qt::Literals::StringLiterals;


Ui::RouteProfileQuickItem::RouteProfileQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(0);
    connect(&m_updateTimer, &QTimer::timeout, this, &Ui::RouteProfileQuickItem::updateProperties);

    connect(GlobalObject::navigator()->flightRoute(), &Navigation::FlightRoute::waypointsChanged, this, &Ui::RouteProfileQuickItem::scheduleUpdate);
    connect(GlobalObject::globalSettings(), &GlobalSettings::airspaceAltitudeLimitChanged, this, &Ui::RouteProfileQuickItem::scheduleUpdate);
    connect(GlobalObject::weatherDataProvider(), &Weather::WeatherDataProvider::QNHInfoChanged, this, &Ui::RouteProfileQuickItem::scheduleUpdate);
    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Ui::RouteProfileQuickItem::scheduleUpdate);

    m_notifiers.push_back(bindableWidth().addNotifier([this]() {scheduleUpdate();}));
    m_notifiers.push_back(bindableHeight().addNotifier([this]() {scheduleUpdate();}));
    m_notifiers.push_back(GlobalObject::geoMapProvider()->bindableAirspaces().addNotifier([this]() {scheduleUpdate();}));
    m_notifiers.push_back(m_minimumTickSpacing.addNotifier([this]() {scheduleUpdate();}));

    updateProperties();
}


void Ui::RouteProfileQuickItem::updateProperties()
{
    const QScopedPropertyUpdateGroup updateLock;
    m_altitudeTicks = QVariantList();
    m_airspaces = Ui::ProfileSampling::emptyAirspacePolygons();
    m_error = QString();
    m_terrain = QPolygonF();
    m_waypoints = QVariantList();

    // If the item is really small, save CPU cycles by doing nothing
    if ((width() < 5.0) || (height() < 5.0))
    {
        return;
    }

    const auto legs = GlobalObject::navigator()->flightRoute()->legs();
    if (legs.isEmpty())
    {
        m_error = tr("Add at least two waypoints to the flight route to see its profile.");
        return;
    }

    // Distance from the departure point to the start of each leg, plus the total
    QList<double> legStart_m;
    legStart_m.reserve(legs.size()+1);
    double total_m = 0.0;
    for (const auto& leg : legs)
    {
        legStart_m << total_m;
        total_m += leg.distance().toM();
    }
    legStart_m << total_m;
    if (!qIsFinite(total_m) || (total_m <= 0.0))
    {
        m_error = tr("Unable to show the route profile: The flight route has no length.");
        return;
    }

    //
    // Sample the profile every few pixels along the route
    //
    const double step = 4.0;
    QList<Ui::ProfileSampling::Sample> samples;
    samples.reserve(qCeil(width()/step)+1);
    qsizetype legIndex = 0;
    for (double x = 0.0; ; x = qMin(x+step, width()))
    {
        auto distance_m = total_m*x/width();
        while ((legIndex < legs.size()-1) && (distance_m > legStart_m[legIndex+1]))
        {
            legIndex++;
        }
        const auto start = legs[legIndex].startPoint().coordinate();
        const auto end = legs[legIndex].endPoint().coordinate();
        auto coordinate = start.atDistanceAndAzimuth(distance_m - legStart_m[legIndex], start.azimuthTo(end));
        samples << Ui::ProfileSampling::Sample{x, coordinate, {}};
        if (x >= width())
        {
            break;
        }
    }
    if (!Ui::ProfileSampling::fillTerrain(samples))
    {
        m_error = tr("Incomplete terrain data. Please install the relevant terrain maps.");
    }

    //
    // Vertical range: from the lowest terrain to the airspace altitude limit, or FL100
    //
    auto minAlt = samples.constFirst().terrain;
    for (const auto& sample : std::as_const(samples))
    {
        minAlt = qMin(minAlt, sample.terrain);
    }
    auto maxAlt = GlobalObject::globalSettings()->airspaceAltitudeLimit();
    if (!maxAlt.isFinite())
    {
        maxAlt = Units::Distance::fromFT(10000.0);
    }
    if (maxAlt <= minAlt)
    {
        maxAlt = minAlt + Units::Distance::fromFT(1000.0);
    }
    auto altToY = [this, minAlt, maxAlt](Units::Distance alt)
    {
        return height()*(maxAlt - alt)/(maxAlt - minAlt);
    };

    m_terrain = Ui::ProfileSampling::terrainPolygon(samples, altToY, width(), height());

    //
    // Vertical scale: the smallest round step that keeps the ticks apart
    //
    const auto aircraft = GlobalObject::navigator()->aircraft();
    const bool metric = (aircraft.verticalDistanceUnit() == Navigation::Aircraft::Meters);
    auto toUnit = [metric](Units::Distance d) { return metric ? d.toM() : d.toFeet(); };
    auto fromUnit = [metric](double v) { return metric ? Units::Distance::fromM(v) : Units::Distance::fromFT(v); };
    const QList<double> steps = metric ? QList<double>{50, 100, 200, 500, 1000, 2000, 5000} : QList<double>{100, 200, 500, 1000, 2000, 5000, 10000};
    const double pixelPerUnit = height()/(toUnit(maxAlt) - toUnit(minAlt));
    double tickStep = steps.constLast();
    for (auto step : steps)
    {
        if (step*pixelPerUnit >= m_minimumTickSpacing.value())
        {
            tickStep = step;
            break;
        }
    }
    QVariantList newTicks;
    for (double v = qCeil(toUnit(minAlt)/tickStep)*tickStep; v <= toUnit(maxAlt); v += tickStep)
    {
        QVariantMap tick;
        tick[u"y"_s] = altToY(fromUnit(v));
        tick[u"label"_s] = aircraft.verticalDistanceToString(fromUnit(v));
        newTicks << tick;
    }
    m_altitudeTicks = newTicks;

    // There is no aircraft position for planning: use the current QNH, or the
    // standard atmosphere if none is known, and treat flight levels as altitudes.
    auto QNH = GlobalObject::weatherDataProvider()->QNH();
    if (!QNH.isFinite())
    {
        QNH = Units::Pressure::fromHPa(1013.25);
    }
    m_airspaces = Ui::ProfileSampling::airspacePolygons(samples, altToY, QNH, Units::Distance::fromM(0.0), Units::Distance::fromM(0.0));

    //
    // Waypoints
    //
    QVariantList newWaypoints;
    for (qsizetype i = 0; i <= legs.size(); i++)
    {
        const auto waypoint = (i < legs.size()) ? legs[i].startPoint() : legs.constLast().endPoint();
        QVariantMap entry;
        entry[u"x"_s] = width()*legStart_m[i]/total_m;
        entry[u"label"_s] = waypoint.ICAOCode();
        newWaypoints << entry;
    }
    m_waypoints = newWaypoints;
}
