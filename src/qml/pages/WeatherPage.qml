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

  - Unify/coordinate behaviour with MapManager

  - Handle error messages

  - Give feedback on downloading, for interactivlely triggered updates
 */

Page {
    id: pg
    title: qsTr("Weather")

    header: ToolBar {

        RowLayout {
            width: pg.width

            ToolButton {
                id: backButton

                icon.source: "/icons/material/ic_arrow_back.svg"
                icon.color: "white"
                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    if (stackView.depth > 1) {
                        stackView.pop()
                    } else {
                        drawer.open()
                    }
                }
            }

            Label {
                Layout.fillWidth: true

                text: stackView.currentItem.title
                color: "white"
                elide: Label.ElideRight
                font.bold: true
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
            }

            ToolButton {
                id: headerMenuToolButton

                icon.source: "/icons/material/ic_more_vert.svg"
                icon.color: "white"

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    headerMenuX.popup()
                }

                AutoSizingMenu{
                    id: headerMenuX

                    MenuItem {
                        text: qsTr("Update METAR/TAF data")
                        enabled: (!weatherDownloadManager.downloading) && (globalSettings.acceptedWeatherTerms)
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            if (!weatherDownloadManager.downloading)
                                weatherDownloadManager.update(false)
                        }
                    } // MenuItem

                    MenuItem {
                        text: qsTr("Disallow internet connection")
                        enabled: globalSettings.acceptedWeatherTerms
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            globalSettings.acceptedWeatherTerms = false
                        }
                    } // MenuItem

                }

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

                    var wayTo  = model.modelData.wayTo(satNav.positionInfo.coordinate(), globalSettings.useMetricUnits)
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
                    mobileAdaptor.vibrateBrief()
                    weatherReport.weatherStation = model.modelData
                    weatherReport.open()
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        visible: globalSettings.acceptedWeatherTerms

        // List of weather stations
        ListView {
            id: stationList
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true

            model: weatherDownloadManager.weatherStations
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

                    leftPadding: Qt.application.font.pixelSize
                    rightPadding: Qt.application.font.pixelSize
                    topPadding: 2*Qt.application.font.pixelSize

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
                    mobileAdaptor.vibrateBrief()
                    weatherDownloadManager.update(false)
                }
            }

        }
    } // ColumnLayout

    Rectangle {
        id: downloadIndicator

        anchors.fill: parent

        color: "white"
        visible: weatherDownloadManager.downloading && !weatherDownloadManager.backgroundUpdate

        Text {
            id: downloadIndicatorLabel

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

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
            target: weatherDownloadManager
            function onDownloadingChanged () {
                if (weatherDownloadManager.downloading && !weatherDownloadManager.backgroundUpdate) {
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
    } // downloadIndicator - Rectangle

    ScrollView { // Privacy Warning
        anchors.fill: parent
        clip: true
        visible: !globalSettings.acceptedWeatherTerms

        Item {
            width: parent.width
            implicitHeight: t1.height+t2.height

            Label {
                id: t1
                width: parent.width
                text: librarian.getStringFromRessource(":text/weatherPermissions.html")
                leftPadding: Qt.application.font.pixelSize
                rightPadding: Qt.application.font.pixelSize
                topPadding: 2*Qt.application.font.pixelSize
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
                    mobileAdaptor.vibrateBrief()
                    globalSettings.acceptedWeatherTerms = true
                    weatherDownloadManager.update()
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
                text: weatherDownloadManager.QNHInfo
            }

            Icon {
                visible: sunLabel.text != ""
                source: "/icons/material/ic_wb_sunny.svg"
            }
            Label {
                id: sunLabel
                visible: sunLabel.text != ""
                Layout.fillWidth: true
                text: weatherDownloadManager.sunInfo
            }

        }

    } // Pane (footer)

    // Try and update METAR/TAF as soon as someone opens this page if the current list of stations
    // is empty. This is not a background update, we want user interaction.
    Component.onCompleted: {
        if (stationList.count == 0)
            weatherDownloadManager.update(false)
        else
            weatherDownloadManager.update(true)
    }

    // Show error when weather cannot be updated -- but not if we are running a background upate
    Connections {
        target: weatherDownloadManager
        function onError (message) {
            if (weatherDownloadManager.backgroundUpdate)
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
    }

} // Page
