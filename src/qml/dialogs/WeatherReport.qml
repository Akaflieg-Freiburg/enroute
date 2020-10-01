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

Dialog {
    id: dlg

    property var dialogArgs: undefined

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

    modal: true
    standardButtons: Dialog.Close
    focus: true

    ColumnLayout {
        anchors.fill: parent

        RowLayout { // Header with icon and name
            id: headX
            Layout.fillWidth: true

            Image {
                source: dialogArgs.waypoint.icon
                sourceSize.width: 25
            }
            Label {
                text: dialogArgs.waypoint.extendedName
                font.bold: true
                font.pixelSize: 1.2*Qt.application.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: dialogArgs.waypoint.wayTo
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
            contentWidth: dlg.availableWidth

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
                    visible: dialogArgs.waypoint.hasMETAR
                    text: dialogArgs.waypoint.hasMetar ? (dialogArgs.waypoint.metar.messageType + " " + dialogArgs.waypoint.metar.relativeObservationTime) : ""
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                Label { // raw METAR text
                    visible: dialogArgs.waypoint.hasMETAR
                    text: dialogArgs.waypoint.metar.rawText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*Qt.application.font.pixelSize
                    topPadding: 0.2*Qt.application.font.pixelSize
                    leftPadding: 0.2*Qt.application.font.pixelSize
                    rightPadding: 0.2*Qt.application.font.pixelSize

                    // Background color according to METAR/FAA flight category
                    background: Rectangle {
                        border.color: "black"
                        color: dialogArgs.waypoint.flightCategoryColor
                        opacity: 0.2
                    }
                }

                Label { // decoded METAR text
                    visible: dialogArgs.waypoint.hasMetar
                    text: dialogArgs.waypoint.metar.decodedText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                Label { // title: "TAF"
                    visible: dialogArgs.waypoint.hasTaf
                    text: "TAF"
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                Label { // raw TAF text
                    visible: dialogArgs.waypoint.hasTaf
                    text: dialogArgs.waypoint.hasTaf ? dialogArgs.waypoint.taf.rawText : ""
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
                    visible: dialogArgs.waypoint.hasTaf
                    text: dialogArgs.waypoint.hasTaf ? dialogArgs.waypoint.taf.decodedText : ""
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

            }

        }

        Keys.onBackPressed: {
            event.accepted = true;
            dlg.close()
        }
    }

}
