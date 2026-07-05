/***************************************************************************
 *   Copyright (C) 2020-2026 by Stefan Kebekus                             *
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
#include <QTimer>

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "navigation/Aircraft.h"
#include "navigation/Navigator.h"
#include "traffic/TrafficFactor_WithPosition.h"

using namespace Qt::Literals::StringLiterals;

namespace {

// To not store each traffic icon in multiple colors, use a lazy cache keyed by
// "<shape>-<color>"
using StringStringHash = QHash<QString, QString>;
Q_GLOBAL_STATIC(StringStringHash, iconCache)

// Map alarm color name to hex color code for use with SVG template files. At
// night, the color property already holds a muted hex value that is used
// as-is, so only the day color names appear here.
Q_GLOBAL_STATIC(StringStringHash, colorMap, {{ QStringLiteral("green"),  QStringLiteral("#00a000") }, { QStringLiteral("yellow"), QStringLiteral("#f0f000") }, { QStringLiteral("red"), QStringLiteral("#a00000") }})

} // namespace

Traffic::TrafficFactor_WithPosition::TrafficFactor_WithPosition(QObject *parent) : TrafficFactor_Abstract(parent)
{
    // Update the extrapolated data at least whenever the position info changes.
    connect(this, &Traffic::TrafficFactor_WithPosition::positionInfoChanged, this, &Traffic::TrafficFactor_WithPosition::updateExtrapolatedData);

    // Bindings for property valid
    m_valid.setBinding([this]() {
        return positionInfo().isValid() && m_validAbstractTrafficFactor;
    });

    // Bindings for property icon. The installation of the binding is
    // deferred until the event loop runs, because it reads GlobalSettings —
    // see the color binding in TrafficFactor_Abstract for the reason.
    QTimer::singleShot(0, this, [this]() {
        m_icon.setBinding([this]() {
            // Determine base icon shape from direction availability and aircraft
            // type
            auto baseType = QStringLiteral("noDirection");
            if (m_positionInfo.value().groundSpeed().isFinite() && m_positionInfo.value().trueTrack().isFinite())
            {
                auto GS = m_positionInfo.value().groundSpeed();
                switch(type())
                {
                case Aircraft:
                case TowPlane:
                    baseType = QStringLiteral("aircraft");
                    break;
                case Glider:
                    baseType = QStringLiteral("glider");
                    break;
                case Paraglider:
                    baseType = QStringLiteral("paraglider");
                    break;
                case HangGlider:
                    baseType = QStringLiteral("hangGlider");
                    break;
                case Jet:
                    baseType = QStringLiteral("jet");
                    break;
                case Copter:
                    baseType = QStringLiteral("copter");
                    break;
                case Drone:
                    baseType = QStringLiteral("drone");
                    break;
                case Balloon:
                    baseType = QStringLiteral("balloon");
                    break;
                default:
                    baseType = QStringLiteral("withDirection");
                    break;
                }
            }

            // Use icon from Cache if exists, e.g. "glider-green" or "copter-red".
            // At night, color() holds a muted hex value instead of a name, so day
            // and night entries never collide; the suffix documents the intent.
            const bool nightMode = GlobalObject::globalSettings()->nightMode();
            const QString cacheKey = baseType + u'-' + color() + (nightMode ? u"-night"_s : QString());
            const auto cachedIcon = iconCache->constFind(cacheKey);
            if (cachedIcon != iconCache->constEnd())
            {
                return  *cachedIcon;
            }

            // Load the template SVG (placeholder fill color is "#000040"),
            // replace color, store in cache.
            QFile svgFile(u":/icons/traffic-"_s + baseType + u".svg"_s);
            if (!svgFile.open(QIODevice::ReadOnly))
            {
                return QString();
            }
            QString svgContent = QString::fromUtf8(svgFile.readAll());
            const QString fillColor = colorMap->value(color(), color());
            svgContent.replace(QStringLiteral("#000040"), fillColor);
            if (nightMode)
            {
                // Flip the white halo to dark, like the label halos on the moving map
                svgContent.replace(QStringLiteral("#ffffff"), QStringLiteral("#303030"));
            }
            auto newIcon = u"data:image/svg+xml;base64,"_s + QString::fromLatin1(svgContent.toUtf8().toBase64());
            iconCache->insert(cacheKey, newIcon);
            return newIcon;
        });
    });

    // Bindings for property description
    m_description.setBinding([this]() {
        QStringList results;

        // Show aircraft type only when no specific icon exists (generic
        // triangle is used)
        switch(type())
        {
        case Aircraft:
        case Balloon:
        case Copter:
        case Drone:
        case Glider:
        case HangGlider:
        case Jet:
        case Paraglider:
            break; // icon conveys the type
        case Airship:
            results << tr("Airship");
            break;
        case Skydiver:
            results << tr("Skydiver");
            break;
        case StaticObstacle:
            results << tr("Static Obstacle");
            break;
        default:
            // Default: Show no type information, previously we showed
            // "Traffic".
            break;
        }

        if (!positionInfo().coordinate().isValid())
        {
            results << tr("Position unknown");
        }

        if (vDist().isFinite())
        {
            QString result;
            if (vDist() > Units::Distance::fromM(500))
            {
                result = u"++"_s;
            }
            else if (vDist() > Units::Distance::fromM(400))
            {
                result = u"+"_s;
            }
            else if  (vDist() < Units::Distance::fromM(-500))
            {
                result = u"--"_s;
            }
            else if  (vDist() < Units::Distance::fromM(-400))
            {
                result = u"-"_s;
            }
            else
            {
                result = GlobalObject::navigator()->aircraft().verticalDistanceToString(vDist(), true);
                auto climbRateMPS = m_positionInfo.value().verticalSpeed().toMPS();
                if (qIsFinite(climbRateMPS))
                {
                    if (climbRateMPS < -1.0)
                    {
                        result += QStringLiteral(" ↘");
                    }
                    if ((climbRateMPS >= -1.0) && (climbRateMPS <= +1.0))
                    {
                        result += QStringLiteral(" →");
                    }
                    if (climbRateMPS > 1.0) {
                        result += QStringLiteral(" ↗");
                    }
                }
            }
            results << result;
        }

        return results.join(u"<br>");
    });
}

Traffic::TrafficFactor_WithPosition::~TrafficFactor_WithPosition()
{
    // Break all bindings before destruction proceeds
    m_valid.takeBinding();
    m_icon.takeBinding();
    m_description.takeBinding();
}

void Traffic::TrafficFactor_WithPosition::updateExtrapolatedData()
{
    if (!m_valid.value())
    {
        m_extrapolatedCoordinate = QGeoCoordinate();
        m_extrapolatedTrueTrack = Units::Angle();
        m_uncertaintyRadius = Units::Distance::fromM(0);
        return;
    }
    if (!m_positionInfo.value().groundSpeed().isFinite() || !m_positionInfo.value().trueTrack().isFinite())
    {
        m_extrapolatedCoordinate = m_positionInfo.value().coordinate();
        m_extrapolatedTrueTrack = Units::Angle();
        m_uncertaintyRadius = Units::Distance::fromM(0);
        return;
    }

    auto secondsElapsed = static_cast<double>(m_positionInfo.value().timestamp().msecsTo(QDateTime::currentDateTimeUtc()))/1000.0;

    const QScopedPropertyUpdateGroup updateGroup;
    if (secondsElapsed < 30.0)
    {
        m_uncertaintyRadius = Units::Distance::fromM(0);
    }
    else
    {
        m_uncertaintyRadius = m_positionInfo.value().groundSpeed() * Units::Timespan::fromS(secondsElapsed);
    }
    auto horizontalDistanceInMeters = (double)m_positionInfo.value().groundSpeed().toMPS()*secondsElapsed;
    m_extrapolatedCoordinate = m_positionInfo.value().coordinate().atDistanceAndAzimuth(horizontalDistanceInMeters, m_positionInfo.value().trueTrack().toDEG(), 0);
    m_extrapolatedTrueTrack = m_positionInfo.value().trueTrack();
}
