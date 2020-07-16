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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Shapes 1.14

import enroute 1.0

Dialog {
    id: dlg

    property var dialogArgs: undefined

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    modal: true
    standardButtons: Dialog.Close
    focus: true

    Component {
        id: reportPropertyDelegate

        RowLayout {
            id: repRow

            Layout.preferredWidth: sv.width

            property var text: ({});

            Label {
                text: repRow.text.substring(0,4)
                Layout.preferredWidth: Qt.application.font.pixelSize*3
                Layout.alignment: Qt.AlignTop
                font.bold: true

            }
            Label {
                Layout.fillWidth: true
                text: repRow.text.substring(4)
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
            }

        }
    }

    contentItem: ColumnLayout {
        width: dlg.availableWidth
        height: dlg.availableHeight

        RowLayout {
            id: headX
            Layout.fillWidth: true

            Image {
                source: "/icons/material/ic_cloud_queue.svg" // to be replaced by badge
                sourceSize.width: 25
            }
            Label {
                text: dialogArgs.station.id
                font.bold: true
                font.pixelSize: 1.2*Qt.application.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
            ToolButton {
                icon.source: "/icons/material/ic_bug_report.svg"

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    Qt.openUrlExternally(qsTr("mailto:stefan.kebekus@gmail.com?subject=Enroute, Error Report &body=Thank you for suggesting a correction in the map data. Please describe the issue here."))
                }
            }
        }

        ScrollView {
            id: sv
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            Layout.fillHeight: true

            contentHeight: co.implicitHeight
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < co.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            ColumnLayout {
                id: co

                width: parent.width

                Component.onCompleted: {
                    
                    var metar = dialogLoader.dialogArgs.station.metar
                    for (var j in metar)
                        reportPropertyDelegate.createObject(co, {text: metar[j]});
                    var taf = dialogLoader.dialogArgs.station.taf
                    for (var j in taf)
                        reportPropertyDelegate.createObject(co, {text: taf[j]});
                }
            } // ColumnLayout

        } // ScrollView

        Keys.onBackPressed: {
            event.accepted = true;
            dlg.close()
        }
    }

} // Dialog
