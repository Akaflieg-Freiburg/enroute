/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import akaflieg_freiburg.enroute
import enroute 1.0

import "../items"

/* This is a dialog with detailed information about a waypoint. To use this dialog, all you have to do is to set a Waypoint in the property "waypoint" and call open(). */

CenteringDialog {
    id: waypointDescriptionDialog

    property var waypoint: global.geoMapProvider().createWaypoint()
    property var weatherStation: global.weatherDataProvider().findWeatherStation( waypoint.ICAOCode )

    onWaypointChanged : {
        // Delete old text items
        co.children = {}

        // If no waypoint is given, then do nothing
        if (!waypoint.isValid)
            return

        // Create METAR info box
        metarInfo.createObject(co);

        // Create waypoint description items
        var pro = waypoint.tabularDescription
        for (var j in pro)
            waypointPropertyDelegate.createObject(co, {text: pro[j]});

        // Create airspace description items
        var asl = global.geoMapProvider().airspaces(waypoint.coordinate)
        for (var i in asl)
            airspaceDelegate.createObject(co, {airspace: asl[i]});
    }

    modal: true
    standardButtons: Dialog.Cancel
    focus: true

    Component {
        id: metarInfo

        Label { // METAR info
            visible: (weatherStation !== null) && (weatherStation.hasMETAR || weatherStation.hasTAF)
            text: {
                if (weatherStation === null)
                    return ""
                if (weatherStation.hasMETAR)
                    return weatherStation.metar.summary + " • <a href='xx'>" + qsTr("full report") + "</a>"
                return "<a href='xx'>" + qsTr("read TAF") + "</a>"
            }
            Layout.fillWidth: true
            wrapMode: Text.WordWrap

            bottomPadding: 0.2*view.font.pixelSize
            topPadding: 0.2*view.font.pixelSize
            leftPadding: 0.2*view.font.pixelSize
            rightPadding: 0.2*view.font.pixelSize
            onLinkActivated: {
                global.platformAdaptor().vibrateBrief()
                weatherReport.open()
            }

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: "black"
                color: ((weatherStation !== null) && weatherStation.hasMETAR) ? weatherStation.metar.flightCategoryColor : "transparent"
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
                Layout.preferredWidth: view.font.pixelSize*3
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

            property var airspace: ({});


            Item {
                id: box

                Layout.preferredWidth: view.font.pixelSize*3
                Layout.preferredHeight: view.font.pixelSize*2.5
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
                            case "E":
                            case "F":
                            case "G":
                                return "blue";
                            case "CTR":
                                return "blue";
                            case "GLD":
                                return "yellow";
                            case "DNG":
                            case "P":
                            case "PJE":
                            case "R":
                                return "red";
                            case "ATZ":
                            case "RMZ":
                            case "TIZ":
                            case "TIA":
                                return "blue";
                            case "TMZ":
                                return "black";
                            case "FIR":
                            case "FIS":
                            case "NRA":
                                return "green";
                            case "SUA":
                                return "red";
                            }
                            return "transparent"
                        }
                        strokeStyle:  {
                            switch(airspace.CAT) {
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
                            switch(airspace.CAT) {
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
                        case "ATZ":
                        case "RMZ":
                        case "TIZ":
                        case "TIA":
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
                        case "GLD":
                            return "#40ffff00";
                        case "ATZ":
                        case "RMZ":
                        case "TIZ":
                        case "TIA":
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
                text: {
                    switch(global.navigator().aircraft.verticalDistanceUnit) {
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
                color: Material.foreground
                height: 1
                width: view.font.pixelSize*5
            }
            Label {
                Layout.alignment: Qt.AlignHCenter|Qt.AlignTop
                text: {
                    switch(global.navigator().aircraft.verticalDistanceUnit) {
                    case Aircraft.Feet:
                        return gridLYO.airspace.lowerBound
                    case Aircraft.Meters:
                        return gridLYO.airspace.lowerBoundMetric
                    }
                }
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
                source: waypoint.icon
            }

            Label {
                text: waypoint.extendedName
                font.bold: true
                font.pixelSize: 1.2*view.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: global.navigator().aircraft.describeWay(global.positionProvider().positionInfo.coordinate(), waypoint.coordinate)
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

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
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
            text: qsTr("Route")

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                addMenu.open()
            }

            Menu {
                id: addMenu

                Action {
                    text: qsTr("Direct")
                    enabled: global.positionProvider().receivingPositionInfo && (dialogLoader.text !== "noRouteButton")

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        if (global.navigator().flightRoute.size > 0)
                            overwriteDialog.open()
                        else {
                            global.navigator().flightRoute.clear()
                            global.navigator().flightRoute.append(global.positionProvider().lastValidCoordinate)
                            global.navigator().flightRoute.append(waypoint)
                            toast.doToast(qsTr("New flight route: direct to %1.").arg(waypoint.extendedName))
                        }
                        close()
                    }

                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                }

                Action {
                    text: qsTr("Append")
                    enabled: {
                        // Mention Object to ensure that property gets updated
                        // when flight route changes
                        global.navigator().flightRoute.size

                        return global.navigator().flightRoute.canAppend(waypoint)
                    }

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        global.navigator().flightRoute.append(waypoint)
                        close()
                        toast.doToast(qsTr("Added %1 to route.").arg(waypoint.extendedName))
                    }
                }

                Action {
                    text: qsTr("Insert")
                    enabled: {
                        // Mention Object to ensure that property gets updated
                        // when flight route changes
                        global.navigator().flightRoute.size

                        return global.navigator().flightRoute.canInsert(waypoint)
                    }

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        global.navigator().flightRoute.insert(waypoint)
                        close()
                        toast.doToast(qsTr("Inserted %1 into route.").arg(waypoint.extendedName))
                    }
                }

                Action {
                    text: qsTr("Remove")

                    enabled:  {
                        // Mention to ensure that property gets updated
                        // when flight route changes
                        global.navigator().flightRoute.size

                        return global.navigator().flightRoute.contains(waypoint)
                    }
                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        close()
                        var index = global.navigator().flightRoute.lastIndexOf(waypoint)
                        if (index < 0)
                            return
                        global.navigator().flightRoute.removeWaypoint(index)
                        toast.doToast(qsTr("Removed %1 from route.").arg(waypoint.extendedName))
                    }
                }
            }

        }

        ToolButton {
            text: qsTr("Library")
            enabled: waypoint.category === "WP"

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                libraryMenu.open()
            }

            Menu {
                id: libraryMenu

                Action {
                    text: qsTr("Add…")
                    enabled: !global.waypointLibrary().hasNearbyEntry(waypoint)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        wpAdd.waypoint = waypoint
                        wpAdd.open()
                        close()
                    }
                }

                Action {
                    text: qsTr("Remove…")
                    enabled: global.waypointLibrary().contains(waypoint)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        removeDialog.waypoint = waypoint
                        removeDialog.open()
                        close()
                    }
                }                

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                }

                Action {
                    text: qsTr("Edit…")
                    enabled: global.waypointLibrary().contains(waypoint)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        wpEdit.waypoint = waypoint
                        wpEdit.open()
                        close()
                    }
                }

            }

        }

        onRejected: close()
    }

    CenteringDialog { // WARNING   qrc:/akaflieg_freiburg/enroute/qml/dialogs/CenteringDialog.qml:31: TypeError: Cannot read property 'width' of null
        id: overwriteDialog

        title: qsTr("Overwrite Current Flight Route?")

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            global.navigator().flightRoute.clear()
            global.navigator().flightRoute.append(waypoint)
            close()
            toast.doToast(qsTr("New flight route: direct to %1.").arg(waypoint.extendedName))
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
            waypointDescriptionDialog.open()
        }
    }

    WeatherReport {
        id: weatherReport
        weatherStation: waypointDescriptionDialog.weatherStation
    }

    WaypointEditor {
        id: wpEdit

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            var newWP = waypoint.renamed(newName)
            newWP = newWP.relocated( QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter) )
            global.waypointLibrary().replace(waypoint, newWP)
            toast.doToast(qsTr("Modified entry %1 in library.").arg(newWP.extendedName))
        }
    }

    WaypointEditor {
        id: wpAdd

        title: qsTr("Add Waypoint to Library")

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            var newWP = waypoint.renamed(newName)
            newWP = newWP.relocated( QtPositioning.coordinate(newLatitude, newLongitude) )
            global.waypointLibrary().add(newWP)
            toast.doToast(qsTr("Added %1 to waypoint library.").arg(newWP.extendedName))
        }
    }

    CenteringDialog {
        id: removeDialog

        property var waypoint: global.geoMapProvider().createWaypoint()

        title: qsTr("Remove from Device?")

        Label {
            width: removeDialog.availableWidth

            text: qsTr("Once the waypoint <strong>%1</strong> is removed, it cannot be restored.").arg(removeDialog.waypoint.name)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            global.waypointLibrary().remove(removeDialog.waypoint)
            toast.doToast(qsTr("Waypoint removed from device"))
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
        }

    }


} // Dialog
