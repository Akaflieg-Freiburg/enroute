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
    QElapsedTimer timer;
    timer.start();

    auto info = GlobalObject::positionProvider()->positionInfo();
    auto track = info.trueTrack().toDEG();
    if (qIsNaN(track)) {
        drawNoTrackAvailable(painter);
        return;
    }


    auto geoMapProvider = GlobalObject::geoMapProvider();
    drawSky(painter);

    const float steps = 100;
    const float stepSizeInMeter = (info.groundSpeed().toKMH() > 3) ? (info.groundSpeed().toKMH() / 6 / steps * 1000) : 250;
    const float defaultUpperLimit = 1000;

    std::vector<int> elevations = getElevations(info, track, steps, stepSizeInMeter, geoMapProvider);
    int highestElevation = getHighestElevation(elevations, info, defaultUpperLimit);

    auto mergedAirspaces = getMergedAirspaces(info, track, steps, stepSizeInMeter, geoMapProvider);

    drawAirspaces(painter, elevations, mergedAirspaces, highestElevation, steps);

    drawTerrain(painter, elevations, highestElevation, steps);
    drawAircraft(painter, info, highestElevation);

    drawFlightPath(painter, info, highestElevation);


    qDebug() << "Drawing took" << timer.elapsed() << "milliseconds"; //TODO Remove
}

void Ui::SideViewQuickItem::drawNoTrackAvailable(QPainter *painter)
{
    drawSky(painter);

    // Define gradient for the ground
    QLinearGradient groundGradient(0, widgetHeight() * 0.8, 0, widgetHeight());
    groundGradient.setColorAt(0.0, QColor(139, 69, 19));   // SaddleBrown at the top
    groundGradient.setColorAt(1.0, QColor(210, 180, 140)); // Tan at the bottom

    // Fill the ground with the ground gradient
    painter->fillRect(0, widgetHeight() * 0.8, widgetWidth(), widgetHeight(), groundGradient);

    // Draw a semi-transparent overlay
    painter->fillRect(0, 0, widgetWidth(), widgetHeight(), QColor(0, 0, 0, 50));

    // Set up the font
    QFont font = painter->font();
    font.setPixelSize(35); // Larger font size
    font.setBold(true);    // Bold font
    painter->setFont(font);

    // Set up text color
    painter->setPen(QPen(Qt::white));

    // Draw the text with shadow for better readability
    auto text = "Not sufficient data";
    painter->setPen(QPen(Qt::black));
    painter->drawText(1, 21, widgetWidth(), 40, Qt::AlignCenter, text);
    painter->setPen(QPen(Qt::white));
    painter->drawText(0, 20, widgetWidth(), 40, Qt::AlignCenter, text);
}

void Ui::SideViewQuickItem::drawSky(QPainter *painter)
{
    // Define gradient for the sky
    QLinearGradient skyGradient(0, 0, 0, widgetHeight());
    skyGradient.setColorAt(0.0, QColor(135, 206, 235)); // Light blue at the top
    skyGradient.setColorAt(1.0, QColor(0, 191, 255));   // Deeper blue at the bottom

    // Fill the background with the sky gradient
    painter->fillRect(0, 0, widgetWidth(), widgetHeight(), skyGradient);
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
    auto categories = airspaceSortedCategories();
    for (int i = 0; i <= steps; i++) {
        auto position = info.coordinate().atDistanceAndAzimuth(i * stepSizeInMeter, track, 0);

        auto airspaces = GlobalObject::geoMapProvider()->airspaces(position);

        for (const QVariant &var : airspaces) {
            GeoMaps::Airspace airspace = qvariant_cast<GeoMaps::Airspace>(var);
            if (std::find(std::begin(categories), std::end(categories), airspace.CAT()) != std::end(categories)) {
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

QStringList Ui::SideViewQuickItem::airspaceSortedCategories() {
    return {"TMZ", "RMZ", "DNG", "D", "C", "CTR", "R"};
}

void Ui::SideViewQuickItem::drawAirspaces(QPainter *painter, std::vector<int> elevations, const std::vector<MergedAirspace> &mergedAirspaces, int highestElevation, float steps)
{
    auto categories = airspaceSortedCategories();
    std::vector<MergedAirspace> sortedAirspaces = mergedAirspaces;
    std::sort(sortedAirspaces.begin(), sortedAirspaces.end(), [categories](const MergedAirspace &a, const MergedAirspace &b) {
        return categories.indexOf(a.airspace.CAT()) < categories.indexOf(b.airspace.CAT());
    });

    for (const MergedAirspace &mergedAirspace : sortedAirspaces) {

        auto airspace = mergedAirspace.airspace;

        int firstStep = mergedAirspace.firstStep;
        int lastStep = mergedAirspace.lastStep;

        int xStart = static_cast<int>((firstStep / steps) * widgetWidth());
        int xEnd = static_cast<int>((lastStep / steps) * widgetWidth());

        QColor color;
        if (mergedAirspace.airspace.CAT() == "CTR" || mergedAirspace.airspace.CAT() == "R" || mergedAirspace.airspace.CAT() == "DNG") {
            color = QColor("red");
        } else if (mergedAirspace.airspace.CAT() == "TMZ"){
            color = QColor("grey");
        } else {
            color = QColor("blue");
        }
        color.setAlphaF(0.75);
        painter->setBrush(color.lighter(160));

        auto lowerBound = mergedAirspace.airspace.lowerBound().toLower();
        auto upperBound = mergedAirspace.airspace.upperBound().toLower();

        //Are Bounds relative to Ground? If so, we cant use rects, we have to use polygons to follow the elevation
        if (lowerBound.endsWith("agl") || lowerBound.endsWith("gnd") || upperBound.endsWith("agl") || upperBound.endsWith("gnd") ) {
            std::vector<QPointF> polygons;
            for (int i = mergedAirspace.firstStep; i <= mergedAirspace.lastStep; i++) {
                auto x = widgetWidth() / steps * i;
                auto y = yCoordinate(airspace.estimatedLowerBoundMSL(elevations[i]).toFeet(), highestElevation, 0);
                qWarning() << "y" << y;
                polygons.push_back(QPointF(x, y));
            }
            for (int i = mergedAirspace.lastStep; i >= mergedAirspace.firstStep; i--) {
                auto x = widgetWidth() / steps * i;
                auto y = yCoordinate(airspace.estimatedUpperBoundMSL(elevations[i]).toFeet(), highestElevation, 0);
                qWarning() << "y" << y;
                polygons.push_back(QPointF(x, y));
            }
            painter->drawPolygon(polygons.data(), static_cast<int>(polygons.size()));

            // Calculate the centroid of the polygon
            QPointF centroid(0, 0);
            for (const auto &point : polygons) {
                centroid += point;
            }
            centroid /= polygons.size();

            QFont font = painter->font();
            font.setPointSizeF(13); //TODO: Make dynamic??
            painter->setFont(font);
            // Adjust for label positioning
            QFontMetrics metrics = painter->fontMetrics();
            QString label = airspace.CAT();
            int textWidth = metrics.horizontalAdvance(label);
            int textHeight = metrics.height();

            // Draw the label at the centroid, adjusting to center the text
            painter->drawText(centroid.x() - textWidth / 2, centroid.y() + textHeight / 2, label);
        } else {
            int lowerY = yCoordinate(airspace.estimatedLowerBoundMSL().toFeet(), highestElevation, 0);
            int upperY = qMax(yCoordinate(airspace.estimatedUpperBoundMSL().toFeet(), highestElevation, 0), 0);

            QRect rect(xStart, upperY, xEnd - xStart, lowerY - upperY);

            // Set brush color based on airspace category

            QPen pen = painter->pen();
            pen.setStyle(Qt::DotLine);
            painter->setPen(pen);
            painter->drawRect(rect);

            // Restore solid line for text
            pen.setStyle(Qt::SolidLine);
            painter->setPen(pen);

            // Calculate appropriate font size
            QFont font = painter->font();
            int fontSize = qMin(12.0, qMax(qMin(rect.height() * 0.9, (double)rect.width()), 15.0)); // Ensure minimum font size
            font.setPointSizeF(fontSize);
            painter->setFont(font);

            QString category = mergedAirspace.airspace.CAT();

            // Draw category text inside the rectangle
            painter->drawText(rect, Qt::AlignCenter, category);
            //TODO: What if that possition is blocked? -> Draw labels later on top?
            //TODO: Draw font beyond rect if needed and possible?
        }

    }
}


void Ui::SideViewQuickItem::drawTerrain(QPainter *painter, const std::vector<int> &elevations, int highestElevation, float steps)
{
    QLinearGradient terrainGradient(0, 0, 0, widgetHeight());
    terrainGradient.setColorAt(0.0, QColor(153, 102, 51));   // Dark brown at the base
    terrainGradient.setColorAt(1.0, QColor(101, 67, 33));  // Lighter brown at the top
    painter->setBrush(terrainGradient);

    std::vector<QPointF> polygons;
    polygons.push_back(QPointF(0, widgetHeight())); // Additional polygon at the very left side to fill the terrain with color

    for (size_t i = 0; i < elevations.size(); ++i) {
        auto elevation = elevations[i];
        auto x = widgetWidth() / steps * i;
        auto y = yCoordinate(elevation, highestElevation, 0);
        polygons.push_back(QPointF(x, y));
    }

    polygons.push_back(QPointF(widgetWidth() * 1.1, widgetHeight())); // Additional polygon outside the screen for improved design at the RH side of the screen
    painter->drawPolygon(polygons.data(), static_cast<int>(polygons.size()));
}

void Ui::SideViewQuickItem::drawAircraft(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation)
{
    auto altitude = info.trueAltitudeAMSL().toFeet(); //TODO: This is the wrong altitude
    painter->fillRect(0, yCoordinate(altitude, highestElevation, 10), 10, 10, QColor("black"));
}

void Ui::SideViewQuickItem::drawFlightPath(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation)
{
    auto altitude = info.trueAltitudeAMSL().toFeet(); //TODO: This is the wrong altitude
    auto verticalSpeed = info.verticalSpeed().toFPM();
    auto altitudeIn10Minutes = verticalSpeed * 10 + altitude;

    painter->drawLine(10, yCoordinate(altitude, highestElevation, 0), widgetWidth() + 10, yCoordinate(altitude, highestElevation, 0));
    QPen pen = painter->pen();
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    painter->drawLine(10, yCoordinate(altitude, highestElevation, 0), widgetWidth() + 10, yCoordinate(altitudeIn10Minutes, highestElevation, 0));
}

int Ui::SideViewQuickItem::yCoordinate(int altitude, int maxHeight, int objectHeight)
{

    if (altitude > maxHeight) return 0;
    int heightOnDisplay = static_cast<int>(static_cast<double>(altitude) / maxHeight * widgetHeight());
    return widgetHeight() - heightOnDisplay - objectHeight / 2;
}

int Ui::SideViewQuickItem::widgetHeight()
{
    return static_cast<int>(height());
}

int Ui::SideViewQuickItem::widgetWidth()
{
    return static_cast<int>(width());
}
