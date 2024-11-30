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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Weather")
    focus: true

    header: PageHeader {

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

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right

            icon.source: "/icons/material/ic_refresh.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                if (!WeatherDataProvider.downloading)
                    WeatherDataProvider.update(false)
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
                    color: model.modelData.metar.isValid ? model.modelData.metar.flightCategoryColor : "transparent"
                    opacity: 0.2
                }

                WordWrappingItemDelegate {
                    leftPadding: SafeInsets.left+16
                    rightPadding: SafeInsets.right+16

                    id: idel
                    text: {
                        var result = model.modelData.twoLineTitle

                        var wayTo = Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), model.modelData.coordinate)
                        if (wayTo !== "")
                            result = result + "<br>" + wayTo

                        if (model.modelData.metar.isValid)
                            result = result + "<br>" + model.modelData.metar.summary(Navigator.aircraft, Clock.time)

                        return result
                    }
                    icon.source: model.modelData.icon
                    icon.color: "transparent"

                    width: parent.width

                    onClicked: {
                        // WARNING!
                        PlatformAdaptor.vibrateBrief()

                        dlgLoader.setSource("../dialogs/WeatherReport.qml",
                                            {"weatherStation": model.modelData})
                        dlgLoader.item.open()
                        //weatherReport.weatherStation =
                        //weatherReport.open()
                    }
                }
            }
        }

    DecoratedListView {
            id: stationList

            anchors.fill: parent
            anchors.leftMargin: SafeInsets.left
            anchors.rightMargin: SafeInsets.right

            clip: true

            model: WeatherDataProvider.weatherStations
            delegate: stationDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Rectangle {  // No data label
                anchors.fill: parent
                color: "white"
                visible: stationList.count === 0

                Text {
                    anchors.fill: parent

                    leftPadding: font.pixelSize
                    rightPadding: font.pixelSize
                    topPadding: 2*font.pixelSize

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No METAR/TAF data available. You can restart the download manually using the refresh button at the top right corner of the screen.</p>")
                }
            }

            Rectangle {  // Download in progress label
                id: downloadIndicator

                anchors.fill: parent

                color: "white"
                visible: WeatherDataProvider.downloading && !WeatherDataProvider.backgroundUpdate

                ColumnLayout {
                    anchors.fill: parent

                    Item { Layout.fillHeight: true }

                    Text {
                        Layout.fillWidth: true

                        leftPadding: font.pixelSize
                        rightPadding: font.pixelSize

                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.StyledText
                        wrapMode: Text.Wrap
                        text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download METAR/TAF data from the Aviation Weather Center…</p>")
                    }

                    BusyIndicator {
                        Layout.fillWidth: true
                    }

                    Item { Layout.fillHeight: true }
                }

                // The Connections and the SequentialAnimation here provide a fade-out animation for the downloadindicator.
                // Without this, the downaloadIndication would not be visible on very quick downloads, leaving the user
                // without any feedback if the download did actually take place.
                Connections {
                    target: WeatherDataProvider
                    function onDownloadingChanged () {
                        if (WeatherDataProvider.downloading && !WeatherDataProvider.backgroundUpdate) {
                            downloadIndicator.visible = true
                            downloadIndicator.opacity = 1.0
                        } else
                            fadeOut.start()
                    }
                }
                SequentialAnimation {
                    id: fadeOut
                    NumberAnimation { target: downloadIndicator; property: "opacity"; to:1.0; duration: 400 }
                    NumberAnimation { target: downloadIndicator; property: "opacity"; to:0.0; duration: 400 }
                    NumberAnimation { target: downloadIndicator; property: "visible"; to:1.0; duration: 20}
                }
            }


            // Refresh METAR/TAF data on overscroll
            property int refreshFlick: 0
            onFlickStarted: {
                refreshFlick = atYBeginning
            }

            onFlickEnded: {
                if ( atYBeginning && refreshFlick ) {
                    PlatformAdaptor.vibrateBrief()
                    WeatherDataProvider.update(false)
                }
            }

            // Try and update METAR/TAF as soon as someone opens this page if the current list of stations
            // is empty. This is not a background update, we want user interaction.
            Component.onCompleted: {
                if (stationList.count === 0)
                    WeatherDataProvider.update(false)
                else
                    WeatherDataProvider.update(true)
            }

            // Show error when weather cannot be updated -- but not if we are running a background upate
            Connections {
                target: WeatherDataProvider
                function onError (message) {
                    if (WeatherDataProvider.backgroundUpdate)
                        return
                    dlgLoader.active = false
                    dlgLoader.setSource("../dialogs/LongTextDialog.qml",
                                        {
                                            standardButtons: Dialog.Ok,
                                            text: qsTr("<p>Failed to update the list of weather stations.</p><p>Reason: %1.</p>").arg(message),
                                            title: qsTr("Update Error")
                                        })
                    dlgLoader.active = true
                }
            }

        }

    footer: Footer {
        visible: (sunLabel.text !== "") || (qnhLabel.text !== "")

        GridLayout {
            anchors.fill: parent
            columns: 2

            Icon {
                visible: qnhLabel.text !== ""
                source: "/icons/material/ic_speed.svg"
            }
            Label {
                id: qnhLabel
                visible: qnhLabel.text !== ""
                Layout.fillWidth: true
                text: {
                    var txt = WeatherDataProvider.QNHInfo
                    if (txt !== "")
                        txt = "QNH: " + txt
                    return txt
                }
            }

            Icon {
                visible: sunLabel.text !== ""
                source: "/icons/material/ic_wb_sunny.svg"
            }
            Label {
                id: sunLabel
                visible: sunLabel.text !== ""
                Layout.fillWidth: true
                text: WeatherDataProvider.sunInfo
            }

        }
    }

    Loader {
        id: dlgLoader
        onLoaded: item.open()
    }
} // Page
