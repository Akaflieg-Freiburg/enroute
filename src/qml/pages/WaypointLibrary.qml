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

import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Waypoint Library")
    focus: true


    header: StandardHeader {}

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: view.font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: view.font.pixelSize*2.0

        placeholderText: qsTr("Filter Waypoint Names")
        font.pixelSize: view.font.pixelSize*1.5
    }

    Component {
        id: waypointDelegate

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true
            height: iDel.heigt

            SwipeToDeleteDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData.name
                icon.source: modelData.icon

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    waypointDescription.waypoint = modelData
                    waypointDescription.open()
                }

                swipe.onCompleted: {
                    global.mobileAdaptor().vibrateBrief()
                    removeDialog.waypoint = modelData
                    removeDialog.open()
                }

            }

            ToolButton {
                id: editButton

                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    wpEditor.waypoint = modelData
                    wpEditor.open()
                }
            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: removeAction
                        text: qsTr("Remove …")
                        onTriggered: {
                            global.mobileAdaptor().vibrateBrief()
                            removeDialog.waypoint = modelData
                            removeDialog.open()
                        }
                    } // removeAction
                } // AutoSizingMenu

            } // ToolButton

        }

    }

    ListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        clip: true

        model: {
            // Mention waypoints to ensure that the list gets updated
            global.waypointLibrary().waypoints

            return global.waypointLibrary().filteredWaypoints(textInput.text)
        }
        delegate: waypointDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: wpList
        anchors.topMargin: view.font.pixelSize*2

        visible: (wpList.count === 0)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        leftPadding: view.font.pixelSize*2
        rightPadding: view.font.pixelSize*2

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

    Dialog {
        id: removeDialog

        property var waypoint: global.geoMapProvider().createWaypoint()

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Remove from device?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(parent.height-view.font.pixelSize, implicitHeight)

        Label {
            width: removeDialog.availableWidth

            text: qsTr("Once the waypoint <strong>%1</strong> is removed, it cannot be restored.").arg(removeDialog.waypoint.name)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            global.waypointLibrary().remove(removeDialog.waypoint)
            page.reloadWaypointList()
            toast.doToast(qsTr("Waypoint removed from device"))
        }
        onRejected: {
            global.mobileAdaptor().vibrateBrief()
            page.reloadWaypointList() // Re-display aircraft that have been swiped out
            close()
        }

    }


    WaypointEditor {
        id: wpEditor

        onAccepted: {
            var newWP = waypoint.renamed(newName)
            newWP = newWP.relocated( QtPositioning.coordinate(newLatitude, newLongitude) )
            global.waypointLibrary().replace(waypoint, newWP)
            page.reloadWaypointList()
            toast.doToast(qsTr("Waypoint modified"))
        }

    }


    WaypointDescription {
        id: waypointDescription
    }

} // Page
