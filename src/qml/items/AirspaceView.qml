/***************************************************************************
 *   Copyright (C) 2026 by Simon Schneider, Stefan Kebekus                 *
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


/* Airspace section of the dialog WaypointDescription.
 *
 * Two views of the same airspaces, behind a tab bar: the list with the full
 * information, and the diagram that shows how the airspaces stack up. The list
 * comes first and is the one shown initially, because it is the view in which
 * no text can ever be cut.
 *
 * The views are shown and hidden rather than put into a SwipeView: this section
 * lives inside the scrolling column of the dialog, where a swipe view would
 * need its height bound to the current page and its gesture would compete with
 * the vertical scroll. Qt Quick Layouts skip invisible items, so the section is
 * always exactly as tall as the view that is on show.
 */

ColumnLayout {
    id: airspaceView

    /*! \brief Airspaces to show
     *
     * Airspaces at one position, as delivered by
     * GeoMapProvider.airspacesAtPosition().
     */
    required property var airspaces

    TabBar {
        id: bar

        Layout.fillWidth: true

        TabButton {
            text: qsTr("List")
            onClicked: PlatformAdaptor.vibrateBrief()
        }

        TabButton {
            text: qsTr("Diagram")
            onClicked: PlatformAdaptor.vibrateBrief()
        }
    }

    AirspaceList {
        Layout.fillWidth: true

        visible: bar.currentIndex === 0
        airspaces: airspaceView.airspaces
    }

    AirspaceStack {
        Layout.fillWidth: true

        visible: bar.currentIndex === 1
        airspaces: airspaceView.airspaces
    }
}
