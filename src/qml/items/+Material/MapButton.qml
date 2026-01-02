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
import QtQuick.Controls.Material
import QtQuick.Effects

import akaflieg_freiburg.enroute


Button {
    id: control

    width: 4*font.pixelSize
    height: 4*font.pixelSize
    implicitWidth: width
    implicitHeight: height
    padding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: Math.round(Math.max(6, 0.75*font.pixelSize))
    bottomPadding: 0
    spacing: 0

    icon.height: 2*font.pixelSize
    icon.width: 2*font.pixelSize
    icon.color: control.palette.buttonText
    display: AbstractButton.IconOnly

    Material.background: GlobalSettings.nightMode ? undefined : "white"

    background: Rectangle {
        property color baseColor: control.Material.background !== undefined ? control.Material.background : control.palette.button

        radius: 0
        width: control.width
        height: control.height
        color: control.down ? Qt.darker(baseColor, 1.08) : baseColor

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowOpacity: 0.25
            shadowBlur: 0.35
            shadowVerticalOffset: 1
        }
    }
}
