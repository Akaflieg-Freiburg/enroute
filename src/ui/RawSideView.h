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

#include <QQuickItem>


namespace Ui {


class RawSideView : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RawSideView)

public:
    explicit RawSideView(QQuickItem *parent = nullptr);

    Q_PROPERTY(QString error READ error BINDABLE bindableError)
    QString error() {return m_error.value();}
    QBindable<QString> bindableError() {return &m_error;}

    Q_PROPERTY(QString track READ track BINDABLE bindableTrack)
    QString track() {return m_track.value();}
    QBindable<QString> bindableTrack() {return &m_track;}

    Q_PROPERTY(QPolygonF terrain READ terrain BINDABLE bindableTerrain)
    QPolygonF terrain() {return m_terrain.value();}
    QBindable<QPolygonF> bindableTerrain() {return &m_terrain;}

    Q_PROPERTY(QVector<QPolygonF> airspaces READ airspaces BINDABLE bindableAirspaces)
    QVector<QPolygonF> airspaces() {return m_airspaces.value();}
    QBindable<QVector<QPolygonF>> bindableAirspaces() {return &m_airspaces;}

    Q_PROPERTY(QPointF fiveMinuteBar READ fiveMinuteBar BINDABLE bindableFiveMinuteBar)
    QPointF fiveMinuteBar() {return m_fiveMinuteBar.value();}
    QBindable<QPointF> bindableFiveMinuteBar() {return &m_fiveMinuteBar;}

    Q_PROPERTY(QPointF ownshipPosition READ ownshipPosition BINDABLE bindableOwnshipPosition)
    QPointF ownshipPosition() {return m_ownshipPosition.value();}
    QBindable<QPointF> bindableOwnshipPosition() {return &m_ownshipPosition;}

    Q_PROPERTY(double pixelPer10km READ pixelPer10km WRITE setPixelPer10km BINDABLE bindablePixelPer10km)
    double pixelPer10km() {return m_pixelPer10km.value();}
    QBindable<double> bindablePixelPer10km() {return &m_pixelPer10km;}
    void setPixelPer10km(double newVal) {m_pixelPer10km = newVal;}

private:
    void updateProperties();

    QProperty<QString> m_error;
    QProperty<QString> m_track;

    QProperty<double> m_pixelPer10km;

    QProperty<QPointF> m_ownshipPosition;

    QProperty<QPointF> m_fiveMinuteBar;

    QProperty<QPolygonF> m_terrain;

    QProperty<QVector<QPolygonF>> m_airspaces;

    std::vector<QPropertyNotifier> notifiers;

    Q_DISABLE_COPY_MOVE(RawSideView)
};

} // namespace Ui
