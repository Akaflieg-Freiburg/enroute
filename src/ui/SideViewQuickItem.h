/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include <QQmlEngine>
#include <QtQuick/QQuickPaintedItem>
#include <set>

#include "GlobalObject.h"
#include "PositionInfo.h"
#include "GeoMapProvider.h"

namespace Ui {

/*! \brief QML Class implementing a SideView
 */

class SideViewQuickItem : public QQuickPaintedItem
{
  Q_OBJECT
  QML_NAMED_ELEMENT(SideView)

public:
  /*! \brief Standard constructor
   *
   *  @param parent The standard QObject parent pointer
   */
  explicit SideViewQuickItem(QQuickItem *parent = nullptr);

  /*! \brief Re-implemented from QQuickPaintedItem to implement painting
   *
   *  @param painter Pointer to the QPainter used for painting
   */
  void paint(QPainter *painter) override;

private:
  struct Airspace2D { //TODO: Rename later
      GeoMaps::Airspace airspace;
      int firstStep;
      int lastStep;
      QPolygon polygon;
      bool operator<(const Airspace2D &other) const {
          return GeoMaps::qHash(airspace) < GeoMaps::qHash(other.airspace);
      }
  };

  struct MergedAirspace2D {
      std::vector<Airspace2D> airspaces;
      QString category;

  };

  void drawNoTrackAvailable(QPainter *painter);
  void drawSky(QPainter *painter);
  std::vector<int> getElevations(const Positioning::PositionInfo &info, double track, float steps, float stepSizeInMeter, float stepOffset);
  int getHighestElevation(std::vector<int> &elevations, const Positioning::PositionInfo &info, float defaultUpperLimit);
  std::vector<Airspace2D> get2dAirspaces(double track, float steps, float stepsBackwards, float stepSizeInMeter);
  std::vector<MergedAirspace2D> mergedAirspaces2D(std::vector<Airspace2D> airspaces, std::vector<int> &elevations, float steps, int highestElevation);
  std::vector<MergedAirspace2D> mergeAirspaces(std::vector<Airspace2D> mergedAirspaces);
  void drawAirspacesOutline(QPainter *painter, const MergedAirspace2D &mergedAirspaces2D);
  void drawAirspacesArea(QPainter *painter, const MergedAirspace2D &mergedAirspaces2D);
  void drawAirspacesLabel(QPainter *painter, const MergedAirspace2D &mergedAirspaces2D);
  void drawTerrain(QPainter *painter, const std::vector<int> &elevations, int highestElevation);
  QStringList airspaceSortedCategories();
  void drawAircraft(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation, float steps, float stepsOffset);
  void drawCurrentHorizontalPosition(QPainter *painter, const Positioning::PositionInfo &info, float steps, float stepsBackwards);
  void drawFlightPath(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation, float steps, float stepOffset);
  int yCoordinate(int altitude, int maxHeight, int objectHeight);
  int widgetHeight();
  int widgetWidth();
  Units::Distance pressureAltitude();
  QPointF getPolygonCentroid(const QPolygonF &polygon);
  Q_DISABLE_COPY_MOVE(SideViewQuickItem)

};

} // namespace Ui
