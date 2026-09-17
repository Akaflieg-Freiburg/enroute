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

import QtPositioning
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Templates as T
import QtQuick.Layouts

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound


/* This is a dialog with detailed information about a waypoint. To use this dialog, all you have to do is to set a Waypoint in the property "waypoint" and call open(). */

CenteringDialog {
    id: waypointDescriptionDialog

    property waypoint waypoint

    onWaypointChanged : {
        WeatherDataProvider.requestUpdate4Waypoint(waypoint)

        // Delete old text items
        let childCount = co.children.length;
        // Iterate through the children in reverse order
        var i
        for (i = childCount - 1; i >= 0; i--) {
            // Check if the child is a valid QML item
            if (co.children[i] instanceof QtObject) {
                    // Destroy the child item
                    co.children[i].destroy();
                }
            }

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

        // Create airspace view, with its list and diagram tabs
        var asl = GeoMapProvider.airspacesAtPosition(waypoint.coordinate)
        if (asl.length > 0)
            airspaceViewDelegate.createObject(co, {airspaces: asl});

        // Create VAC items
        var vac = VACLibrary.vacs4Point(waypoint.coordinate)
        for (i in vac)
            vacButtonDelegate.createObject(co, {vac: vac[i]});

        satButtonDelegate.createObject(co, {});
    }

    modal: true
    standardButtons: Dialog.Close
    focus: true

    title: {
        if (waypoint.ICAOCode === "")
            return waypoint.extendedName
        return waypoint.ICAOCode + " • " +waypoint.extendedName
    }


    Component {
        id: metarInfo

        Label { // METAR info
            Loader {
                id: secondaryDlgLoader
                onLoaded: (item as T.Popup).open()
            }
            Observer {
                id: obs
                waypoint: waypointDescriptionDialog.waypoint
            }

            visible:  obs.metar.isValid || obs.taf.isValid
            text: {
                if (obs.metar.isValid)
                    return obs.metar.summary(Navigator.aircraft, Clock.time) + " • <a href='xx'>" + qsTr("full report") + "</a>"
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
                secondaryDlgLoader.setSource("../dialogs/MetarTafDialog.qml", {"weatherStation": obs})
            }

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: Global.airspaceNeutral
                color: obs.metar.flightCategoryColor
                opacity: 0.2
            }
        }
    }

    Component {
        id: notamInfo

        Label { // NOTAM info
            id: notamLabel

            Loader {
                // WARNING This does not really belong here.
                id: dlgLoader
                onLoaded: (item as T.Popup).open()
            }

            property notamList notamList: NOTAMProvider.notams(waypointDescriptionDialog.waypoint)

            // The NOTAM database updates in the background. Re-query it when
            // that happens.
            Connections {
                target: NOTAMProvider
                function onLastUpdateChanged() {
                    notamLabel.notamList = NOTAMProvider.notams(waypointDescriptionDialog.waypoint)
                }
            }

            visible: text !== ""
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
                dlgLoader.setSource("../dialogs/NotamListDialog.qml", {"notamList": notamList, "waypoint": waypointDescriptionDialog.waypoint})
            }

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: Global.airspaceNeutral
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
        id: airspaceViewDelegate

        AirspaceView {
            Layout.preferredWidth: sv.width
        }
    }

    Component {
        id: vacButtonDelegate

        RowLayout {
            id: vb

            Layout.preferredWidth: sv.width

            property vac vac

            Icon {
                Layout.preferredWidth: button.font.pixelSize*3
                Layout.alignment: Qt.AlignVCenter
                source: "/icons/material/ic_map.svg"
            }

            Button {
                id: button
                text: vb.vac.name
                flat: true
                Material.foreground: Global.linkColor
                Layout.alignment: Qt.AlignVCenter
                onPressed:  {
                    PlatformAdaptor.vibrateBrief()
                    Global.currentVAC = VACLibrary.materialize(vb.vac)
                    waypointDescriptionDialog.close()
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Component {
        id: satButtonDelegate

        RowLayout {
            Layout.preferredWidth: sv.width

            Icon {
                Layout.preferredWidth: button.font.pixelSize*3
                Layout.alignment: Qt.AlignVCenter
                source: "/icons/material/ic_open_in_browser.svg"
            }

            Button {
                id: button
                text: qsTr("Satellite View")
                flat: true
                Material.foreground: Global.linkColor
                Layout.alignment: Qt.AlignVCenter
                onPressed:  {
                    PlatformAdaptor.vibrateBrief()
                    if (GlobalSettings.alwaysOpenExternalWebsites === true)
                    {
                        PlatformAdaptor.openSatView(waypointDescriptionDialog.waypoint.coordinate)
                        return
                    }
                    privacyWarning.coordinate = waypointDescriptionDialog.waypoint.coordinate
                    privacyWarning.open()
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Label { // Second header line with distance and QUJ
            text: Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), waypointDescriptionDialog.waypoint.coordinate)
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
            }

        } // DecoratedScrollView

        Keys.onBackPressed: (event) => {
            event.accepted = true;
            waypointDescriptionDialog.close()
        }
    }


    footer: DialogButtonBox {

        Button {
            text: qsTr("Route")
            flat: true

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                addMenu.open()
            }

            AutoSizingMenu {
                id: addMenu

                // The route can change while the dialog is open. The state
                // only matters while the menu is open, so evaluate it on
                // opening rather than tracking every change.
                onAboutToShow: {
                    appendToRouteAction.enabled = Navigator.flightRoute.canAppend(waypointDescriptionDialog.waypoint)
                    insertIntoRouteAction.enabled = Navigator.flightRoute.canInsert(waypointDescriptionDialog.waypoint)
                    removeFromRouteAction.enabled = Navigator.flightRoute.contains(waypointDescriptionDialog.waypoint)
                }

                Action {
                    text: qsTr("Direct")
                    enabled: PositionProvider.receivingPositionInfo && (Global.textDialogLoader.text !== "noRouteButton")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.directTo(waypointDescriptionDialog.waypoint, PositionProvider.positionInfo)
                        Global.toast.doToast(qsTr("New flight route: direct to %1.").arg(waypointDescriptionDialog.waypoint.extendedName))
                        addMenu.close()
                        waypointDescriptionDialog.close()
                    }
                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Global.airspaceNeutral
                }

                Action {
                    id: appendToRouteAction

                    text: qsTr("Append")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.append(waypointDescriptionDialog.waypoint)
                        addMenu.close()
                        waypointDescriptionDialog.close()
                        Global.toast.doToast(qsTr("Added %1 to route.").arg(waypointDescriptionDialog.waypoint.extendedName))
                    }
                }

                Action {
                    id: insertIntoRouteAction

                    text: qsTr("Insert")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.insert(waypointDescriptionDialog.waypoint)
                        addMenu.close()
                        waypointDescriptionDialog.close()
                        Global.toast.doToast(qsTr("Inserted %1 into route.").arg(waypointDescriptionDialog.waypoint.extendedName))
                    }
                }

                Action {
                    id: removeFromRouteAction

                    text: qsTr("Remove")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()                        
                        var index = Navigator.flightRoute.lastIndexOf(waypointDescriptionDialog.waypoint)
                        if (index < 0)
                            return
                        Navigator.flightRoute.removeWaypoint(index)
                        addMenu.close()
                        waypointDescriptionDialog.close()
                        Global.toast.doToast(qsTr("Removed %1 from route.").arg(waypointDescriptionDialog.waypoint.extendedName))
                    }
                }
            }
        }

        Button {
            text: qsTr("Library")
            enabled: waypointDescriptionDialog.waypoint.category === "WP" //TODO: Warum kann ich keine nearby waypoints speichern?
            flat: true

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                libraryMenu.open()
            }

            AutoSizingMenu {
                id: libraryMenu

                // The library can change while the dialog is open. The state
                // only matters while the menu is open, so evaluate it on
                // opening rather than tracking every change.
                onAboutToShow: {
                    addToLibraryAction.enabled = !WaypointLibrary.hasNearbyEntry(waypointDescriptionDialog.waypoint)
                    removeFromLibraryAction.enabled = WaypointLibrary.contains(waypointDescriptionDialog.waypoint)
                    editInLibraryAction.enabled = WaypointLibrary.contains(waypointDescriptionDialog.waypoint)
                }

                Action {
                    id: addToLibraryAction

                    text: qsTr("Add…")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        wpAdd.waypoint = waypointDescriptionDialog.waypoint
                        wpAdd.open()
                        libraryMenu.close()
                    }
                }

                Action {
                    id: removeFromLibraryAction

                    text: qsTr("Remove…")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        removeDialog.waypoint = waypointDescriptionDialog.waypoint
                        removeDialog.open()
                        libraryMenu.close()
                    }
                }                

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Global.airspaceNeutral
                }

                Action {
                    id: editInLibraryAction

                    text: qsTr("Edit…")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        wpEdit.waypoint = waypointDescriptionDialog.waypoint
                        wpEdit.open()
                        libraryMenu.close()
                    }
                }

            }
        }

        onRejected: waypointDescriptionDialog.close()
    }


    CenteringDialog {
        id: privacyWarning

        property var coordinate

        modal: true

        title: qsTr("Privacy warning")

        ColumnLayout {
            anchors.fill: parent

            DecoratedScrollView{
                Layout.fillHeight: true
                Layout.fillWidth: true

                contentWidth: availableWidth // Disable horizontal scrolling

                clip: true

                Label {
                    id: lbl
                    text: "<p>"
                          + qsTr("In order to show a satellite view, <strong>Enroute Flight Navigation</strong> will ask your system to open Google Earth or Google Maps in an external web browser or a dedicated app.")
                          + " " + qsTr("The authors of <strong>Enroute Flight Navigation</strong> do not control Google Earth or Google Maps.")
                          + " " + qsTr("They do not know what data it collects or how that data is processed.")
                          + "</p>"
                          + "<p>"
                          + " " + qsTr("With the click on OK, you consent to opening Google Earth or Google Maps on your device.")
                          + " " + qsTr("Click OK only if you agree with the terms and privacy policies of that site.")
                          + "</p>"

                    width: privacyWarning.availableWidth
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
            }

            Item {
                Layout.preferredHeight: lbl.font.pixelSize
            }

            WordWrappingCheckDelegate {
                id: alwaysOpen

                Layout.fillWidth: true

                text: qsTr("Always open external web sites and apps, do not ask again")
                checked: GlobalSettings.alwaysOpenExternalWebsites
            }
        }

        standardButtons: Dialog.Cancel|Dialog.Ok

        onAccepted: {
            GlobalSettings.alwaysOpenExternalWebsites = alwaysOpen.checked
            PlatformAdaptor.openSatView(coordinate)
        }
    }


    WaypointEditor {
        id: wpEdit

        onAccepted: {
            var newWP = waypointDescriptionDialog.waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            WaypointLibrary.replace(waypointDescriptionDialog.waypoint, newWP)
            waypointDescriptionDialog.close()
            Global.toast.doToast(qsTr("Modified entry %1 in library.").arg(newWP.extendedName))
        }
    }

    WaypointEditor {
        id: wpAdd

        title: qsTr("Add Waypoint to Library")

        onAccepted: {
            var newWP = waypointDescriptionDialog.waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            WaypointLibrary.add(newWP)
            waypointDescriptionDialog.close()
            Global.toast.doToast(qsTr("Added %1 to waypoint library.").arg(newWP.extendedName))
        }
    }

    LongTextDialog {
        id: removeDialog

        property var waypoint: GeoMapProvider.createWaypoint()

        title: qsTr("Remove from Device?")
        text: qsTr("Once the waypoint <strong>%1</strong> is removed, it cannot be restored.").arg(removeDialog.waypoint.name)

        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            WaypointLibrary.remove(removeDialog.waypoint)
            waypointDescriptionDialog.close()
            Global.toast.doToast(qsTr("Waypoint removed from device"))
        }
        onRejected: {
            removeDialog.close()
        }
    }

} // Dialog
