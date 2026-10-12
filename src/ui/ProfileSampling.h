/***************************************************************************
 *   Copyright (C) 2023-2025 by Stefan Kebekus                             *
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

#pragma once

#include <QGeoCoordinate>
#include <QPolygonF>
#include <QVariantMap>

#include <functional>

#include "units/Distance.h"
#include "units/Pressure.h"


/*! \brief Shared computations for vertical profiles of terrain and airspaces
 *
 * Used by SideviewQuickItem (profile along the current track) and
 * RouteProfileQuickItem (profile along the flight route). Both sample the
 * profile at a list of x-coordinates, each associated with a geographic
 * position.
 */
namespace Ui::ProfileSampling
{

/*! \brief One column of a vertical profile */
struct Sample
{
    /*! \brief x-coordinate in pixels */
    double x {0.0};

    /*! \brief Geographic position shown at x */
    QGeoCoordinate coordinate;

    /*! \brief Terrain elevation at coordinate */
    Units::Distance terrain;
};

/*! \brief Maps an altitude to a y-coordinate in pixels */
using AltitudeToY = std::function<double(Units::Distance)>;

/*! \brief Look up the terrain elevation of all samples
 *
 *  Samples without terrain data get an elevation of 0 m.
 *
 *  @param samples Samples whose terrain member is set
 *
 *  @returns False if terrain data was missing for at least one sample
 */
bool fillTerrain(QList<Sample>& samples);

/*! \brief Polygon of the terrain silhouette, closed below the item
 *
 *  @param samples Samples with terrain set
 *
 *  @param altToY Altitude-to-pixel mapping
 *
 *  @param width Width of the item in pixels
 *
 *  @param height Height of the item in pixels
 *
 *  @returns Polygon for the "terrain" property of the profile items
 */
QPolygonF terrainPolygon(const QList<Sample>& samples, const AltitudeToY& altToY, double width, double height);

/*! \brief Map with empty polygon lists for all airspace drawing styles
 *
 *  @returns Map with the keys described in airspacePolygons()
 */
QVariantMap emptyAirspacePolygons();

/*! \brief Vertical cuts through all airspaces along the samples
 *
 *  @param samples Samples with terrain set
 *
 *  @param altToY Altitude-to-pixel mapping
 *
 *  @param QNH QNH used to estimate airspace bounds given above MSL
 *
 *  @param geometricAltitude Geometric altitude of the own aircraft, see
 *  Airspace::estimatedLowerBoundMSL()
 *
 *  @param barometricAltitude Barometric altitude of the own aircraft, see
 *  Airspace::estimatedLowerBoundMSL()
 *
 *  @returns Polygons sorted by drawing style. Keys: "A" (CAT A, B, C, D),
 *  "CTR", "NRA", "PJE" (PJE, SUA), "R" (R, P, DNG), "RMZ" (ATZ, RMZ, TIA, TIZ),
 *  "TMZ".
 */
QVariantMap airspacePolygons(const QList<Sample>& samples, const AltitudeToY& altToY, Units::Pressure QNH, Units::Distance geometricAltitude, Units::Distance barometricAltitude);

} // namespace Ui::ProfileSampling
