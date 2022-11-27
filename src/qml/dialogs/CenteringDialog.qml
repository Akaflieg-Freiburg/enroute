/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

import QtQuick.Controls

Dialog {

    leftMargin: global.platformAdaptor().safeInsetLeft + Qt.application.font.pixelSize
    rightMargin: global.platformAdaptor().safeInsetRight + Qt.application.font.pixelSize
    topMargin: global.platformAdaptor().safeInsetTop + Qt.application.font.pixelSize
    bottomMargin: global.platformAdaptor().safeInsetBottom + Qt.application.font.pixelSize

    // We center the dialog manually, taking care of safe insets
    x: global.platformAdaptor().safeInsetLeft + (Overlay.overlay.width-global.platformAdaptor().safeInsetLeft-global.platformAdaptor().safeInsetRight-width)/2.0
    y: global.platformAdaptor().safeInsetTop + (Overlay.overlay.height-global.platformAdaptor().safeInsetTop-global.platformAdaptor().safeInsetBottom-height)/2.0

    implicitWidth: 40*Qt.application.font.pixelSize

    parent: Overlay.overlay
    
} // Dialog
