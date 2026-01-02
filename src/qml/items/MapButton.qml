/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Button {
    id: control

    width: 3*font.pixelSize
    height: 3*font.pixelSize
    padding: 0
    topPadding: Math.round(Math.max(6, 0.75*font.pixelSize))
    bottomPadding: 0
    spacing: 0

    visible: enabled

    icon.height: 2*font.pixelSize
    icon.width: 2*font.pixelSize
    icon.color: control.palette.buttonText
    display: AbstractButton.IconOnly

    background: Rectangle {
        anchors.fill: control
        color: control.down ? Qt.darker(control.palette.button, 1.08) : control.palette.button

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowOpacity: 0.25
            shadowBlur: 0.35
            shadowVerticalOffset: 1
        }
    }
}
