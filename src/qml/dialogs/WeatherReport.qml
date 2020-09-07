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
import QtQuick.Shapes 1.15

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

            width: dlg.availableWidth
            implicitWidth: dlg.availableWidth
//            Layout.preferredWidth: sv.width
//            width: 200

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
                source: "/icons/weather/" + dialogArgs.station.cat + ".svg"
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
        }

        ScrollView {
            id: sv

            Layout.fillHeight: true
            Layout.fillWidth: true

            contentHeight: co.implicitHeight
            contentWidth: dlg.availableWidth

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < co.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            ColumnLayout {
                id: co
                width: dlg.availableWidth

                Label {
                    text: "METAR"
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                ColumnLayout {
                    id: metarCO

                    width: parent.width
                    
                    Component.onCompleted: {
                        var metar = dialogLoader.dialogArgs.station.metarStrings
                        console.log(metar)

                        for (var j in metar)
                            reportPropertyDelegate.createObject(metarCO, {text: metar[j]});
                    }
                }

                Label {
                    text: "TAF"
                    font.bold: true
                    font.pixelSize: 1.2*Qt.application.font.pixelSize
                }

                ColumnLayout {
                    id: tafCO

                    width: parent.width

                    Component.onCompleted: {
                        var taf = dialogLoader.dialogArgs.station.tafStrings
                        for (var j in taf)
                            reportPropertyDelegate.createObject(tafCO, {text: taf[j]});
                    }
                }

            } // ColumnLayout

        } // ScrollView

        Keys.onBackPressed: {
            event.accepted = true;
            dlg.close()
        }
    }

} // Dialog
