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
import QtQuick.Shapes

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound

/* Airspace category symbol, name and vertical bounds, styled like the moving map. */

GridLayout {
    id: gridLYO

    columns: 3
    rowSpacing: 0

    property var airspace: ({})

    // Airspace legend colors, kept in sync with the moving map through the
    // Global singleton so day/night styling matches the map exactly.
    readonly property color asBlue:   Global.airspaceBlue
    readonly property color asRed:    Global.airspaceRed
    readonly property color asGreen:  Global.airspaceGreen
    readonly property color asYellow: Global.airspaceYellow


    Item {
        id: box

        Layout.preferredWidth: colorGlean.font.pixelSize*3
        Layout.preferredHeight: colorGlean.font.pixelSize*2.5
        Layout.rowSpan: 3
        Layout.alignment: Qt.AlignLeft

        Shape {
            anchors.fill: parent

            ShapePath {
                strokeWidth: 2
                fillColor: "transparent"
                strokeColor:  {
                    switch(gridLYO.airspace.CAT) {
                    case "A":
                    case "B":
                    case "C":
                    case "D":
                    case "E":
                    case "F":
                    case "G":
                        return Global.airspaceBlue;
                    case "CTR":
                        return Global.airspaceBlue;
                    case "GLD":
                        return Global.airspaceYellow;
                    case "DNG":
                    case "P":
                    case "PJE":
                    case "R":
                        return Global.airspaceRed;
                    case "ATZ":
                    case "RMZ":
                    case "TIZ":
                    case "TIA":
                        return Global.airspaceBlue;
                    case "TMZ":
                        return Global.airspaceNeutral;
                    case "FIR":
                    case "FIS":
                    case "NRA":
                        return Global.airspaceGreen;
                    case "SUA":
                        return Global.airspaceRed;
                    }
                    return "transparent"
                }
                strokeStyle:  {
                    switch(gridLYO.airspace.CAT) {
                    case "A":
                    case "B":
                    case "C":
                    case "D":
                    case "E":
                    case "F":
                    case "G":
                    case "GLD":
                    case "NRA":
                        return ShapePath.SolidLine;
                    }
                    return ShapePath.DashLine
                }
                dashPattern:  {
                    switch(gridLYO.airspace.CAT) {
                    case "TMZ":
                        return [4, 2, 1, 2];
                    case "FIR":
                    case "FIS":
                        return [4, 0]
                    }
                    return [4, 4]
                }

                startX: 1; startY: 1
                PathLine { x: 1;           y: box.height-1 }
                PathLine { x: box.width-1; y: box.height-1 }
                PathLine { x: box.width-1; y: 1 }
                PathLine { x: 1;           y: 1 }
            }
        }

        Rectangle {
            width: box.width
            height: box.height

            border.color: {
                switch(gridLYO.airspace.CAT) {
                case "A":
                case "B":
                case "C":
                case "D":
                    return Qt.rgba(gridLYO.asBlue.r, gridLYO.asBlue.g, gridLYO.asBlue.b, 0.25);
                case "DNG":
                case "P":
                case "R":
                    return Qt.rgba(gridLYO.asRed.r, gridLYO.asRed.g, gridLYO.asRed.b, 0.25);
                case "ATZ":
                case "RMZ":
                case "TIZ":
                case "TIA":
                    return Qt.rgba(gridLYO.asBlue.r, gridLYO.asBlue.g, gridLYO.asBlue.b, 0.25);
                case "NRA":
                    return Qt.rgba(gridLYO.asGreen.r, gridLYO.asGreen.g, gridLYO.asGreen.b, 0.25);
                }
                return "transparent"
            }
            border.width: 6

            color: {
                switch(gridLYO.airspace.CAT) {
                case "CTR":
                    return Qt.rgba(gridLYO.asRed.r, gridLYO.asRed.g, gridLYO.asRed.b, 0.25);
                case "GLD":
                    return Qt.rgba(gridLYO.asYellow.r, gridLYO.asYellow.g, gridLYO.asYellow.b, 0.25);
                case "ATZ":
                case "RMZ":
                case "TIZ":
                case "TIA":
                    return Qt.rgba(gridLYO.asBlue.r, gridLYO.asBlue.g, gridLYO.asBlue.b, 0.25);
                }
                return "transparent"
            }

            Label {
                anchors.centerIn: parent
                text: gridLYO.airspace.CAT
            }

        }

    }

    Label {
        Layout.fillWidth: true
        Layout.rowSpan: 3
        Layout.alignment: Qt.AlignVCenter
        text: gridLYO.airspace.name
        wrapMode: Text.WordWrap
    }

    Label {
        id: colorGlean
        Layout.alignment: Qt.AlignHCenter|Qt.AlignBottom
        text: {
            switch(Navigator.aircraft.verticalDistanceUnit) {
            case Aircraft.Feet:
                return gridLYO.airspace.upperBound
            case Aircraft.Meters:
                return gridLYO.airspace.upperBoundMetric
            }
        }
        wrapMode: Text.WordWrap
    }

    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        color:  colorGlean.color
        Layout.preferredHeight: 1
        Layout.preferredWidth: colorGlean.font.pixelSize*5
    }

    Label {
        Layout.alignment: Qt.AlignHCenter|Qt.AlignTop
        text: {
            switch(Navigator.aircraft.verticalDistanceUnit) {
            case Aircraft.Feet:
                return gridLYO.airspace.lowerBound
            case Aircraft.Meters:
                return gridLYO.airspace.lowerBoundMetric
            }
        }
        wrapMode: Text.WordWrap
    }
}
