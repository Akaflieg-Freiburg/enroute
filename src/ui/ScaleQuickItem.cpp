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
#include <cmath>

#include "Global.h"
#include "ScaleQuickItem.h"
#include "Settings.h"


Ui::ScaleQuickItem::ScaleQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{

    connect(Global::settings(), &Settings::useMetricUnitsChanged, this, &QQuickItem::update);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);

}


void Ui::ScaleQuickItem::paint(QPainter *painter)
{
    // Safety check. Continue only if data provided is sane
    if (_pixelPer10km < 20) {
        return;
    }

    // Pre-compute a few numbers that will be used when drawing
    auto _useMetricUnits = Settings::useMetricUnitsStatic();
    qreal pixelPerUnit        = _useMetricUnits ? _pixelPer10km * 0.1 : _pixelPer10km * 0.1852;
    qreal scaleSizeInUnit     = _vertical ? (height()-10.0)/pixelPerUnit : (width()-10.0)/pixelPerUnit;
    qreal ScaleUnitInUnit     = pow(10.0, floor(log10(scaleSizeInUnit)));
    int   sizeOfUnitInPix     = qRound(ScaleUnitInUnit*pixelPerUnit);
    qreal sizeOfScaleInUnit   = floor(scaleSizeInUnit/ScaleUnitInUnit)*ScaleUnitInUnit;
    int   sizeOfScaleInPix    = qRound(sizeOfScaleInUnit*pixelPerUnit);

    // Compute size of text. Set font to somewhat smaller than standard size.
    QFont font = painter->font();
    if (font.pointSizeF() > 0.0) {
        font.setPointSizeF(font.pointSizeF()*0.8);
    } else {
        font.setPixelSize(qRound(font.pixelSize()*0.8));
    }
    painter->setFont(font);
    QString text = QString(_useMetricUnits ? QString("%1 km") : QString("%1 nm")).arg(sizeOfScaleInUnit);
    int textWidth  = painter->fontMetrics().horizontalAdvance(text);
    int textHeight = painter->fontMetrics().height();

    // Draw only if width() or height() is large enough
    if (_vertical) {
        if (height() < textWidth*1.5) {
            return;
        }
    } else {
        if (width() < textWidth*1.5) {
            return;
        }
    }

    // Coordinates for the left/top point of the scale
    int baseX = _vertical ? 8 : qRound((width()-sizeOfScaleInPix)/2.0);
    int baseY = _vertical ? qRound((height()-sizeOfScaleInPix)/2.0) : qRound(height()) - 8 ;

    // Draw underlying white, slightly tranparent rectangle
    painter->fillRect(0, 0, static_cast<int>(width()), static_cast<int>(height()), QColor(0xff, 0xff, 0xff, 0xe0));

    // Draw scale
    if (_vertical) {
        painter->setPen(QPen(Qt::white, 2));
        painter->drawLine(baseX, baseY, baseX, baseY+sizeOfScaleInPix);
        painter->drawLine(baseX+3, baseY, baseX-3, baseY);
        painter->drawLine(baseX+3, baseY+sizeOfScaleInPix, baseX-3, baseY+sizeOfScaleInPix);
        for(int i=1; i*ScaleUnitInUnit<sizeOfScaleInUnit; i+= 1) {
            painter->drawLine(baseX, baseY + i*sizeOfUnitInPix, baseX-3, baseY + i*sizeOfUnitInPix);
        }

        painter->setPen(QPen(Qt::black, 1));
        painter->drawLine(baseX, baseY, baseX, baseY+sizeOfScaleInPix);
        painter->drawLine(baseX+3, baseY, baseX-3, baseY);
        painter->drawLine(baseX+3, baseY+sizeOfScaleInPix, baseX-3, baseY+sizeOfScaleInPix);
        for(int i=1; i*ScaleUnitInUnit<sizeOfScaleInUnit; i+= 1) {
            painter->drawLine(baseX, baseY + i*sizeOfUnitInPix, baseX-3, baseY + i*sizeOfUnitInPix);
        }

        // Draw text
        painter->rotate(-90.0);
        painter->drawText(-qRound(height()/2.0)-textWidth/2, baseX+textHeight, text);
    } else {
        painter->setPen(QPen(Qt::white, 2));
        painter->drawLine(baseX, baseY, baseX+sizeOfScaleInPix, baseY);
        painter->drawLine(baseX, baseY+3, baseX, baseY-3);
        painter->drawLine(baseX+sizeOfScaleInPix, baseY+3, baseX+sizeOfScaleInPix, baseY-3);
        for(int i=1; i*ScaleUnitInUnit<sizeOfScaleInUnit; i+= 1) {
            painter->drawLine(baseX + i*sizeOfUnitInPix, baseY, baseX + i*sizeOfUnitInPix, baseY+3);
        }

        painter->setPen(QPen(Qt::black, 1));
        painter->drawLine(baseX, baseY, baseX+sizeOfScaleInPix, baseY);
        painter->drawLine(baseX, baseY+3, baseX, baseY-3);
        painter->drawLine(baseX+sizeOfScaleInPix, baseY+3, baseX+sizeOfScaleInPix, baseY-3);
        for(int i=1; i*ScaleUnitInUnit<sizeOfScaleInUnit; i+= 1) {
            painter->drawLine(baseX + i*sizeOfUnitInPix, baseY, baseX + i*sizeOfUnitInPix, baseY+3);
        }

        // Draw text
        painter->drawText(baseX+sizeOfScaleInPix/2-textWidth/2, baseY-5, text);
    }


}


void Ui::ScaleQuickItem::setPixelPer10km(qreal _pxp10k)
{
    if (qFuzzyCompare(_pixelPer10km, _pxp10k)) {
        return;
    }

    _pixelPer10km = _pxp10k;
    update();
    emit pixelPer10kmChanged();
}


void Ui::ScaleQuickItem::setVertical(bool newVertical)
{
    if (_vertical == newVertical) {
        return;
    }

    _vertical = newVertical;
    update();
    emit verticalChanged();
}
