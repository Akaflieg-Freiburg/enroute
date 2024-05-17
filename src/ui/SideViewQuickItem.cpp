/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QPainter>

#include "GlobalObject.h"
#include "PositionProvider.h"
#include "PositionInfo.h"
#include "GeoMapProvider.h"
#include "SideViewQuickItem.h"
#include "QRandomGenerator"


Ui::SideViewQuickItem::SideViewQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{

    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::positionInfoChanged, this, &QQuickItem::update);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}


void Ui::SideViewQuickItem::paint(QPainter *painter)
{

    QElapsedTimer timer;
    timer.start();


    int widgetHeight = static_cast<int>(height());
    int widgetWidth = static_cast<int>(width());

    auto info = GlobalObject::positionProvider()->positionInfo();
    auto geoMapProvider = GlobalObject::geoMapProvider();


    //Paint the Sky
    painter->fillRect(0, 0, static_cast<int>(width()), widgetHeight, QColor("lightblue"));


    //Paint the Terrain - Initialize
    float steps = 50;
    float stepSizeInMeter = 500; //TODO: Make Dynamic
    int defaultUpperLimit = 3000;
    painter->setBrush(QColor("brown"));

    auto track = info.trueTrack().toDEG(); //TODO: Dont use that function
    if (qIsNaN(track)) {
        track = QRandomGenerator::global()->bounded(360);; //TODO: What to do when stationary?
    }
    qWarning() << "Track (DEG): " << track;

    //Paint the Terrain - Get Elevations and find the highest
    std::vector<int> elevations;
    for (int i = 0; i <= steps; i++) {
        auto position = info.coordinate().atDistanceAndAzimuth(i * stepSizeInMeter, track, 0);
        auto elevation = geoMapProvider->terrainElevationAMSL(position).toFeet();
        elevations.push_back(elevation);
    }
    int highestElevation = elevations[0];
    for (int elevation : elevations) {
        if (elevation > highestElevation) {
            highestElevation = elevation;
        }
    }
    elevations.push_back(info.trueAltitudeAMSL().toFeet());
    highestElevation = (highestElevation > defaultUpperLimit) ? highestElevation * 1.1 : defaultUpperLimit;
    elevations.pop_back();


    //Paint the Terrain - Draw the polygon
    std::vector<QPointF> polygons;

    polygons.push_back(QPointF(0, widgetHeight));
    for (int i = 0; i < elevations.size(); i++) {
        auto elevation = elevations[i];
        auto x = widgetWidth / steps * i;
        auto y = yCoordinate(elevation, widgetHeight, highestElevation, 0);
        polygons.push_back(QPointF(x, y));
    }
    polygons.push_back(QPointF(widgetWidth * 1.1, widgetHeight));
    painter->drawPolygon(polygons.data(), polygons.size());



    //Paint the Aircraft
    painter->fillRect(0, yCoordinate(info.trueAltitudeAMSL().toFeet(), widgetHeight, highestElevation, 10), 10, 10, QColor("white"));

    qDebug() << "Drawing took" << timer.elapsed() << "milliseconds"; //TODO Remove

}



int Ui::SideViewQuickItem::yCoordinate(int altitude, int windowHeight, int maxHeight, int objectHeight) {
    if (altitude > maxHeight) {
        return maxHeight;
    }

    int heightOnDisplay = static_cast<double>(altitude) / maxHeight * windowHeight;

    auto result = windowHeight - heightOnDisplay - objectHeight / 2;
    return result;
}
