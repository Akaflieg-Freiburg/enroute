/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

#include <QFile>
#include <QHash>

#include "GlobalObject.h"
#include "navigation/Aircraft.h"
#include "navigation/Navigator.h"
#include "traffic/TrafficFactor_WithPosition.h"

using namespace Qt::Literals::StringLiterals;


Traffic::TrafficFactor_WithPosition::TrafficFactor_WithPosition(QObject *parent) : TrafficFactor_Abstract(parent)
{  
    // Bindings for property description
    connect(this, &Traffic::TrafficFactor_WithPosition::positionInfoChanged, this, &Traffic::TrafficFactor_WithPosition::dispatchUpdateDescription);

    // Bindings for property icon
    connect(this, &Traffic::TrafficFactor_Abstract::colorChanged, this, &Traffic::TrafficFactor_WithPosition::updateIcon);
    connect(this, &Traffic::TrafficFactor_Abstract::typeChanged, this, &Traffic::TrafficFactor_WithPosition::updateIcon);
    connect(this, &Traffic::TrafficFactor_WithPosition::positionInfoChanged, this, &Traffic::TrafficFactor_WithPosition::updateIcon);

    // Bindings for property valid
    connect(this, &Traffic::TrafficFactor_WithPosition::positionInfoChanged, this, &Traffic::TrafficFactor_WithPosition::dispatchUpdateValid);
}


void Traffic::TrafficFactor_WithPosition::setPositionInfo(const Positioning::PositionInfo& newPositionInfo)
{
    if (m_positionInfo == newPositionInfo) {
        return;
    }
    m_positionInfo = newPositionInfo;
    emit positionInfoChanged();
}


void Traffic::TrafficFactor_WithPosition::updateDescription()
{
    QStringList results;

    if (!callSign().isEmpty()) {
        results << callSign();
    }

    switch(type()) {
    case Traffic::Aircraft:
        results << tr("Aircraft");
        break;
    case Traffic::Airship:
        results << tr("Airship");
        break;
    case Traffic::Balloon:
        results << tr("Balloon");
        break;
    case Traffic::Copter:
        results << tr("Copter");
        break;
    case Traffic::Drone:
        results << tr("Drone");
        break;
    case Traffic::Glider:
        results << tr("Glider");
        break;
    case Traffic::HangGlider:
        results << tr("Hang glider");
        break;
    case Traffic::Jet:
        results << tr("Jet");
        break;
    case Traffic::Paraglider:
        results << tr("Paraglider");
        break;
    case Traffic::Skydiver:
        results << tr("Skydiver");
        break;
    case Traffic::StaticObstacle:
        results << tr("Static Obstacle");
        break;
    case Traffic::TowPlane:
        results << tr("Tow Plane");
        break;
    default:
        results << tr("Traffic");
        break;
    }

    if (!positionInfo().coordinate().isValid()) {
        results << tr("Position unknown");
    }

    if (vDist().isFinite()) {       
        QString result = GlobalObject::navigator()->aircraft().verticalDistanceToString(vDist(), true);
        auto climbRateMPS = m_positionInfo.verticalSpeed().toMPS();
        if ( qIsFinite(climbRateMPS) ) {
            if (climbRateMPS < -1.0) {
                result += QStringLiteral(" ↘");
            }
            if ((climbRateMPS >= -1.0) && (climbRateMPS <= +1.0)) {
                result += QStringLiteral(" →");
            }
            if (climbRateMPS > 1.0) {
                result += QStringLiteral(" ↗");
            }
        }
        results << result;
    }

    // Set property value
    auto newDescription = results.join(u"<br>");
    if (m_description == newDescription) {
        return;
    }
    m_description = newDescription;
    emit descriptionChanged();
}


void Traffic::TrafficFactor_WithPosition::updateIcon()
{
    // To not store each traffic icon in multiple colors, 
    // use a lazy cache keyed by "<shape>-<color>"
    static QHash<QString, QString> iconCache;

    // Map alarm color name to hex color code for use with SVG template files
    static const QHash<QString, QString> colorMap = {
        { QStringLiteral("green"),  QStringLiteral("#00a000") },
        { QStringLiteral("yellow"), QStringLiteral("#f0f000") },
        { QStringLiteral("red"),    QStringLiteral("#a00000") },
    };

    // Determine base icon shape from direction availability and aircraft type
    QString baseType = QStringLiteral("noDirection");
    if (m_positionInfo.groundSpeed().isFinite() && m_positionInfo.trueTrack().isFinite())
    {
        auto GS = m_positionInfo.groundSpeed();
        if (GS.isFinite() && (GS.toKN() > 4))
        {
            switch(type()) {
            case Traffic::Aircraft:
            case Traffic::TowPlane:
                baseType = QStringLiteral("aircraft");
                break;
            case Traffic::Glider:
                baseType = QStringLiteral("glider");
                break;
            case Traffic::Paraglider:
                baseType = QStringLiteral("paraglider");
                break;
            case Traffic::HangGlider:
                baseType = QStringLiteral("hangGlider");
                break;
            case Traffic::Jet:
                baseType = QStringLiteral("jet");
                break;
            case Traffic::Copter:
                baseType = QStringLiteral("copter");
                break;
            case Traffic::Drone:
                baseType = QStringLiteral("drone");
                break;
            case Traffic::Balloon:
                baseType = QStringLiteral("balloon");
                break;
            default:
                baseType = QStringLiteral("withDirection");
                break;
            }
        }
    }

    // Use icon from Cache if exists, e.g. "glider-green" or "copter-red"
    const QString cacheKey = baseType + u'-' + color();
    const auto cachedIcon = iconCache.constFind(cacheKey);
    if (cachedIcon != iconCache.constEnd()) {
        if (m_icon != *cachedIcon) {
            m_icon = *cachedIcon;
            emit iconChanged();
        }
        return;
    }

    // Load the template SVG (placeholder fill color is "pink"), 
    // replace color, 
    // store in Cache
    QFile svgFile(u":/icons/traffic-"_s + baseType + u".svg"_s);
    if (!svgFile.open(QIODevice::ReadOnly)) {
        return;
    }
    QString svgContent = QString::fromUtf8(svgFile.readAll());
    const QString fillColor = colorMap.value(color(), QStringLiteral("#a00000"));
    svgContent.replace(QStringLiteral("pink"), fillColor);
    auto newIcon = u"data:image/svg+xml;base64,"_s
                   + QString::fromLatin1(svgContent.toUtf8().toBase64());
    iconCache.insert(cacheKey, newIcon);
    if (m_icon == newIcon) {
        return;
    }
    m_icon = newIcon;
    emit iconChanged();
}


void Traffic::TrafficFactor_WithPosition::updateValid()
{
    if (!positionInfo().isValid()) {
        if (m_valid) {
            m_valid = false;
            emit validChanged();
        }
        return;
    }

    TrafficFactor_Abstract::updateValid();
}
