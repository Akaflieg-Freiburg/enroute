/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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
import "../items"

/* TODO

  - Sort entries by distance to current position

  - Give one-line weather description, including time ("23 minutes ago")

  - Perhaps give sunset information in top line

  - Unify/coordinate behaviour with MapManager

 */

Page {
    id: pg
    title: qsTr("Weather")

    header: ToolBar {

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        } // ToolButton

        Label {
            anchors.left: backButton.right
            anchors.right: headerMenuToolButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Update METAR/TAF")
                    enabled: (!meteorologist.downloading) && (globalSettings.acceptedWeatherTerms)
                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        if (!meteorologist.downloading)
                            meteorologist.update()
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

        } // ToolButton

    } // ToolBar

    Component {
        id: stationDelegate

        ItemDelegate {
            width: pg.width

            text: {
                var wp = geoMapProvider.findByID(model.modelData.id)
                if (wp === null)
                    return model.modelData.id
                var result = wp.richTextName
                if (satNav.status === SatNav.OK)
                    result += "<br>" + wp.wayFrom(satNav.lastValidCoordinate, globalSettings.useMetricUnits)
                return result
            }

            icon.source: "/icons/weather/" + model.modelData.cat + ".svg"
            icon.color: "transparent"
            icon.width: Qt.application.font.pixelSize*3
            icon.height: Qt.application.font.pixelSize*3

            onClicked: {
                mobileAdaptor.vibrateBrief()
                dialogLoader.active = false
                dialogLoader.dialogArgs = {station: model.modelData}
                dialogLoader.text = ""
                dialogLoader.source = "../dialogs/WeatherReport.qml"
                dialogLoader.active = true
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

            model: meteorologist.reports
            delegate: stationDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Rectangle {
                anchors.fill: parent
                color: "white"
                visible: stationList.count == 0

                Label {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top

                    leftPadding: Qt.application.font.pixelSize
                    rightPadding: Qt.application.font.pixelSize
                    topPadding: 2*Qt.application.font.pixelSize

                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No METAR/TAF data available. You can restart the download manually using the item 'Update METAR/TAF' from the menu.  To find the menu, look for the symbol '&#8942;' at the top right corner of the screen.</p>")
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
                    meteorologist.update()
                }
            }

        }
    } // ColumnLayout

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
                    meteorologist.update()
                }
            }

        }
    }

    // Manual update button in footer
    footer: Pane {
        width: parent.width

        Material.elevation: 3
        visible: globalSettings.acceptedWeatherTerms

        Label {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap

            text: {
                if (meteorologist.downloading)
                    return qsTr("Downloading METAR/TAF data…")
                if (meteorologist.timeOfLastUpdateAsString === "")
                    return ""
                return qsTr("Last METAR/TAF update: %1").arg(meteorologist.timeOfLastUpdateAsString)
            }
        }
    } // Pane (footer)

    // Show error when weather cannot be updated
    Connections {
        target: meteorologist
        function onError (message) {
            dialogLoader.active = false
            dialogLoader.title = qsTr("Update Error")
            dialogLoader.text = qsTr("<p>Failed to update the list of stations.</p><p>Reason: %1.</p>").arg(message)
            dialogLoader.source = "../dialogs/ErrorDialog.qml"
            dialogLoader.active = true
        }
    }
    
} // Page
