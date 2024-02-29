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

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "SideViewQuickItem.h"
#include "navigation/Aircraft.h"
#include "navigation/Navigator.h"


Ui::SideViewQuickItem::SideViewQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{

    setRenderTarget(QQuickPaintedItem::FramebufferObject);

}


void Ui::SideViewQuickItem::paint(QPainter *painter)
{

    painter->fillRect(0, 0, static_cast<int>(width()), static_cast<int>(height()), QColor("lightblue"));
    painter->fillRect(0, static_cast<int>(height())*0.8, static_cast<int>(width()), static_cast<int>(height()), QColor("brown"));

}
