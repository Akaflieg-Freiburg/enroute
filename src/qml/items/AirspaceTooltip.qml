/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound

/* Floating list of the airspaces at a map position, shown while the mouse
   rests over the moving map. Placed next to the cursor rather than under it,
   so that it does not capture the hover itself. */

Pane {
    id: airspaceTooltip

    property var airspaces: []
    property point cursor

    function showAt(newCursor : point, newAirspaces : var) {
        cursor = newCursor
        airspaces = newAirspaces
        visible = (airspaces.length > 0)
    }

    // Bindings rather than assignments in showAt(): the height is only known
    // once the layout has been polished
    readonly property int offset: 16
    x: Math.max(0, Math.min(cursor.x + offset, parent.width - width))
    // Flip above the cursor if there is no room below
    y: (cursor.y + offset + height <= parent.height) ? cursor.y + offset : Math.max(0, cursor.y - offset - height)

    visible: false
    width: Math.min(parent.width, font.pixelSize*24)
    Material.elevation: 6

    ColumnLayout {
        width: airspaceTooltip.availableWidth

        Repeater {
            model: airspaceTooltip.airspaces

            AirspaceDelegate {
                required property var modelData

                Layout.fillWidth: true
                airspace: modelData
            }
        }
    }
}
