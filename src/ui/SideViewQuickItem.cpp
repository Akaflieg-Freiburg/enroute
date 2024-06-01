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
    if (!pressureAltitude().isFinite() || qIsNaN(track)) {
        drawNoTrackAvailable(painter);
        return;
    }


    auto geoMapProvider = GlobalObject::geoMapProvider();
    drawSky(painter);

    const float steps = 100;
    const float stepSizeInMeter = info.groundSpeed().toKMH() / 6 / steps * 1000;
    const float defaultUpperLimit = 1000;

    std::vector<int> elevations = getElevations(info, track, steps, stepSizeInMeter, geoMapProvider);
    int highestElevation = getHighestElevation(elevations, info, defaultUpperLimit);

    auto airspaces = getMergedAirspaces(info, track, steps, stepSizeInMeter, geoMapProvider);
    auto mergedAirspaces = processAirspaces(airspaces, elevations, steps, highestElevation);

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
            if (!airspace.name().contains("KARLSRUHE")) {
                //continue;
            }
            if (std::find(std::begin(categories), std::end(categories), airspace.CAT()) != std::end(categories)) {
                stepAirspaces[i].push_back(airspace);
            }
        }
    }

    std::vector<MergedAirspace> allMergedAirspaces;
    for (const auto &step : stepAirspaces) {
        int stepIndex = step.first;

        for (const GeoMaps::Airspace &airspace : step.second) {
            bool merged = false;

            for (auto &mergedAirspace : allMergedAirspaces) {
                if (mergedAirspace.airspace.CAT() == airspace.CAT() &&
                    mergedAirspace.airspace.name() == airspace.name() &&
                    mergedAirspace.airspace.lowerBound() == airspace.lowerBound() &&
                    mergedAirspace.airspace.upperBound() == airspace.upperBound()) {
                    mergedAirspace.lastStep = stepIndex + 1; //TODO: Is the +1 a good idea?
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                MergedAirspace newMergedAirspace = { airspace, stepIndex, stepIndex };
                allMergedAirspaces.push_back(newMergedAirspace);
            }
        }
    }

    return allMergedAirspaces;
}

std::vector<Ui::SideViewQuickItem::MergedAirspace> Ui::SideViewQuickItem::processAirspaces(std::vector<MergedAirspace> mergedAirspaces, std::vector<int> &elevations, float steps, int highestElevation) {
    for (auto &mergedAirspace : mergedAirspaces) {
        auto airspace = mergedAirspace.airspace;
        auto lowerBound = airspace.lowerBound().toLower();
        auto upperBound = airspace.upperBound().toLower();

        QList<QPoint> polygons;

        //Are Bounds relative to Ground? If so, we cant use simple rectangles
        if (lowerBound.endsWith("agl") || lowerBound.endsWith("gnd") || upperBound.endsWith("agl") || upperBound.endsWith("gnd") ) {
            for (int i = mergedAirspace.firstStep; i <= mergedAirspace.lastStep; i++) {
                auto x = widgetWidth() / steps * i;
                auto y = yCoordinate(airspace.estimatedLowerBoundMSL(elevations[i]).toFeet(), highestElevation, 0);
                polygons.push_back(QPoint(x, y));
            }
            for (int i = mergedAirspace.lastStep; i >= mergedAirspace.firstStep; i--) {
                auto x = widgetWidth() / steps * i;
                auto y = yCoordinate(airspace.estimatedUpperBoundMSL(elevations[i]).toFeet(), highestElevation, 0);
                polygons.push_back(QPoint(x, y));
            }

        } else {
            int xStart = static_cast<int>((mergedAirspace.firstStep / steps) * widgetWidth());
            int xEnd = static_cast<int>((mergedAirspace.lastStep / steps) * widgetWidth());

            int lowerY = yCoordinate(airspace.estimatedLowerBoundMSL().toFeet(), highestElevation, 0);
            int upperY = qMax(yCoordinate(airspace.estimatedUpperBoundMSL().toFeet(), highestElevation, 0), 0);

            QRect rect(xStart, upperY, xEnd - xStart, lowerY - upperY);
            //qWarning() << "xStart" << xStart << "upperY" << upperY << "xEnd" << xEnd << "lowerY" << lowerY << airspace.estimatedLowerBoundMSL().toFeet();
            polygons.push_back(QPoint(xStart, lowerY));
            polygons.push_back(QPoint(xStart, upperY));
            polygons.push_back(QPoint(xEnd, upperY));
            polygons.push_back(QPoint(xEnd, lowerY));
        }
        mergedAirspace.polygon = QPolygon(polygons);
    }

    /*
    std::vector<MergedAirspace> airspacesBefore = mergedAirspaces;
    std::vector<MergedAirspace> airspacesAfter = mergeAirspaces(airspacesBefore);
    int counter = 1;


    while (airspacesBefore.size() != airspacesAfter.size()) {
        airspacesBefore = airspacesAfter;
        airspacesAfter = mergeAirspaces(airspacesBefore);
        counter++;
    }
    qWarning() << "counter" << counter;


*/

    return mergedAirspaces; //TODO merge later
}


QStringList Ui::SideViewQuickItem::airspaceSortedCategories() {
    return {"TMZ", "RMZ", "NRA", "DNG", "D", "C", "B", "A", "CTR", "R"};
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
        } else if (airspace.CAT() == "NRA") {
            color = QColor("green");
        } else {
            color = QColor("blue");
        }
        //color.setAlphaF(0.75);
        painter->setBrush(color.lighter(160));

        auto lowerBound = mergedAirspace.airspace.lowerBound().toLower();
        auto upperBound = mergedAirspace.airspace.upperBound().toLower();

        QPolygon polygons;
        polygons = mergedAirspace.polygon;


        QPen pen = painter->pen();
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawPolygon(polygons);
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);

        QPointF centroid = getPolygonCentroid(mergedAirspace.polygon);

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


    }
}


void Ui::SideViewQuickItem::drawTerrain(QPainter *painter, const std::vector<int> &elevations, int highestElevation, float steps)
{
    QLinearGradient terrainGradient(0, 0, 0, widgetHeight());
    terrainGradient.setColorAt(0.0, QColor(153, 102, 51));
    terrainGradient.setColorAt(1.0, QColor(101, 67, 33));
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
    auto altitude = pressureAltitude().toFeet();
    painter->fillRect(0, yCoordinate(altitude, highestElevation, 10), 10, 10, QColor("black"));
}

void Ui::SideViewQuickItem::drawFlightPath(QPainter *painter, const Positioning::PositionInfo &info, int highestElevation)
{
    auto altitude = pressureAltitude().toFeet();
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

    if (altitude > maxHeight) return 0; //Dont draw above the widget
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

Units::Distance Ui::SideViewQuickItem::pressureAltitude() {
    //return GlobalObject::positionProvider()->pressureAltitude();
    return GlobalObject::positionProvider()->positionInfo().trueAltitudeAMSL();
}


QPointF Ui::SideViewQuickItem::getPolygonCentroid(const QPolygonF &polygon)
{
    qreal centroid_x = 0.0, centroid_y = 0.0;
    qreal signedArea = 0.0;
    qreal x0 = 0.0, y0 = 0.0;  // Current vertex coordinates
    qreal x1 = 0.0, y1 = 0.0;  // Next vertex coordinates
    qreal a = 0.0;  // Partial signed area

    // For all vertices except last
    int i;
    for (i = 0; i < polygon.size() - 1; ++i)
    {
        x0 = polygon[i].x();
        y0 = polygon[i].y();
        x1 = polygon[i + 1].x();
        y1 = polygon[i + 1].y();
        a = x0 * y1 - x1 * y0;
        signedArea += a;
        centroid_x += (x0 + x1) * a;
        centroid_y += (y0 + y1) * a;
    }

    // Do last vertex separately to avoid performing an expensive modulo operation in each iteration
    x0 = polygon[i].x();
    y0 = polygon[i].y();
    x1 = polygon[0].x();
    y1 = polygon[0].y();
    a = x0 * y1 - x1 * y0;
    signedArea += a;
    centroid_x += (x0 + x1) * a;
    centroid_y += (y0 + y1) * a;

    signedArea *= 0.5;
    centroid_x /= (6 * signedArea);
    centroid_y /= (6 * signedArea);

    return QPointF(centroid_x, centroid_y);
}

std::vector<Ui::SideViewQuickItem::MergedAirspace> Ui::SideViewQuickItem::mergeAirspaces(std::vector<MergedAirspace> mergedAirspaces) {
    std::vector<MergedAirspace> reallyMergedAirspaces;
    std::set<MergedAirspace> processed;
    for (size_t i = 0; i < mergedAirspaces.size(); ++i) {
        auto airspace = mergedAirspaces[i];
        qWarning() << "Outer loop" << airspace.airspace.name();
        if (!processed.contains(airspace)) {
            MergedAirspace merged = airspace;
            processed.insert(airspace);
            qWarning() << "inserting airspace" << airspace.airspace.name();

            // Iterate through the remaining airspaces to find and merge overlapping ones
            for (size_t j = i + 1; j < mergedAirspaces.size(); ++j) {
                auto comparingAirspace = mergedAirspaces[j];
                qWarning() << "Comparing to airspace" << comparingAirspace.airspace.name();
                if (!processed.contains(comparingAirspace) &&
                    comparingAirspace.airspace.name() == airspace.airspace.name() &&
                    comparingAirspace.airspace.CAT() == airspace.airspace.CAT() &&
                    comparingAirspace.polygon.intersects(airspace.polygon)) {

                    // Merge the polygons
                    merged.polygon = merged.polygon.united(comparingAirspace.polygon);
                    merged.firstStep = std::min(merged.firstStep, comparingAirspace.firstStep);
                    merged.lastStep = std::max(merged.lastStep, comparingAirspace.lastStep);
                    processed.insert(comparingAirspace);
                    qWarning() << "Merged" << merged.airspace.name();
                }
            }

            reallyMergedAirspaces.push_back(merged);
        } else {
            qWarning() << "skipping airspace" << airspace.airspace.name();
        }
    }
    return reallyMergedAirspaces;
}
