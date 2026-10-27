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

/* Menu to add a waypoint to, or remove it from, the flight route. Emits
   routeChanged() after an action has modified the route. */

AutoSizingMenu {
    id: routeMenu

    property waypoint waypoint

    signal routeChanged()

    // The route can change while the menu is closed. The state only matters
    // while the menu is open, so evaluate it on opening rather than tracking
    // every change.
    onAboutToShow: {
        appendToRouteAction.enabled = Navigator.flightRoute.canAppend(routeMenu.waypoint)
        insertIntoRouteAction.enabled = Navigator.flightRoute.canInsert(routeMenu.waypoint)
        removeFromRouteAction.enabled = Navigator.flightRoute.contains(routeMenu.waypoint)
    }

    Action {
        text: qsTr("Direct")
        enabled: PositionProvider.receivingPositionInfo && (Global.textDialogLoader.text !== "noRouteButton")

        onTriggered: {
            PlatformAdaptor.vibrateBrief()
            Navigator.flightRoute.directTo(routeMenu.waypoint, PositionProvider.positionInfo)
            Global.toast.doToast(qsTr("New flight route: direct to %1.").arg(routeMenu.waypoint.extendedName))
            routeMenu.close()
            routeMenu.routeChanged()
        }
    }

    Rectangle {
        height: 1
        Layout.fillWidth: true
        color: Global.airspaceNeutral
    }

    Action {
        id: appendToRouteAction

        text: qsTr("Append")

        onTriggered: {
            PlatformAdaptor.vibrateBrief()
            Navigator.flightRoute.append(routeMenu.waypoint)
            routeMenu.close()
            routeMenu.routeChanged()
            Global.toast.doToast(qsTr("Added %1 to route.").arg(routeMenu.waypoint.extendedName))
        }
    }

    Action {
        id: insertIntoRouteAction

        text: qsTr("Insert")

        onTriggered: {
            PlatformAdaptor.vibrateBrief()
            Navigator.flightRoute.insert(routeMenu.waypoint)
            routeMenu.close()
            routeMenu.routeChanged()
            Global.toast.doToast(qsTr("Inserted %1 into route.").arg(routeMenu.waypoint.extendedName))
        }
    }

    Action {
        id: removeFromRouteAction

        text: qsTr("Remove")

        onTriggered: {
            PlatformAdaptor.vibrateBrief()
            var index = Navigator.flightRoute.lastIndexOf(routeMenu.waypoint)
            if (index < 0)
                return
            Navigator.flightRoute.removeWaypoint(index)
            routeMenu.close()
            routeMenu.routeChanged()
            Global.toast.doToast(qsTr("Removed %1 from route.").arg(routeMenu.waypoint.extendedName))
        }
    }
}
