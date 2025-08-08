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

#include "RawSideView.h"

Ui::RawSideView::RawSideView(QQuickItem *parent)
    : QQuickItem(parent)
{
}

QPolygonF Ui::RawSideView::terrain()
{
    qWarning() << isVisible();
    QPolygonF polygon;
    polygon
        << QPointF(0, height()-20)
        << QPointF(100+width(), height()-20)
        << QPointF(100+width(), height()+20)
        << QPointF(0, height()+20);
    qWarning() << polygon;
    return polygon;
}
