/***************************************************************************
 *   Copyright (C) 2022-2023 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute
import enroute 1.0

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Waypoint Library")
    focus: true


    header: ToolBar {

        Material.foreground: "white"
        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                MenuItem {
                    text: qsTr("Import…")
                    enabled: Qt.platform.os !== "android"
                    visible: Qt.platform.os !== "android"
                    height: enabled ? undefined : 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        FileExchange.importContent()
                    }
                }

                AutoSizingMenu {
                    title: Qt.platform.os === "android" ? qsTr("Share…") : qsTr("Export…")
                    enabled: WaypointLibrary.waypoints.length > 0

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = FileExchange.shareContent(WaypointLibrary.toGeoJSON(), "application/geo+json", qsTr("Waypoint Library"))
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Waypoint library shared"))
                            else
                                toast.doToast(qsTr("Waypoint library exported"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… to GPX file")
                        onTriggered: {
                            headerMenuX.close()
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = FileExchange.shareContent(WaypointLibrary.toGpx(), "application/gpx+xml", qsTr("Waypoint Library"))
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Waypoint library shared"))
                            else
                                toast.doToast(qsTr("Waypoint library exported"))
                        }
                    }
                }

                AutoSizingMenu {
                    title: qsTr("Open in Other App…")
                    enabled: WaypointLibrary.waypoints.length > 0

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = FileExchange.viewContent(WaypointLibrary.toGeoJSON(), "application/geo+json", "WaypointLibrary-%1.geojson")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Waypoint library opened in other app"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… in GPX format")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = FileExchange.viewContent(WaypointLibrary.toGpx(), "application/gpx+xml", "WaypointLibrary-%1.gpx")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Waypoint library opened in other app"))
                        }
                    }

                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Clear")
                    enabled: WaypointLibrary.waypoints.length > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }

                }

            }
        }
    }

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: font.pixelSize*2.0

        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        placeholderText: qsTr("Filter Waypoint Names")
        font.pixelSize: page.font.pixelSize*1.5
    }

    Component {
        id: waypointDelegate

        RowLayout {
            width: wpList.width
            height: iDel.height

            SwipeToDeleteDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData.name
                icon.source: modelData.icon

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    waypointDescription.waypoint = modelData
                    waypointDescription.open()
                }

                swipe.onCompleted: {
                    PlatformAdaptor.vibrateBrief()
                    removeDialog.waypoint = modelData
                    removeDialog.open()
                }

            }

            ToolButton {
                id: editButton

                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    wpEditor.waypoint = modelData
                    wpEditor.open()
                }
            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: removeAction
                        text: qsTr("Remove…")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            removeDialog.waypoint = modelData
                            removeDialog.open()
                        }
                    } // removeAction
                } // AutoSizingMenu

            }

        }

    }

    ListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        leftMargin: SafeInsets.left
        rightMargin: SafeInsets.right
        bottomMargin: SafeInsets.bottom

        clip: true

        model: {
            // Mention waypoints to ensure that the list gets updated
            WaypointLibrary.waypoints

            return WaypointLibrary.filteredWaypoints(textInput.text)
        }
        delegate: waypointDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: wpList
        anchors.topMargin: font.pixelSize*2

        visible: (wpList.count === 0)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        leftPadding: font.pixelSize*2
        rightPadding: font.pixelSize*2

        textFormat: Text.RichText
        wrapMode: Text.Wrap
        text: (textInput.text === "")
              ? qsTr("<h3>Sorry!</h3><p>No waypoint available. To add a waypoint here, double-tap on a point in the moving map.</p>")
              : qsTr("<h3>Sorry!</h3><p>No waypoints match your filter criteria.</p>")
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function reloadWaypointList() {
        var cache = textInput.text
        textInput.text = textInput.text+"XXXXX"
        textInput.text = cache
    }

    CenteringDialog {
        id: shareErrorDialog

        title: qsTr("Error Exporting Data…")
        standardButtons: Dialog.Ok
        modal: true

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }
    }

    CenteringDialog {
        id: removeDialog

        property var waypoint: GeoMapProvider.createWaypoint()

        title: qsTr("Remove from Device?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: removeDialog.availableWidth

            text: qsTr("Once the waypoint <strong>%1</strong> is removed, it cannot be restored.").arg(removeDialog.waypoint.name)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            WaypointLibrary.remove(removeDialog.waypoint)
            page.reloadWaypointList()
            toast.doToast(qsTr("Waypoint removed from device"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            page.reloadWaypointList() // Re-display aircraft that have been swiped out
            close()
        }

    }

    CenteringDialog {
        id: clearDialog

        title: qsTr("Clear Waypoint Library?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: clearDialog.availableWidth

            text: qsTr("Once cleared, the library cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            WaypointLibrary.clear()
            page.reloadWaypointList()
            toast.doToast(qsTr("Waypoint library cleared"))
        }
    }

    WaypointEditor {
        id: wpEditor

        onAccepted: {
            let newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            WaypointLibrary.replace(waypoint, newWP)
            page.reloadWaypointList()
            toast.doToast(qsTr("Waypoint modified"))
        }

    }

    WaypointDescription {
        id: waypointDescription
    }

} // Page
