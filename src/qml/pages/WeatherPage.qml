/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

import enroute 1.0
import "../dialogs"
import "../items"

/* TODO

  - Sort entries by distance to current position

  - Give one-line weather description, including time ("23 minutes ago")

  - Perhaps give sunset information in top line

  - Unify/coordinate behaviour with DataManager

  - Handle error messages

  - Give feedback on downloading, for interactivlely triggered updates
 */

Page {
    id: pg
    title: qsTr("Weather")

    header: ToolBar {

        Material.foreground: "white"
        height: 60

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
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

            anchors.verticalCenter: parent.verticalCenter

            anchors.right: parent.right

            icon.source: "/icons/material/ic_more_vert.svg"
            icon.color: "white"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Update METAR/TAF data")
                    enabled: (!global.weatherDataProvider().downloading) && (global.settings().acceptedWeatherTerms)
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        if (!global.weatherDataProvider().downloading)
                            global.weatherDataProvider().update(false)
                    }
                } // MenuItem

                MenuItem {
                    text: qsTr("Disallow internet connection")
                    enabled: global.settings().acceptedWeatherTerms
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        global.settings().acceptedWeatherTerms = false
                    }
                } // MenuItem

            }

        }

    }

    Component {
        id: stationDelegate

        Item {
            width: stationList.width
            height: idel.height

            // Background color according to METAR/FAA flight category
            Rectangle {
                anchors.fill: parent
                color: model.modelData.hasMETAR ? model.modelData.metar.flightCategoryColor : "transparent"
                opacity: 0.2
            }

            WordWrappingItemDelegate {
                id: idel
                text: {
                    var result = model.modelData.twoLineTitle

                    var wayTo  = global.navigator().aircraft.describeWay(global.positionProvider().positionInfo.coordinate(), model.modelData.coordinate)
                    if (wayTo !== "")
                        result = result + "<br>" + wayTo

                    if (model.modelData.hasMETAR)
                        result = result + "<br>" + model.modelData.metar.summary

                    return result
                }
                icon.source: model.modelData.icon
                icon.color: "transparent"

                width: parent.width

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    weatherReport.weatherStation = model.modelData
                    weatherReport.open()
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        visible: global.settings().acceptedWeatherTerms

        // List of weather stations
        ListView {
            id: stationList
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true

            model: global.weatherDataProvider().weatherStations
            delegate: stationDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Rectangle {
                anchors.fill: parent
                color: "white"
                visible: stationList.count == 0

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top

                    leftPadding: view.font.pixelSize
                    rightPadding: view.font.pixelSize
                    topPadding: 2*view.font.pixelSize

                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No METAR/TAF data available. You can restart the download manually using the item 'Update METAR/TAF' from the three-dot menu at the top right corner of the screen.</p>")
                }
            }

            // Refresh METAR/TAF data on overscroll
            property int refreshFlick: 0
            onFlickStarted: {
                refreshFlick = atYBeginning
            }

            onFlickEnded: {
                if ( atYBeginning && refreshFlick ) {
                    global.mobileAdaptor().vibrateBrief()
                    global.weatherDataProvider().update(false)
                }
            }

        }
    }

    Rectangle {
        id: downloadIndicator

        anchors.fill: parent

        color: "white"
        visible: global.weatherDataProvider().downloading && !global.weatherDataProvider().backgroundUpdate

        Text {
            id: downloadIndicatorLabel

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: view.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.StyledText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download METAR/TAF data from the Aviation Weather Center…</p>")
        } // downloadIndicatorLabel

        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: downloadIndicatorLabel.bottom
            anchors.topMargin: 10
        }

        // The Connections and the SequentialAnimation here provide a fade-out animation for the downloadindicator.
        // Without this, the downaloadIndication would not be visible on very quick downloads, leaving the user
        // without any feedback if the download did actually take place.
        Connections {
            target: global.weatherDataProvider()
            function onDownloadingChanged () {
                if (global.weatherDataProvider().downloading && !global.weatherDataProvider().backgroundUpdate) {
                    downloadIndicator.visible = true
                    downloadIndicator.opacity = 1.0
                } else
                    fadeOut.start()
            }
        }
        SequentialAnimation{
            id: fadeOut
            NumberAnimation { target: downloadIndicator; property: "opacity"; to:1.0; duration: 400 }
            NumberAnimation { target: downloadIndicator; property: "opacity"; to:0.0; duration: 400 }
            NumberAnimation { target: downloadIndicator; property: "visible"; to:1.0; duration: 20}
        }
    }

    ScrollView { // Privacy Warning
        anchors.fill: parent
        clip: true
        visible: !global.settings().acceptedWeatherTerms

        Item {
            width: parent.width
            implicitHeight: t1.height+t2.height

            Label {
                id: t1
                width: parent.width
                text: global.librarian().getStringFromRessource(":text/weatherPermissions.html")
                leftPadding: view.font.pixelSize
                rightPadding: view.font.pixelSize
                topPadding: 2*view.font.pixelSize
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Button {
                id: t2
                anchors.top: t1.bottom
                anchors.horizontalCenter: t1.horizontalCenter

                text: qsTr("Allow internet connection")
                Layout.alignment: Qt.AlignHCenter

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    global.settings().acceptedWeatherTerms = true
                    global.weatherDataProvider().update()
                }
            }

        }
    }

    // Manual update button in footer
    footer: Pane {
        width: parent.width

        Material.elevation: 3
        visible: (sunLabel.text != "") || (qnhLabel.text != "")

        GridLayout {
            anchors.fill: parent
            columns: 2

            Icon {
                visible: qnhLabel.text != ""
                source: "/icons/material/ic_speed.svg"
            }
            Label {
                id: qnhLabel
                visible: qnhLabel.text != ""
                Layout.fillWidth: true
                text: global.weatherDataProvider().QNHInfo
            }

            Icon {
                visible: sunLabel.text != ""
                source: "/icons/material/ic_wb_sunny.svg"
            }
            Label {
                id: sunLabel
                visible: sunLabel.text != ""
                Layout.fillWidth: true
                text: global.weatherDataProvider().sunInfo
            }

        }

    }

    // Try and update METAR/TAF as soon as someone opens this page if the current list of stations
    // is empty. This is not a background update, we want user interaction.
    Component.onCompleted: {
        if (stationList.count == 0)
            global.weatherDataProvider().update(false)
        else
            global.weatherDataProvider().update(true)
    }

    // Show error when weather cannot be updated -- but not if we are running a background upate
    Connections {
        target: global.weatherDataProvider()
        function onError (message) {
            if (global.weatherDataProvider().backgroundUpdate)
                return
            dialogLoader.active = false
            dialogLoader.title = qsTr("Update Error")
            dialogLoader.text = qsTr("<p>Failed to update the list of stations.</p><p>Reason: %1.</p>").arg(message)
            dialogLoader.source = "../dialogs/ErrorDialog.qml"
            dialogLoader.active = true
        }
    }

    WeatherReport {
        id: weatherReport
        objectName: "weatherReport"
    }

} // Page
