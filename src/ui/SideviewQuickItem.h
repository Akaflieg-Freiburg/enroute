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

#include <QQuickItem>


namespace Ui {


class SideviewQuickItem : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SideviewQuickItem)

public:
    explicit SideviewQuickItem(QQuickItem *parent = nullptr);

    ~SideviewQuickItem() override = default;

    //
    // Properties
    //

    /*! \brief Airspace polygons
     *
     * This property holds polygons that can be used with the QML class "Shape"
     * in order to draw a lateral view of the airspace situation. The size and
     * position of the polygons is adjusted to the current size of the QML item.
     * The polygons are sorted by drawing style. The following keys are used:
     *
     * - "A"   … for airspaces with CAT equal to A, B, C or D.
     * - "CTR" … for airspaces with CAT equal to CTR.
     * - "NRA" … for airspaces with CAT equal to NRA.
     * - "PJE" … for airspaces with CAT equal to PJE.
     * - "R"   … for airspaces with CAT equal to R, P or DNG.
     * - "RMZ" … for airspaces with CAT equal to RMZ, TIA or TIZ.
     * - "TMZ" … for airspaces with CAT equal to TMZ.
     */
    Q_PROPERTY(QVariantMap airspaces READ airspaces BINDABLE bindableAirspaces)

    /*! \brief Error string
     *
     * If no sideview can be shown (or if it can be shown only partially), this
     * property holds a human-readable, translated error message that should be
     * shown prominently on top of the sideview, in order to hide partially
     * drawn data underneath.
     */
    Q_PROPERTY(QString error READ error BINDABLE bindableError)

    /*! \brief 5-Minute-Bar
     *
     * This property holds the x- and y-extension of the 5-Minute-Bar, in pixel
     * coordinates of the this QQuickItem.
     */
    Q_PROPERTY(QPointF fiveMinuteBar READ fiveMinuteBar BINDABLE bindableFiveMinuteBar)

    /*! \brief Position of the own aircraft
     *
     * This property holds the position of the own aircraft, in pixel
     * coordinates of the this QQuickItem.
     */
    Q_PROPERTY(QPointF ownshipPosition READ ownshipPosition BINDABLE bindableOwnshipPosition)

    /*! \brief Map scale
     *
     * This property defines the scale of the side view. Copy the datum from a
     * flightmap in order to synchronize the scales.
     */
    Q_PROPERTY(double pixelPer10km READ pixelPer10km WRITE setPixelPer10km BINDABLE bindablePixelPer10km REQUIRED)

    /*! \brief Track string
     *
     * If the own aircraft is not moving sufficiently fast, this property holds
     * a human-readable, translated string of the form "Direction → 210°".
     * Otherwise, this property holds an empty string.
     */
    Q_PROPERTY(QString track READ track BINDABLE bindableTrack)


    QString error() {return m_error.value();}
    QBindable<QString> bindableError() {return &m_error;}

    QString track() {return m_track.value();}
    QBindable<QString> bindableTrack() {return &m_track;}

    Q_PROPERTY(QPolygonF terrain READ terrain BINDABLE bindableTerrain)
    QPolygonF terrain() {return m_terrain.value();}
    QBindable<QPolygonF> bindableTerrain() {return &m_terrain;}

    QVariantMap airspaces() {return m_airspaces.value();}
    QBindable<QVariantMap> bindableAirspaces() {return &m_airspaces;}

    QPointF fiveMinuteBar() {return m_fiveMinuteBar.value();}
    QBindable<QPointF> bindableFiveMinuteBar() {return &m_fiveMinuteBar;}

    QPointF ownshipPosition() {return m_ownshipPosition.value();}
    QBindable<QPointF> bindableOwnshipPosition() {return &m_ownshipPosition;}

    double pixelPer10km() {return m_pixelPer10km.value();}
    QBindable<double> bindablePixelPer10km() {return &m_pixelPer10km;}
    void setPixelPer10km(double newVal) {m_pixelPer10km = newVal;}

    Q_INVOKABLE void updateProperties();

private:
    Q_DISABLE_COPY_MOVE(SideviewQuickItem)

    QProperty<QString> m_error;

    QProperty<QString> m_track;

    QProperty<double> m_pixelPer10km;

    QProperty<QPointF> m_ownshipPosition;

    QProperty<QPointF> m_fiveMinuteBar;

    QProperty<QPolygonF> m_terrain;

    QProperty<QVariantMap> m_airspaces;

    std::vector<QPropertyNotifier> notifiers;
};

} // namespace Ui
