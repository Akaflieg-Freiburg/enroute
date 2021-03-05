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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import enroute 1.0

import "../items"

/* This is a dialog with detailed information about a waypoint. To use this dialog, all you have to do is to set a Waypoint in the property "waypoint" and call open(). */

Dialog {
    id: waypointDescriptionDialog

    // Property waypoint, and code to handle waypoint changes
    property Waypoint waypoint

    onWaypointChanged: {
        // Delete old text items
        co.children = {}

        // If no waypoint is given, then do nothing
        if (waypoint === null)
            return

        // Create METAR info box
        metarInfo.createObject(co);

        // Create waypoint description items
        var pro = waypoint.tabularDescription
        for (var j in pro)
            waypointPropertyDelegate.createObject(co, {text: pro[j]});

        // Create airspace description items
        var asl = geoMapProvider.airspaces(waypoint.coordinate)
        for (var i in asl)
            airspaceDelegate.createObject(co, {airspace: asl[i]});
    }

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(Overlay.overlay.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: (parent.width-width)/2.0
    y: (parent.height-height)/2.0

    modal: true
    standardButtons: Dialog.Cancel
    focus: true

    Component {
        id: metarInfo

        Label { // METAR info
            visible: (waypoint !== null) && (waypoint.hasMETAR || waypoint.hasTAF)
            text: {
                if (waypoint === null)
                    return ""
                if (waypoint.hasMETAR)
                    return waypoint.weatherStation.metar.summary + " • <a href='xx'>" + qsTr("full report") + "</a>"
                return "<a href='xx'>" + qsTr("read TAF") + "</a>"
            }
            Layout.fillWidth: true
            wrapMode: Text.WordWrap

            bottomPadding: 0.2*Qt.application.font.pixelSize
            topPadding: 0.2*Qt.application.font.pixelSize
            leftPadding: 0.2*Qt.application.font.pixelSize
            rightPadding: 0.2*Qt.application.font.pixelSize
            onLinkActivated: {
                mobileAdaptor.vibrateBrief()
                weatherReport.open()
            }

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: "black"
                color: ((waypoint !== null) && waypoint.hasMETAR) ? waypoint.weatherStation.metar.flightCategoryColor : "transparent"
                opacity: 0.2
            }

        }

    }

    Component {
        id: waypointPropertyDelegate


        RowLayout {
            id: rowLYO

            Layout.preferredWidth: sv.width

            property var text: ({});

            Label {
                text: rowLYO.text.substring(0,4)
                Layout.preferredWidth: Qt.application.font.pixelSize*3
                Layout.alignment: Qt.AlignTop
                font.bold: true

            }
            Label {
                Layout.fillWidth: true
                text: rowLYO.text.substring(4)
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
            }

        }
    }

    Component {
        id: airspaceDelegate

        GridLayout {
            id: gridLYO

            columns: 3
            rowSpacing: 0

            Layout.preferredWidth: sv.width

            property Airspace airspace: ({});


            Item {
                id: box

                Layout.preferredWidth: Qt.application.font.pixelSize*3
                Layout.preferredHeight: Qt.application.font.pixelSize*2.5
                Layout.rowSpan: 3
                Layout.alignment: Qt.AlignLeft

                Shape {
                    anchors.fill: parent

                    ShapePath {
                        strokeWidth: 2
                        fillColor: "transparent"
                        strokeColor:  {
                            switch(airspace.CAT) {
                            case "A":
                            case "B":
                            case "C":
                            case "D":
                                return "blue";
                            case "CTR":
                                return "blue";
                            case "DNG":
                            case "P":
                            case "PJE":
                            case "R":
                                return "red";
                            case "RMZ":
                                return "blue";
                            case "TMZ":
                                return "black";
                            case "FIS":
                            case "NRA":
                                return "green";
                            }
                            return "transparent"
                        }
                        strokeStyle:  {
                            switch(airspace.CAT) {
                            case "A":
                            case "B":
                            case "C":
                            case "D":
                            case "NRA":
                                return ShapePath.SolidLine;
                            }
                            return ShapePath.DashLine
                        }
                        dashPattern:  {
                            switch(airspace.CAT) {
                            case "TMZ":
                                return [4, 2, 1, 2];
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
                        switch(airspace.CAT) {
                        case "A":
                        case "B":
                        case "C":
                        case "D":
                            return "#400000ff";
                        case "DNG":
                        case "P":
                        case "R":
                            return "#40ff0000";
                        case "RMZ":
                            return "#400000ff";
                        case "NRA":
                            return "#4000ff00";
                        }
                        return "transparent"
                    }
                    border.width: 6

                    color: {
                        switch(airspace.CAT) {
                        case "CTR":
                            return "#40ff0000";
                        case "RMZ":
                            return "#400000ff";
                        }
                        return "transparent"
                    }

                    Label {
                        anchors.centerIn: parent
                        text: airspace.CAT
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
                Layout.alignment: Qt.AlignHCenter|Qt.AlignBottom
                text: gridLYO.airspace.upperBound
                wrapMode: Text.WordWrap
            }
            Rectangle {
                color: Material.foreground
                height: 1
                width: Qt.application.font.pixelSize*5
            }
            Label {
                Layout.alignment: Qt.AlignHCenter|Qt.AlignTop
                text: gridLYO.airspace.lowerBound
                wrapMode: Text.WordWrap
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout { // Header with icon and name
            id: headX
            Layout.fillWidth: true

            Icon {
                source: (waypoint !== null) ? waypoint.icon : "/icons/waypoints/WP.svg"
            }

            Label {
                text: (waypoint !== null) ? waypoint.extendedName : ""
                font.bold: true
                font.pixelSize: 1.2*Qt.application.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: (waypoint !== null) ? waypoint.wayTo(satNav.coordinate, globalSettings.useMetricUnits) : ""
            visible: (text !== "")
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        ScrollView {
            id: sv

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentHeight: co.height
            contentWidth: waypointDescriptionDialog.availableWidth

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            clip: true

            ColumnLayout {
                id: co
                width: parent.width
            } // ColumnLayout

        } // ScrollView

        Keys.onBackPressed: {
            event.accepted = true;
            waypointDescriptionDialog.close()
        }
    }

    footer: DialogButtonBox {

        ToolButton {
            text: qsTr("Direct")
            icon.source: "/icons/material/ic_keyboard_tab.svg"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: (satNav.status === SatNav.OK) && (dialogLoader.text !== "noRouteButton")

            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (flightRoute.routeObjects.length > 0)
                    overwriteDialog.open()
                else {
                    console.log(flightRoute)
                    flightRoute.clear()
                    flightRoute.append(satNav.lastValidCoordinate)
                    flightRoute.append(waypoint)
                    toast.doToast(qsTr("New flight route: direct to %1.").arg(waypoint.extendedName))
                }
            }
        }

        ToolButton {
            enabled: {
                // Mention lastWaypointObject to ensure that property gets updated
                // when flight route changes
                flightRoute.lastWaypointObject

                return (waypoint !== null) && flightRoute.canAppend(waypoint)
            }
            icon.source: "/icons/material/ic_add_circle.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                flightRoute.append(waypoint)
                close()
                toast.doToast(qsTr("Added %1 to route.").arg(waypoint.extendedName))
            }
        }

        ToolButton {
            icon.source: "/icons/material/ic_remove_circle.svg"
            enabled:  {
                // Mention lastWaypointObject to ensure that property gets updated
                // when flight route changes
                flightRoute.lastWaypointObject

                return flightRoute.contains(waypoint)
            }
            onClicked: {
                mobileAdaptor.vibrateBrief()
                close()
                toast.doToast(qsTr("Removed %1 from route.").arg(waypoint.extendedName))
                flightRoute.removeWaypoint(waypoint)
            }
        }

        onRejected: close()
    }

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Overwrite current flight route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            flightRoute.clear()
            flightRoute.append(satNav.lastValidCoordinate)
            flightRoute.append(waypoint)
            close()
            toast.doToast(qsTr("New flight route: direct to %1.").arg(waypoint.extendedName))
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
            waypointDescriptionDialog.open()
        }

    }

    WeatherReport {
        id: weatherReport
        weatherStation: (waypoint !== null) ? waypoint.weatherStation : null
    }
} // Dialog
