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
import QtQuick.Shapes

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound


/* Textual view of the airspaces above one point, used by the dialog
 * WaypointDescription.
 *
 * One row per airspace: the color legend of its category, the name, and the two
 * bounds as published. Rows use the full width of the dialog and wrap their
 * text, so that nothing is ever cut - a name may carry a frequency to monitor.
 * The graphical counterpart of this view is AirspaceStack.
 */

ColumnLayout {
    id: airspaceList

    /*! \brief Airspaces to show
     *
     * Airspaces at one position, as delivered by
     * GeoMapProvider.airspacesAtPosition().
     */
    required property var airspaces

    // No spacing of its own: the rows used to be children of the dialog's own
    // ColumnLayout, so they are spaced the way that layout spaces everything
    // else.

    // One airspace: category legend, name, bounds. This is the row that the
    // dialog used before it also had a graphical view.
    component AirspaceListRow: GridLayout {
        id: row

        required property var airspace

        columns: 3
        rowSpacing: 0

        Item {
            id: legend

            Layout.preferredWidth: boundsLabel.font.pixelSize*3
            Layout.preferredHeight: boundsLabel.font.pixelSize*2.5
            Layout.rowSpan: 3
            Layout.alignment: Qt.AlignLeft

            Shape {
                anchors.fill: parent

                ShapePath {
                    strokeWidth: 2
                    fillColor: "transparent"
                    strokeColor: Global.airspaceOutlineColor(row.airspace.CAT)
                    strokeStyle: Global.airspaceOutlineIsSolid(row.airspace.CAT) ? ShapePath.SolidLine : ShapePath.DashLine
                    dashPattern: Global.airspaceDashPattern(row.airspace.CAT)

                    startX: 1; startY: 1
                    PathLine { x: 1;              y: legend.height-1 }
                    PathLine { x: legend.width-1; y: legend.height-1 }
                    PathLine { x: legend.width-1; y: 1 }
                    PathLine { x: 1;              y: 1 }
                }
            }

            Rectangle {
                width: legend.width
                height: legend.height

                border.color: Global.airspaceBandColor(row.airspace.CAT)
                border.width: 6
                color: Global.airspaceFillColor(row.airspace.CAT)

                Label {
                    anchors.centerIn: parent
                    text: row.airspace.CAT
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.rowSpan: 3
            Layout.alignment: Qt.AlignVCenter

            text: row.airspace.name
            wrapMode: Text.WordWrap
        }

        Label {
            id: boundsLabel

            Layout.alignment: Qt.AlignHCenter|Qt.AlignBottom

            text: {
                switch(Navigator.aircraft.verticalDistanceUnit) {
                case Aircraft.Feet:
                    return row.airspace.upperBound
                case Aircraft.Meters:
                    return row.airspace.upperBoundMetric
                }
            }
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 1
            Layout.preferredWidth: boundsLabel.font.pixelSize*5

            color: boundsLabel.color
        }

        Label {
            Layout.alignment: Qt.AlignHCenter|Qt.AlignTop

            text: {
                switch(Navigator.aircraft.verticalDistanceUnit) {
                case Aircraft.Feet:
                    return row.airspace.lowerBound
                case Aircraft.Meters:
                    return row.airspace.lowerBoundMetric
                }
            }
            wrapMode: Text.WordWrap
        }
    }

    Repeater {
        model: airspaceList.airspaces

        AirspaceListRow {
            id: listRow

            required property var modelData

            airspace: listRow.modelData

            Layout.fillWidth: true
        }
    }
}
