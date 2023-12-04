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

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

CenteringDialog {
    id: dialogMain

    Component {
        id: firstStart

        DecoratedScrollView {
            id: sv

            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling
            contentHeight: lbl.contentHeight

            clip: true

            property string title: qsTr("Welcome!")

            Label {
                id: lbl
                text: "<p>" + qsTr("Thank you for using this flight navigation app!  Before we get started, we need to point out that <strong>this app and the aviation data come with no guarantees</strong>.")+"</p>"
                      + "<p>" + qsTr("The app is not certified to satisfy aviation standards. It may contain errors and may not work as expected.") + "</p>"
                      + "<p>" + qsTr("The aviation data does not come from official sources. It might be incomplete, outdated or otherwise incorrect.") + "</p>"
                      + "<p>" + qsTr("<strong>This app is no substitute for proper flight preparation or good pilotage.</strong> We hope you enjoy the app and that you do find it useful.") + "</p>"
                      + "<p>" + qsTr("Fly safely and enjoy many happy landings!") + "</p>"
                      + "<p>&#8212; Stefan Kebekus.</p>";

                width: sv.dialogMain.availableWidth

                textFormat: Text.RichText
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                GlobalSettings.acceptedTerms = 1
                GlobalSettings.lastWhatsNewHash = Librarian.getStringHashFromRessource(":text/whatsnew.html")
                GlobalSettings.lastWhatsNewInMapsHash = DataManager.whatsNewHash
            }
        }

    }

    Component {
        id: privacy

        DecoratedScrollView {
            id: sv

            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling
            contentHeight: lbl.contentHeight

            clip: true

            property string title: qsTr("Privacy")

            Label {
                id: lbl

                text: "<p>" + qsTr("Please take a minute to review our privacy policies.") + "</p>"
                      + Librarian.getStringFromRessource(":text/privacy.html")
                width: sv.dialogMain.availableWidth
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                GlobalSettings.privacyHash = Librarian.getStringHashFromRessource(":text/privacy.html")
            }
        }
    }

    Component {
        id: permissions

        DecoratedScrollView {
            id: sv

            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling
            contentHeight: lbl.contentHeight

            clip: true

            property string title: qsTr("Permissions")

            Label {
                id: lbl

                text: "<p>" + qsTr("Please grant the following permissions when prompted.") + "</p>"
                      + "<p>" + qsTr("Enroute Flight Navigation needs to access your precise location. The app uses this data to show your position on the moving map and to provide relevant aeronautical information.") + "</p>"
                width: sv.dialogMain.availableWidth
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                console.log("X")
                Global.locationPermission.request()
                PositionProvider.startUpdates()
            }
        }
    }

    Component {
        id: maps

        DecoratedScrollView {
            id: sv

            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling
            contentHeight: cl.implicitHeight

            clip: true

            property string title: qsTr("Download Maps")

            ColumnLayout {
                id: cl

                width: sv.dialogMain.availableWidth

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    text: {
                        var result = "<p>" + qsTr("<strong>Enroute Flight Navigation</strong> needs geographic maps to work.") + " "

                        if (Global.locationPermission.status === Qt.PermissionStatus.Granted)
                        {
                            if (DataManager.mapList.hasFile)
                            {
                                if (PositionProvider.positionInfo.isValid())
                                {
                                    if (lv.model.length === 0)
                                        result += qsTr("Regretfully, we do not offer maps for your present location (%1).").arg(PositionProvider.lastValidCoordinate)
                                    if (lv.model.length === 1)
                                        result += qsTr("Based on your location, we reckon that that the following map might be relevant for you. Click on the map to start the download, then click on 'Done' to close this dialog.")
                                    if (lv.model.length > 1)
                                        result += qsTr("Based on your location, we reckon that that the following maps might be relevant for you. Click on any map to start the download, then click on 'Done' to close this dialog.")
                                }
                                else
                                {
                                    result += qsTr("We're waiting for SatNav position infoformation to suggest maps that might be relevant for you. Please stand by.")
                                }
                            }
                            else
                            {
                                result += qsTr("We're downloading the list of available maps. Please stand by.")
                            }
                        }

                        if (Global.locationPermission.status === Qt.PermissionStatus.Denied)
                        {
                            result += "<strong>"+qsTr("Please grant location permissions, so we can suggest maps to download.")+"</strong>"
                        }

                        if (Global.locationPermission.status === Qt.PermissionStatus.Undetermined)
                        {
                            result += "<strong>"+qsTr("We're unable to suggest maps to download because the location permission was denied.")+"</strong>"
                        }

                        result += "</p>"

                        return result
                    }
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: "black"
                    visible: PositionProvider.receivingPositionInfo
                }

                DecoratedListView {
                    id: lv

                    Layout.preferredHeight: contentHeight
                    Layout.fillWidth: true

                    flickableDirection: Flickable.HorizontalFlick

                    visible: PositionProvider.receivingPositionInfo

                    clip: true
                    model: DataManager.mapSets.downloadables4Location(PositionProvider.lastValidCoordinate)
                    delegate: MapSet {}
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: "black"
                    visible: PositionProvider.receivingPositionInfo
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    text:  "<p>"
                           + qsTr("For the full list of maps, close this dialog, open the main menu and go to 'Library/Maps and Data'. It is also possible to import raster maps into this app. Check the manual for details.")
                           + "</p>"
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }

            }

            function accept() {
            }

            Component.onCompleted: Global.locationPermission.request()

        }
    }


    closePolicy: Popup.NoAutoClose
    modal: true

    title: stack.currentItem ? stack.currentItem.title : ""

    StackView {
        id: stack

        implicitHeight: empty ? 0 : currentItem.implicitHeight
        Behavior on implicitHeight {
            NumberAnimation { duration: 200 }
            enabled: dialogMain.opened
        }

        anchors.fill: parent
    }

    function conditionalOpen() {
        if (!DataManager.aviationMaps.hasFile)
            stack.push(maps, {"dialogMain": dialogMain, "objectName": "maps"})
        if (Global.locationPermission.status !== Qt.PermissionStatus.Granted)
        {
            console.log(Global.locationPermission.status)
    //        stack.push(permissions, {"dialogMain": dialogMain})
        }
        if (GlobalSettings.privacyHash !== Librarian.getStringHashFromRessource(":text/privacy.html"))
            stack.push(privacy, {"dialogMain": dialogMain})
        if (GlobalSettings.acceptedTerms === 0)
            stack.push(firstStart, {"dialogMain": dialogMain})

        if (stack.empty)
            Global.locationPermission.request()
        else
            open()
        return !stack.empty
    }

    footer: DialogButtonBox {
        ToolButton {
            text: {
                if (!stack.currentItem)
                    return ""
                if (stack.currentItem.objectName === "maps")
                    return qsTr("Done")
                return qsTr("Accept")
            }

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stack.currentItem.accept()
                if (stack.depth > 1)
                    stack.pop()
                else
                    dialogMain.close()
            }
        }
    }
}
