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

#include <cmath>
#include <QPainter>

#include "ScaleQuickItem.h"


ScaleQuickItem::ScaleQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}


void ScaleQuickItem::paint(QPainter *painter)
{
    // Safety check. Continue only if data provided is sane
    if (_pixelPer10km < 20)
        return;

    // Pre-compute a few numbers that will be used when drawing
    qreal pixelPerUnit        = _useMetricUnits ? _pixelPer10km * 0.1 : _pixelPer10km * 0.1852;
    qreal widthInUnit         = (width()-10.0)/pixelPerUnit;
    qreal scaleUnitInUnit     = pow(10.0, floor(log10(widthInUnit)));
    int   widthOfUnitInPix    = qRound(scaleUnitInUnit*pixelPerUnit);
    qreal widthOfScaleInUnit  = floor(widthInUnit/scaleUnitInUnit)*scaleUnitInUnit;
    int   widthOfScaleInPix   = qRound(widthOfScaleInUnit*pixelPerUnit);

    // Compute size of text. Set font to somewhat smaller than standard size.
    QFont font = painter->font();
    if (font.pointSizeF() > 0.0)
        font.setPointSizeF(font.pointSizeF()*0.8);
    else
        font.setPixelSize(qRound(font.pixelSize()*0.8));
    painter->setFont(font);
    QString text = QString(_useMetricUnits ? "%1 km" : "%1 NM").arg(widthOfScaleInUnit);
    int textWidth = painter->fontMetrics().horizontalAdvance(text);

    // Draw only if width() is large enough
    if (width() < textWidth*1.5)
        return;


    int baseX = qRound((width()-widthOfScaleInPix)/2.0);
    int baseY = qRound(height()) - 8;

    // Draw underlying white, slightly tranparent rectangle
    painter->fillRect(0, 0, static_cast<int>(width()), static_cast<int>(height()), QColor(0xff, 0xff, 0xff, 0xe0));

    // Draw scale
    painter->setPen(QPen(Qt::white, 2));
    painter->drawLine(baseX, baseY, baseX+widthOfScaleInPix, baseY);
    painter->drawLine(baseX, baseY+3, baseX, baseY-3);
    painter->drawLine(baseX+widthOfScaleInPix, baseY+3, baseX+widthOfScaleInPix, baseY-3);
    for(int i=1; i*scaleUnitInUnit<widthOfScaleInUnit; i+= 1)
        painter->drawLine(baseX + i*widthOfUnitInPix, baseY, baseX + i*widthOfUnitInPix, baseY+3);

    painter->setPen(QPen(Qt::black, 1));
    painter->drawLine(baseX, baseY, baseX+widthOfScaleInPix, baseY);
    painter->drawLine(baseX, baseY+3, baseX, baseY-3);
    painter->drawLine(baseX+widthOfScaleInPix, baseY+3, baseX+widthOfScaleInPix, baseY-3);
    for(int i=1; i*scaleUnitInUnit<widthOfScaleInUnit; i+= 1)
        painter->drawLine(baseX + i*widthOfUnitInPix, baseY, baseX + i*widthOfUnitInPix, baseY+3);

    // Draw text
    painter->drawText(baseX+widthOfScaleInPix/2-textWidth/2, baseY-5, text);
}


void ScaleQuickItem::setPixelPer10km(qreal _pxp10k)
{
    if (qFuzzyCompare(_pixelPer10km, _pxp10k))
        return;

    _pixelPer10km = _pxp10k;
    update();
    emit pixelPer10kmChanged();
}


void ScaleQuickItem::setUseMetricUnits(bool useMetricUnits)
{
    if (_useMetricUnits == useMetricUnits)
        return;

    _useMetricUnits = useMetricUnits;
    update();
}
