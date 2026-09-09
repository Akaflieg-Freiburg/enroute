/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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
                Global.locationPermission.request()
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

            // All map sets covering the current position. Accessing
            // DataManager.mapSets.downloadables makes this binding re-evaluate
            // when the list of available maps arrives after the position fix.
            readonly property var mapSets4Location: {
                if (DataManager.mapSets.downloadables.length === 0) // qmllint disable unresolved-type
                    return []
                return DataManager.mapSets.downloadables4Location(PositionProvider.lastValidCoordinate)
            }

            // Map sets that contain an aviation map. These are what the app needs
            // to work, and they are downloaded automatically.
            readonly property var recommendedMapSets: sv.mapSets4Location.filter(set => sv.hasAviationMap(set))

            // Remaining map sets covering the current position, e.g. raster charts
            // such as 'Switzerland ICAO'. These are offered, but not downloaded
            // automatically.
            readonly property var otherMapSets: sv.mapSets4Location.filter(set => !sv.hasAviationMap(set))

            // Ensures that the automatic download is triggered only once, so that
            // a download cancelled by the user is not restarted.
            property bool autoDownloadStarted: false

            // Map sets are assembled file by file while maps.json is parsed, and
            // recommendedMapSets changes on every step. Defer the download until
            // control returns to the event loop, when all sets are complete.
            onRecommendedMapSetsChanged: Qt.callLater(sv.startAutoDownload)

            function hasAviationMap(mapSet) : bool {
                const parts = mapSet.downloadables // qmllint disable unresolved-type
                for (let i = 0; i < parts.length; i++)
                    if (parts[i].contentType === Downloadable_Abstract.AviationMap)
                        return true
                return false
            }

            function startAutoDownload() {
                if (sv.autoDownloadStarted || (sv.recommendedMapSets.length === 0))
                    return
                sv.autoDownloadStarted = true
                for (let i = 0; i < sv.recommendedMapSets.length; i++) {
                    const mapSet = sv.recommendedMapSets[i]
                    if (!mapSet.hasFile && !mapSet.downloading)
                        mapSet.startDownload()
                }
            }

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
                                    if (sv.mapSets4Location.length === 0)
                                        result += qsTr("Regretfully, we do not offer maps for your present location (%1).").arg(PositionProvider.lastValidCoordinate)
                                    else if (sv.recommendedMapSets.length === 1)
                                        result += qsTr("Based on your location, we are downloading the following map for you. Click on 'Done' to close this dialog. The download continues in the background.")
                                    else if (sv.recommendedMapSets.length > 1)
                                        result += qsTr("Based on your location, we are downloading the following maps for you. Click on 'Done' to close this dialog. The download continues in the background.")
                                    else
                                        result += qsTr("Based on your location, we reckon that the following maps might be relevant for you. Click on any map to start the download, then click on 'Done' to close this dialog.")
                                }
                                else
                                {
                                    result += qsTr("We're waiting for SatNav position information to suggest maps that might be relevant for you. Please stand by.")
                                }
                            }
                            else
                            {
                                result += qsTr("We're downloading the list of available maps. Please stand by.")
                            }
                        }

                        if (Global.locationPermission.status === Qt.PermissionStatus.Undetermined)
                        {
                            result += "<strong>"+qsTr("Please grant location permissions, so we can suggest maps to download.")+"</strong>"
                        }

                        if (Global.locationPermission.status === Qt.PermissionStatus.Denied)
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
                    color: Global.dividerColor
                    visible: recommendedList.visible
                }

                DecoratedListView {
                    id: recommendedList

                    Layout.preferredHeight: contentHeight
                    Layout.fillWidth: true

                    flickableDirection: Flickable.HorizontalFlick

                    visible: PositionProvider.receivingPositionInfo && (sv.recommendedMapSets.length > 0)

                    clip: true
                    model: sv.recommendedMapSets
                    delegate: MapSet {}
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Global.dividerColor
                    visible: recommendedList.visible
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: implicitHeight
                    visible: otherList.visible && (sv.recommendedMapSets.length > 0)
                    text: "<p>"
                          + qsTr("The following additional maps are available for your region. They are not required. Click on a map to start the download.")
                          + "</p>"
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Global.dividerColor
                    visible: otherList.visible
                }

                DecoratedListView {
                    id: otherList

                    Layout.preferredHeight: contentHeight
                    Layout.fillWidth: true

                    flickableDirection: Flickable.HorizontalFlick

                    visible: PositionProvider.receivingPositionInfo && (sv.otherMapSets.length > 0)

                    clip: true
                    model: sv.otherMapSets
                    delegate: MapSet {}
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true
                    color: Global.dividerColor
                    visible: otherList.visible
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

            Component.onCompleted: {
                Global.locationPermission.request()
                Qt.callLater(sv.startAutoDownload)
            }

        }
    }


    closePolicy: Popup.NoAutoClose
    modal: true

    // The pages of the stack are plain items that carry a title and an accept()
    title: stack.currentItem ? stack.currentItem.title : "" // qmllint disable missing-property

    StackView {
        id: stack

        implicitHeight: empty ? 0 : currentItem.implicitHeight
        Behavior on implicitHeight {
            NumberAnimation { duration: 200 }
            enabled: dialogMain.opened
        }

        anchors.fill: parent
    }

    onClosed: {
        Global.locationPermission.request()
    }

    Component.onCompleted: {
        if (!DataManager.aviationMaps.hasFile)
            stack.push(maps, {"dialogMain": dialogMain, "objectName": "maps"})
        if (GlobalSettings.privacyHash !== Librarian.getStringHashFromRessource(":text/privacy.html"))
            stack.push(privacy, {"dialogMain": dialogMain})
        if (GlobalSettings.acceptedTerms === 0)
            stack.push(firstStart, {"dialogMain": dialogMain})
    }

    footer: DialogButtonBox {
        Button {
            flat: true

            text: {
                if (!stack.currentItem)
                    return ""
                if (stack.currentItem.objectName === "maps")
                    return qsTr("Done")
                return qsTr("Accept")
            }

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stack.currentItem.accept() // qmllint disable missing-property
                if (stack.depth > 1)
                    stack.pop()
                else
                    dialogMain.close()
            }
        }
    }
}
