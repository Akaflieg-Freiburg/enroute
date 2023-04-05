/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

import QtPositioning
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Shapes

import akaflieg_freiburg.enroute
import enroute 1.0

import "../dialogs"
import "../items"

/* This is a dialog with detailed information about a waypoint. To use this dialog, all you have to do is to set a Waypoint in the property "waypoint" and call open(). */

CenteringDialog {
    id: waypointDescriptionDialog

    property var waypoint: GeoMapProvider.createWaypoint()
    property var weatherStation: WeatherDataProvider.findWeatherStation( waypoint.ICAOCode )

    onWaypointChanged : {
        // Delete old text items
        co.children = {}

        // If no waypoint is given, then do nothing
        if (!waypoint.isValid)
            return

        // Create METAR info box
        metarInfo.createObject(co);

        // Create NOTAM info box
        notamInfo.createObject(co);

        // Create waypoint description items
        var pro = waypoint.tabularDescription
        for (var j in pro)
            waypointPropertyDelegate.createObject(co, {text: pro[j]});

        // Create airspace description items
        var asl = GeoMapProvider.airspaces(waypoint.coordinate)
        for (var i in asl)
            airspaceDelegate.createObject(co, {airspace: asl[i]});
    }

    modal: true
    standardButtons: Dialog.Close
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

            bottomPadding: 0.2*font.pixelSize
            topPadding: 0.2*font.pixelSize
            leftPadding: 0.2*font.pixelSize
            rightPadding: 0.2*font.pixelSize
            onLinkActivated: {
                PlatformAdaptor.vibrateBrief()
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
        id: notamInfo

        Label { // NOTAM info

            Loader {
                // WARNING This does not really belong here.
                id: dlgLoader
            }

            property notamList notamList: {
                // Mention lastUpdate, so we update whenever there is new data
                NotamProvider.lastUpdate
                return NotamProvider.notams(waypoint)
            }

            visible: text != ""
            text: {
                if (notamList.isValid && notamList.isEmpty)
                    return ""
                if (notamList.isEmpty)
                    return notamList.summary
                return notamList.summary + " • <a href='xx'>" + qsTr("full report") + "</a>"
            }

            Layout.fillWidth: true
            wrapMode: Text.WordWrap

            bottomPadding: 0.2*font.pixelSize
            topPadding: 0.2*font.pixelSize
            leftPadding: 0.2*font.pixelSize
            rightPadding: 0.2*font.pixelSize
            onLinkActivated: {
                PlatformAdaptor.vibrateBrief()
                dlgLoader.setSource("../dialogs/NotamListDialog.qml",
                                    {"notamList": notamList, "waypoint": waypointDescriptionDialog.waypoint})
                dlgLoader.item.open()
            }

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: "black"
                color: "yellow"
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
                Layout.preferredWidth: font.pixelSize*3
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

                Layout.preferredWidth: font.pixelSize*3
                Layout.preferredHeight: font.pixelSize*2.5
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
                color: Material.foreground
                Layout.preferredHeight: 1
                Layout.preferredWidth: font.pixelSize*5
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
                font.pixelSize: 1.2*waypointDescriptionDialog.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), waypoint.coordinate)
            visible: (text !== "")
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        DecoratedScrollView {
            id: sv

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentHeight: co.height
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            ColumnLayout {
                id: co
                width: parent.width
            } // ColumnLayout

        } // DecoratedScrollView

        Keys.onBackPressed: {
            event.accepted = true;
            waypointDescriptionDialog.close()
        }
    }

    footer: DialogButtonBox {

        ToolButton {
            text: qsTr("Route")

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                addMenu.open()
            }

            AutoSizingMenu {
                id: addMenu

                Action {
                    text: qsTr("Direct")
                    enabled: PositionProvider.receivingPositionInfo && (dialogLoader.text !== "noRouteButton")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        if (Navigator.flightRoute.size > 0)
                            overwriteDialog.open()
                        else {
                            Navigator.flightRoute.clear()
                            Navigator.flightRoute.append(PositionProvider.lastValidCoordinate)
                            Navigator.flightRoute.append(waypoint)
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
                        Navigator.flightRoute.size

                        return Navigator.flightRoute.canAppend(waypoint)
                    }

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.append(waypoint)
                        close()
                        toast.doToast(qsTr("Added %1 to route.").arg(waypoint.extendedName))
                    }
                }

                Action {
                    text: qsTr("Insert")
                    enabled: {
                        // Mention Object to ensure that property gets updated
                        // when flight route changes
                        Navigator.flightRoute.size

                        return Navigator.flightRoute.canInsert(waypoint)
                    }

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.insert(waypoint)
                        close()
                        toast.doToast(qsTr("Inserted %1 into route.").arg(waypoint.extendedName))
                    }
                }

                Action {
                    text: qsTr("Remove")

                    enabled:  {
                        // Mention to ensure that property gets updated
                        // when flight route changes
                        Navigator.flightRoute.size

                        return Navigator.flightRoute.contains(waypoint)
                    }
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        close()
                        var index = Navigator.flightRoute.lastIndexOf(waypoint)
                        if (index < 0)
                            return
                        Navigator.flightRoute.removeWaypoint(index)
                        toast.doToast(qsTr("Removed %1 from route.").arg(waypoint.extendedName))
                    }
                }
            }

        }

        ToolButton {
            text: qsTr("Library")
            enabled: waypoint.category === "WP"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                libraryMenu.open()
            }

            AutoSizingMenu {
                id: libraryMenu

                Action {
                    text: qsTr("Add…")
                    enabled: !WaypointLibrary.hasNearbyEntry(waypoint)

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        wpAdd.waypoint = waypoint
                        wpAdd.open()
                        close()
                    }
                }

                Action {
                    text: qsTr("Remove…")
                    enabled: WaypointLibrary.contains(waypoint)

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
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
                    enabled: WaypointLibrary.contains(waypoint)

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        wpEdit.waypoint = waypoint
                        wpEdit.open()
                        close()
                    }
                }

            }

        }

        onRejected: close()
    }

    LongTextDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Current Flight Route?")
        text: qsTr("Once overwritten, the current flight route cannot be restored.")

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            Navigator.flightRoute.clear()
            Navigator.flightRoute.append(waypoint)
            close()
            toast.doToast(qsTr("New flight route: direct to %1.").arg(waypoint.extendedName))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
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
            PlatformAdaptor.vibrateBrief()
            var newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            WaypointLibrary.replace(waypoint, newWP)
            toast.doToast(qsTr("Modified entry %1 in library.").arg(newWP.extendedName))
        }
    }

    WaypointEditor {
        id: wpAdd

        title: qsTr("Add Waypoint to Library")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            var newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            WaypointLibrary.add(newWP)
            toast.doToast(qsTr("Added %1 to waypoint library.").arg(newWP.extendedName))
        }
    }

    LongTextDialog {
        id: removeDialog

        property var waypoint: GeoMapProvider.createWaypoint()

        title: qsTr("Remove from Device?")
        text: qsTr("Once the waypoint <strong>%1</strong> is removed, it cannot be restored.").arg(removeDialog.waypoint.name)

        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            WaypointLibrary.remove(removeDialog.waypoint)
            toast.doToast(qsTr("Waypoint removed from device"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }

    }


} // Dialog
