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

    auto info = GlobalObject::positionProvider()->positionInfo();
    auto track = info.trueTrack().toDEG();
    if (qIsNaN(track)) {
        //TODO: Make more beautiful and dynamic
        painter->fillRect(0, 0, static_cast<int>(width()), static_cast<int>(height()), QColor("lightblue"));
        painter->fillRect(0, static_cast<int>(height())*0.8, static_cast<int>(width()), static_cast<int>(height()), QColor("brown"));
        QFont font = painter->font();
        font.setPixelSize(25);
        painter->setFont(font);
        painter->drawText(0, 20, "No track available");
        return;
    }

    QElapsedTimer timer;
    timer.start();


    int widgetHeight = static_cast<int>(height());
    int widgetWidth = static_cast<int>(width());


    auto geoMapProvider = GlobalObject::geoMapProvider();


    //Paint the Sky
    painter->fillRect(0, 0, static_cast<int>(width()), widgetHeight, QColor("lightblue"));


    //Paint the Terrain - Initialize
    float steps = 200;
    float stepSizeInMeter = (info.groundSpeed().toKMH() > 3) ? (info.groundSpeed().toKMH() / 6 / steps * 1000) : 250;
    float defaultUpperLimit = 1000;

    painter->setBrush(QColor("brown"));

    //Paint the Terrain - Get Elevations and find the highest
    std::vector<int> elevations;
    for (int i = 0; i <= steps; i++) {
        auto position = info.coordinate().atDistanceAndAzimuth(i * stepSizeInMeter, track, 0);
        auto elevation = geoMapProvider->terrainElevationAMSL(position).toFeet();
        elevations.push_back(elevation);
    }

    elevations.push_back(info.trueAltitudeAMSL().toFeet());
    int highestElevation = elevations[0];
    for (int elevation : elevations) {
        if (elevation > highestElevation) {
            highestElevation = elevation;
        }
    }
    highestElevation = qMax(highestElevation * 1.3, defaultUpperLimit);
    elevations.pop_back();


    //Paint the Terrain - Draw the polygon
    std::vector<QPointF> polygons;

    polygons.push_back(QPointF(0, widgetHeight)); //Additional polygon at the very left side to fill the terrain with color
    for (int i = 0; i < elevations.size(); i++) {
        auto elevation = elevations[i];
        auto x = widgetWidth / steps * i;
        auto y = yCoordinate(elevation, highestElevation, 0);
        polygons.push_back(QPointF(x, y));
    }
    polygons.push_back(QPointF(widgetWidth * 1.1, widgetHeight)); //Additional polygon outside the screen for improved design at the RH sight of the screen.
    painter->drawPolygon(polygons.data(), polygons.size());



    //Paint the Aircraft
    auto altitude = info.trueAltitudeAMSL().toFeet();
    painter->fillRect(0, yCoordinate(altitude, highestElevation, 10), 10, 10, QColor("black"));


    //Paint Flight Path
    auto verticalSpeed = info.verticalSpeed().toFPM();
    auto altitudeIn10Minutes = verticalSpeed * 10 + altitude;
    painter->drawLine(0 + 10, yCoordinate(altitude, highestElevation, 0), widgetWidth + 10, yCoordinate(altitude, highestElevation, 0));
    QPen pen = painter->pen();
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    painter->drawLine(0 + 10, yCoordinate(altitude, highestElevation, 0), widgetWidth + 10, yCoordinate(altitudeIn10Minutes, highestElevation, 0));

    qDebug() << "Drawing took" << timer.elapsed() << "milliseconds"; //TODO Remove

}



int Ui::SideViewQuickItem::yCoordinate(int altitude, int maxHeight, int objectHeight) {
    int safeAreaBottom = 34; //TODO: Replace with correct value
    int widgetHeight = static_cast<int>(height());
    auto availableHeight = widgetHeight - safeAreaBottom;
    if (altitude > maxHeight) {
        //return maxHeight;
    }

    int heightOnDisplay = static_cast<double>(altitude) / maxHeight * availableHeight;

    auto result = availableHeight - heightOnDisplay - objectHeight / 2;
    return result;
}
