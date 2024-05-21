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
  struct MergedAirspace {
      GeoMaps::Airspace airspace;
      int firstStep;
      int lastStep;
  };

  void drawNoTrackAvailable(QPainter *painter);
  void drawSky(QPainter *painter, int widgetHeight, int widgetWidth);
  std::vector<int> getElevations(const Positioning::PositionInfo &info, double track, float steps, float stepSizeInMeter, const GeoMaps::GeoMapProvider *geoMapProvider);
  int getHighestElevation(std::vector<int> &elevations, const Positioning::PositionInfo &info, float defaultUpperLimit);
  std::vector<MergedAirspace> getMergedAirspaces(const Positioning::PositionInfo &info, double track, float steps, float stepSizeInMeter, const GeoMaps::GeoMapProvider *geoMapProvider);
  void drawAirspaces(QPainter *painter, const std::vector<MergedAirspace> &mergedAirspaces, int widgetWidth, int widgetHeight, int highestElevation, float steps);
  void drawTerrain(QPainter *painter, const std::vector<int> &elevations, int highestElevation, int widgetWidth, int widgetHeight, float steps);
  void drawAircraft(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation);
  void drawFlightPath(QPainter *painter, const Positioning::PositionInfo &info, int widgetWidth, int highestElevation);
  int yCoordinate(int altitude, int maxHeight, int objectHeight);
  Q_DISABLE_COPY_MOVE(SideViewQuickItem)

};

} // namespace Ui
