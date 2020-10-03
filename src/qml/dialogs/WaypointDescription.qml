/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import enroute 1.0

Dialog {
    id: dlg

    property var dialogArgs: undefined

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

    modal: true
    standardButtons: Dialog.Cancel
    focus: true

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
                textFormat: Text.RichText
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
                Layout.preferredHeight: Qt.application.font.pixelSize*3
                Layout.rowSpan: 3
                Layout.alignment: Qt.AlignLeft

                property var boxWidth: Qt.application.font.pixelSize*3
                property var boxHeight: Qt.application.font.pixelSize*2.5

                Shape {
                    anchors.fill: parent

                    ShapePath {
                        strokeWidth: 2
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
                            }
                            return "transparent"
                        }
                        strokeStyle:  {
                            switch(airspace.mapLayout) {
                            case "A":
                            case "B":
                            case "C":
                            case "D":
                                return ShapePath.SolidLine;
                            }
                            return ShapePath.DashLine
                        }
                        dashPattern:  {
                            switch(airspace.mapLayout) {
                            case "TMZ":
                                return [4, 2, 1, 2];
                            }
                            return [4, 4]
                        }

                        startX: 1; startY: 1
                        PathLine { x: 1;           y: box.boxHeight }
                        PathLine { x: box.boxWidth; y: box.boxHeight }
                        PathLine { x: box.boxWidth; y: 1 }
                        PathLine { x: 1;           y: 1 }
                    }
                }

                Rectangle {
                    width: box.boxWidth
                    height: box.boxHeight

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
                text: gridLYO.airspace.name
                wrapMode: Text.WordWrap
            }

            Label {
                Layout.alignment: Qt.AlignHCenter|Qt.AlignBottom
                text: gridLYO.airspace.upperBound
                wrapMode: Text.WordWrap
            }
            Rectangle {
                color: "black"
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

            Image {
                source: dialogArgs.waypoint.icon
                sourceSize.width: 25
            }

            Label {
                text: dialogArgs.waypoint.extendedName
                font.bold: true
                font.pixelSize: 1.2*Qt.application.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
            ToolButton {
                icon.source: "/icons/material/ic_bug_report.svg"

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    Qt.openUrlExternally(qsTr("mailto:stefan.kebekus@gmail.com?subject=Enroute, Error Report &body=Thank you for suggesting a correction in the map data. Please describe the issue here."))
                }
            }
        }

        Label { // Second header line with distance and QUJ
            text: dialogArgs.waypoint.wayTo
            visible: satNav.status === SatNav.OK
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        ScrollView {
            id: sv

            Layout.fillWidth: true
            Layout.fillHeight: true

            contentHeight: co.height
            contentWidth: dlg.availableWidth

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < co.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            ColumnLayout {
                id: co
                width: parent.width

                Label { // METAR info
                    visible: dialogArgs.waypoint.hasMETAR || dialogArgs.waypoint.hasTAF
                    text: {
                        if (dialogArgs.waypoint.hasMETAR)
                            return dialogArgs.waypoint.METARSummary + " • <a href='xx'>" + qsTr("full report") + "</a>"
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
                        color: dialogArgs.waypoint.flightCategoryColor
                        opacity: 0.2
                    }

                }

                Component.onCompleted: {
                    var pro = dialogLoader.dialogArgs.waypoint.tabularDescription
                    for (var j in pro)
                        waypointPropertyDelegate.createObject(co, {text: pro[j]});

                    var asl = geoMapProvider.airspaces(dialogLoader.dialogArgs.waypoint.coordinate)
                    for (var i in asl)
                        airspaceDelegate.createObject(co, {airspace: asl[i]});
                }
            } // ColumnLayout

        } // ScrollView

        Keys.onBackPressed: {
            event.accepted = true;
            dlg.close()
        }
    }

    footer: DialogButtonBox {

        Button {
            flat: true
            text: qsTr("Direct")
            icon.source: "/icons/material/ic_keyboard_tab.svg"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: (satNav.status === SatNav.OK) && (dialogLoader.text !== "noRouteButton")

            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (flightRoute.routeObjects.length > 0)
                    overwriteDialog.open()
                else {
                    flightRoute.clear()
                    flightRoute.append(satNav.lastValidCoordinate)
                    flightRoute.append(dialogArgs.waypoint)
                }
            }
        }

        Button {
            flat: true
            text: flightRoute.contains(dialogArgs.waypoint) ? qsTr("from Route") : qsTr("to Route")
            icon.source: flightRoute.contains(dialogArgs.waypoint) ? "/icons/material/ic_remove_circle.svg" : "/icons/material/ic_add_circle.svg"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (!flightRoute.contains(dialogArgs.waypoint)) {
                    flightRoute.append(dialogArgs.waypoint)
                } else {
                    flightRoute.removeWaypoint(dialogArgs.waypoint)
                }
            }
        }

        onRejected: close()
    } // DialogButtonBox

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Overwrite current flight route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            flightRoute.clear()
            flightRoute.append(satNav.lastValidCoordinate)
            flightRoute.append(dlg.dialogArgs.waypoint)
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

    } // overWriteDialog

    WeatherReport {
        id: weatherReport
        weatherStation: dialogArgs.waypoint.weatherStation
    }

} // Dialog
