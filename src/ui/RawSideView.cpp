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
#include <QtConcurrent>
#include <QMutex>
#include <QMutexLocker>

#include "GeoMapProvider.h"
#include "GlobalObject.h"
#include "PositionProvider.h"
#include "RawSideView.h"

Ui::RawSideView::RawSideView(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_terrain.setBinding([this]() {return computeTerrain();});

    notifiers.push_back(bindableHeight().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(bindableWidth().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePositionInfo().addNotifier([this]() {updateProperties();}));
    notifiers.push_back(m_pixelPer10km.addNotifier([this]() {updateProperties();}));
    updateProperties();
}


QPolygonF Ui::RawSideView::computeTerrain()
{
    if (height() < 20)
    {
        return {};
    }

    auto info = GlobalObject::positionProvider()->positionInfo();
    auto track = GlobalObject::positionProvider()->lastValidTT().toDEG();

    QPolygonF polygon;
    int x = 0;
    for(x = 0; x <= width()+10; x += 5)
    {
        auto dist = 10000.0*(x-0.2*width())/(m_pixelPer10km.value());
        auto position = info.coordinate().atDistanceAndAzimuth(dist, track);
        auto elevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(position).toFeet();

        auto y = height() * (1 - elevation/10000);

        polygon << QPointF(x, y);
    }

    polygon  << QPointF(x, height()+20) << QPointF(0, height()+20);
    return polygon;
}

void Ui::RawSideView::updateProperties()
{
    const QScopedPropertyUpdateGroup updateLock;

    auto positionInfo = GlobalObject::positionProvider()->positionInfo();

    m_fiveMinuteBar = {0, 0};
    m_ownshipPosition = {-100, -100};
    //m_terrain = QPolygonF();
    m_track = QString();
    m_error = QString();

    if (!positionInfo.isValid())
    {
        m_error = tr("No valid position data.");
        return;
    }
    auto track = positionInfo.trueTrack();
    if (!track.isFinite())
    {
        track = GlobalObject::positionProvider()->lastValidTT();
        m_track = u"→ %1°"_s.arg(qRound(track.toDEG()));
        return;
    }

    m_ownshipPosition = {width()*0.2, height() * (1 - positionInfo.trueAltitudeAMSL().toFeet()/10000)};
    m_fiveMinuteBar = {m_pixelPer10km.value()*(positionInfo.groundSpeed().toMPS()*5*60)/10000, -height()*positionInfo.verticalSpeed().toFPM()*5.0/10000.0};
    m_error = QString();
}
