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
#include <QElapsedTimer>
#include <QFont>
#include <QDebug>
#include <QPen>
#include <QRect>
#include <QVector>
#include <set>

#include "GlobalObject.h"
#include "PositionProvider.h"
#include "PositionInfo.h"
#include "GeoMapProvider.h"
#include "SideViewQuickItem.h"

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
        drawNoTrackAvailable(painter);
        return;
    }

    QElapsedTimer timer;
    timer.start();

    int widgetHeight = static_cast<int>(height());
    int widgetWidth = static_cast<int>(width());

    auto geoMapProvider = GlobalObject::geoMapProvider();
    drawSky(painter, widgetHeight, widgetWidth);

    const float steps = 100;
    const float stepSizeInMeter = (info.groundSpeed().toKMH() > 3) ? (info.groundSpeed().toKMH() / 6 / steps * 1000) : 250;
    const float defaultUpperLimit = 1000;

    std::vector<int> elevations = getElevations(info, track, steps, stepSizeInMeter, geoMapProvider);
    int highestElevation = getHighestElevation(elevations, info, defaultUpperLimit);

    auto mergedAirspaces = getMergedAirspaces(info, track, steps, stepSizeInMeter, geoMapProvider);
    drawAirspaces(painter, mergedAirspaces, widgetWidth, widgetHeight, highestElevation, steps);

    drawTerrain(painter, elevations, highestElevation, widgetWidth, widgetHeight, steps);
    drawAircraft(painter, info, highestElevation);

    drawFlightPath(painter, info, widgetWidth, highestElevation);


    qDebug() << "Drawing took" << timer.elapsed() << "milliseconds"; //TODO Remove
}

void Ui::SideViewQuickItem::drawNoTrackAvailable(QPainter *painter)
{
    painter->fillRect(0, 0, static_cast<int>(width()), static_cast<int>(height()), QColor("lightblue"));
    painter->fillRect(0, static_cast<int>(height()) * 0.8, static_cast<int>(width()), static_cast<int>(height()), QColor("brown"));
    QFont font = painter->font();
    font.setPixelSize(25);
    painter->setFont(font);
    painter->drawText(0, 20, "No track available");
}

void Ui::SideViewQuickItem::drawSky(QPainter *painter, int widgetHeight, int widgetWidth)
{
    painter->fillRect(0, 0, widgetWidth, widgetHeight, QColor("lightblue"));
}

std::vector<int> Ui::SideViewQuickItem::getElevations(const Positioning::PositionInfo &info, double track, float steps, float stepSizeInMeter, const GeoMaps::GeoMapProvider *geoMapProvider)
{
    std::vector<int> elevations;
    for (int i = 0; i <= steps; i++) {
        auto position = info.coordinate().atDistanceAndAzimuth(i * stepSizeInMeter, track, 0);
        auto elevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(position).toFeet();
        elevations.push_back(elevation);
    }
    return elevations;
}

int Ui::SideViewQuickItem::getHighestElevation(std::vector<int> &elevations, const Positioning::PositionInfo &info, float defaultUpperLimit)
{
    elevations.push_back(info.trueAltitudeAMSL().toFeet());
    int highestElevation = *std::max_element(elevations.begin(), elevations.end());
    highestElevation = std::max(static_cast<int>(highestElevation * 1.3), static_cast<int>(defaultUpperLimit));
    elevations.pop_back();
    return highestElevation;
}

std::vector<Ui::SideViewQuickItem::MergedAirspace> Ui::SideViewQuickItem::getMergedAirspaces(const Positioning::PositionInfo &info, double track, float steps, float stepSizeInMeter, const GeoMaps::GeoMapProvider *geoMapProvider)
{
    std::map<int, std::vector<GeoMaps::Airspace>> stepAirspaces;
    for (int i = 0; i <= steps; i++) {
        auto position = info.coordinate().atDistanceAndAzimuth(i * stepSizeInMeter, track, 0);

        auto airspaces = GlobalObject::geoMapProvider()->airspaces(position);

        for (const QVariant &var : airspaces) {
            GeoMaps::Airspace airspace = qvariant_cast<GeoMaps::Airspace>(var);
            if (airspace.CAT() == "CTR" || airspace.CAT() == "D") {
                stepAirspaces[i].push_back(airspace);
            }
        }
    }

    std::vector<MergedAirspace> mergedAirspaces;
    for (const auto &step : stepAirspaces) {
        int stepIndex = step.first;

        for (const GeoMaps::Airspace &airspace : step.second) {
            bool merged = false;

            for (auto &mergedAirspace : mergedAirspaces) {
                if (mergedAirspace.airspace.CAT() == airspace.CAT() &&
                    mergedAirspace.airspace.name() == airspace.name() &&
                    mergedAirspace.airspace.lowerBound() == airspace.lowerBound() &&
                    mergedAirspace.airspace.upperBound() == airspace.upperBound()) {
                    mergedAirspace.lastStep = stepIndex;
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                MergedAirspace newMergedAirspace = { airspace, stepIndex, stepIndex };
                mergedAirspaces.push_back(newMergedAirspace);
            }
        }
    }

    return mergedAirspaces;
}

void Ui::SideViewQuickItem::drawAirspaces(QPainter *painter, const std::vector<MergedAirspace> &mergedAirspaces, int widgetWidth, int widgetHeight, int highestElevation, float steps)
{
    for (const MergedAirspace &mergedAirspace : mergedAirspaces) {
        int firstStep = mergedAirspace.firstStep;
        int lastStep = mergedAirspace.lastStep;

        int xStart = static_cast<int>((firstStep / steps) * widgetWidth);
        int xEnd = static_cast<int>((lastStep / steps) * widgetWidth);

        double lowerBound = mergedAirspace.airspace.estimatedLowerBoundMSL().toFeet();
        double upperBound = mergedAirspace.airspace.estimatedUpperBoundMSL().toFeet();

        int lowerY = yCoordinate(lowerBound, highestElevation, 0);
        int upperY = qMax(yCoordinate(upperBound, highestElevation, 0), 0);

        QRect rect(xStart, upperY, xEnd - xStart, lowerY - upperY);

        // Set brush color based on airspace category
        if (mergedAirspace.airspace.CAT() == "CTR") {
            painter->setBrush(QColor("red").lighter(160));
        } else {
            painter->setBrush(QColor("blue").lighter(160));
        }

        QPen pen = painter->pen();
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRect(rect);

        // Restore solid line for text
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);

        // Calculate appropriate font size
        QFont font = painter->font();
        int rectHeight = lowerY - upperY;
        int fontSize = qMax(10, rectHeight / 3); // Ensure minimum font size
        font.setPointSizeF(fontSize);
        painter->setFont(font);

        QString category = mergedAirspace.airspace.CAT();

        // Draw category text inside the rectangle
        painter->drawText(rect, Qt::AlignCenter, category);
        //TODO: What if that possition is blocked? -> Draw labels later on top?
    }
}

void Ui::SideViewQuickItem::drawTerrain(QPainter *painter, const std::vector<int> &elevations, int highestElevation, int widgetWidth, int widgetHeight, float steps)
{
    painter->setBrush(QColor("brown"));

    std::vector<QPointF> polygons;
    polygons.push_back(QPointF(0, widgetHeight)); // Additional polygon at the very left side to fill the terrain with color

    for (size_t i = 0; i < elevations.size(); ++i) {
        auto elevation = elevations[i];
        auto x = widgetWidth / steps * i;
        auto y = yCoordinate(elevation, highestElevation, 0);
        polygons.push_back(QPointF(x, y));
    }

    polygons.push_back(QPointF(widgetWidth * 1.1, widgetHeight)); // Additional polygon outside the screen for improved design at the RH side of the screen
    painter->drawPolygon(polygons.data(), static_cast<int>(polygons.size()));
}

void Ui::SideViewQuickItem::drawAircraft(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation)
{
    auto altitude = info.trueAltitudeAMSL().toFeet();
    painter->fillRect(0, yCoordinate(altitude, highestElevation, 10), 10, 10, QColor("black"));
}

void Ui::SideViewQuickItem::drawFlightPath(QPainter *painter, const Positioning::PositionInfo &info, int widgetWidth, int highestElevation)
{
    auto altitude = info.trueAltitudeAMSL().toFeet();
    auto verticalSpeed = info.verticalSpeed().toFPM();
    auto altitudeIn10Minutes = verticalSpeed * 10 + altitude;

    painter->drawLine(10, yCoordinate(altitude, highestElevation, 0), widgetWidth + 10, yCoordinate(altitude, highestElevation, 0));
    QPen pen = painter->pen();
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    painter->drawLine(10, yCoordinate(altitude, highestElevation, 0), widgetWidth + 10, yCoordinate(altitudeIn10Minutes, highestElevation, 0));
}

int Ui::SideViewQuickItem::yCoordinate(int altitude, int maxHeight, int objectHeight)
{
    int safeAreaBottom = 34; //TODO: Replace with correct value
    int widgetHeight = static_cast<int>(height());
    auto availableHeight = widgetHeight - safeAreaBottom;

    int heightOnDisplay = static_cast<int>(static_cast<double>(altitude) / maxHeight * availableHeight);
    return availableHeight - heightOnDisplay - objectHeight / 2;
}
