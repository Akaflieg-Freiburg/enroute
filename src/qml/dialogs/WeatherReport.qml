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

import enroute 1.0

/* This is a dialog with detailed information about a weather station. To use this dialog, all you have to do is to set a WeatherStation in the property "weatherStation" and call open(). */


Dialog {
    id: weatherReportDialog

    property WeatherStation weatherStation

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(view.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: (parent.width-width)/2.0
    y: (parent.height-height)/2.0

    modal: true
    standardButtons: Dialog.Close
    focus: true

    ColumnLayout {
        anchors.fill: parent

        RowLayout { // Header with icon and name
            id: headX
            Layout.fillWidth: true

            Image {
                source: (weatherStation !== null) ? weatherStation.icon : "/icons/waypoints/WP.svg"
                sourceSize.width: 25
            }

            Label {
                text: (weatherStation !== null) ? weatherStation.extendedName : ""
                font.bold: true
                font.pixelSize: 1.2*Qt.application.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: (weatherStation !== null) ? weatherStation.wayTo : ""
            visible: satNav.status === SatNav.OK
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        ScrollView { // ScrollView with METAR & TAF information
            id: sv

            Layout.fillHeight: true
            Layout.fillWidth: true

            contentHeight: co.height
            contentWidth: weatherReportDialog.availableWidth

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < co.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            ColumnLayout {
                id: co
                width: parent.width

                Label { // title: "METAR"
                    visible: (weatherStation !== null) && weatherStation.hasMETAR
                    text: (weatherStation !== null) && weatherStation.hasMETAR ? (weatherStation.metar.messageType + " " + weatherStation.metar.relativeObservationTime) : ""
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                Label { // raw METAR text
                    visible: (weatherStation !== null) && weatherStation.hasMETAR
                    text: (weatherStation !== null) ? weatherStation.metar.rawText : ""
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*Qt.application.font.pixelSize
                    topPadding: 0.2*Qt.application.font.pixelSize
                    leftPadding: 0.2*Qt.application.font.pixelSize
                    rightPadding: 0.2*Qt.application.font.pixelSize

                    // Background color according to METAR/FAA flight category
                    background: Rectangle {
                        border.color: "black"
                        color: (weatherStation !== null) && weatherStation.hasMETAR ? weatherStation.metar.flightCategoryColor : "transparent"
                        opacity: 0.2
                    }
                }

                Label { // decoded METAR text
                    visible: (weatherStation !== null) && weatherStation.hasMETAR
                    text: (weatherStation !== null) ? weatherStation.metar.decodedText : ""
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Label { // title: "TAF"
                    visible: (weatherStation !== null) && weatherStation.hasTAF
                    text: "TAF"
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                Label { // raw TAF text
                    visible: (weatherStation !== null) && weatherStation.hasTAF
                    text: (weatherStation !== null) && weatherStation.hasTAF ? weatherStation.taf.rawText : ""
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*Qt.application.font.pixelSize
                    topPadding: 0.2*Qt.application.font.pixelSize
                    leftPadding: 0.2*Qt.application.font.pixelSize
                    rightPadding: 0.2*Qt.application.font.pixelSize

                    background: Rectangle {
                        border.color: "black"
                    }
                }

                Label { // decoded TAF text
                    visible: (weatherStation !== null) && weatherStation.hasTAF
                    text: (weatherStation !== null) && weatherStation.hasTAF ? weatherStation.taf.decodedText : ""
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

            }

        }

        Keys.onBackPressed: {
            event.accepted = true;
            weatherReportDialog.close()
        }
    }

}
