/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#include <QPolygonF>
#include <QProperty>
#include <QQuickItem>
#include <QTimer>


namespace Ui {

/*! \brief QML base class for the vertical profile of the flight route
 *
 * This class is the base class for the QML type "RouteProfile". It shows terrain
 * and airspaces along the current flight route: the departure point is at the
 * left edge, the destination at the right edge, and the horizontal axis is
 * proportional to the distance along the route. The vertical axis reaches from
 * the lowest terrain along the route to the airspace altitude limit set in the
 * global settings, or to FL100 if no limit is set.
 */
class RouteProfileQuickItem : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RouteProfileQuickItem)

public:
    explicit RouteProfileQuickItem(QQuickItem *parent = nullptr);

    ~RouteProfileQuickItem() override = default;

    //
    // Properties
    //

    /*! \brief Ticks of the vertical scale
     *
     * List of maps, one per tick, with the keys "y" (vertical position in
     * pixels) and "label" (altitude in the vertical distance unit of the
     * current aircraft). The tick spacing is chosen so that adjacent ticks are
     * at least minimumTickSpacing pixels apart.
     */
    Q_PROPERTY(QVariantList altitudeTicks READ altitudeTicks BINDABLE bindableAltitudeTicks)

    /*! \brief Airspace polygons
     *
     * Polygons for the QML class "Shape", sorted by drawing style. The keys are
     * described in Ui::ProfileSampling::airspacePolygons().
     */
    Q_PROPERTY(QVariantMap airspaces READ airspaces BINDABLE bindableAirspaces)

    /*! \brief Error string
     *
     * If no profile can be shown (or only partially), this property holds a
     * human-readable, translated message. Otherwise, it is empty.
     */
    Q_PROPERTY(QString error READ error BINDABLE bindableError)

    /*! \brief Minimum distance between ticks of the vertical scale, in pixels */
    Q_PROPERTY(double minimumTickSpacing READ minimumTickSpacing WRITE setMinimumTickSpacing BINDABLE bindableMinimumTickSpacing)

    /*! \brief Terrain polygon
     *
     * Polygon for the QML class "Shape" that draws the terrain silhouette.
     */
    Q_PROPERTY(QPolygonF terrain READ terrain BINDABLE bindableTerrain)

    /*! \brief Waypoints of the flight route
     *
     * List of maps, one per waypoint, with the keys "x" (horizontal position in
     * pixels) and "label" (ICAO code, or empty if the waypoint has none).
     */
    Q_PROPERTY(QVariantList waypoints READ waypoints BINDABLE bindableWaypoints)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property altitudeTicks
     */
    [[nodiscard]] QVariantList altitudeTicks() const {return m_altitudeTicks.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property altitudeTicks
     */
    [[nodiscard]] QBindable<QVariantList> bindableAltitudeTicks() const {return &m_altitudeTicks;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property airspaces
     */
    [[nodiscard]] QVariantMap airspaces() const {return m_airspaces.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property airspaces
     */
    [[nodiscard]] QBindable<QVariantMap> bindableAirspaces() const {return &m_airspaces;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property error
     */
    [[nodiscard]] QString error() const {return m_error.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property error
     */
    [[nodiscard]] QBindable<QString> bindableError() const {return &m_error;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property minimumTickSpacing
     */
    [[nodiscard]] double minimumTickSpacing() const {return m_minimumTickSpacing.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property minimumTickSpacing
     */
    [[nodiscard]] QBindable<double> bindableMinimumTickSpacing() {return &m_minimumTickSpacing;}

    /*! \brief Setter method for property with the same name
     *
     *  @param newVal Property minimumTickSpacing
     */
    void setMinimumTickSpacing(double newVal) {m_minimumTickSpacing = newVal;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property terrain
     */
    [[nodiscard]] QPolygonF terrain() const {return m_terrain.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property terrain
     */
    [[nodiscard]] QBindable<QPolygonF> bindableTerrain() const {return &m_terrain;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property waypoints
     */
    [[nodiscard]] QVariantList waypoints() const {return m_waypoints.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property waypoints
     */
    [[nodiscard]] QBindable<QVariantList> bindableWaypoints() const {return &m_waypoints;}

private:
    Q_DISABLE_COPY_MOVE(RouteProfileQuickItem)

    // Several inputs often change at once (e.g. width and height), so updates
    // are collected and computed once on the next turn of the event loop.
    void scheduleUpdate() { m_updateTimer.start(); }
    QTimer m_updateTimer;

    void updateProperties();

    QProperty<QVariantList> m_altitudeTicks;
    QProperty<QVariantMap> m_airspaces;
    QProperty<double> m_minimumTickSpacing {40.0};
    QProperty<QString> m_error;
    QProperty<QPolygonF> m_terrain;
    QProperty<QVariantList> m_waypoints;

    std::vector<QPropertyNotifier> m_notifiers;
};

} // namespace Ui
