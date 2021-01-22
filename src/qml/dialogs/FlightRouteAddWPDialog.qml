/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

import "../items"

Dialog {
    id: dlg
    title: qsTr("Add Waypoint to Route")
    modal: true
    focus: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: parent.height-2*Qt.application.font.pixelSize
    implicitHeight: height

    standardButtons: DialogButtonBox.Cancel

    Component {
        id: waypointDelegate

        WordWrappingItemDelegate {
            text: model.modelData.twoLineTitle
            icon.source: model.modelData.icon

            width: wpList.width

            onClicked: {
                mobileAdaptor.vibrateBrief()
                flightRoute.append(model.modelData)
                close()
            }
        }

    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Choose a waypoint from the list below.")
            color: Material.accent
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        TextField {
            id: textInput

            Layout.fillWidth: true

            placeholderText: qsTr("Filter Waypoint Names")
            font.pixelSize: Qt.application.font.pixelSize*1.5
            focus: true

            onAccepted: {
                if (wpList.model.length > 0) {
                    mobileAdaptor.vibrateBrief()
                    flightRoute.append(wpList.model[0])
                    close()
                }
            }
        }

        ListView {
            id: wpList

            Layout.fillHeight: true
            Layout.fillWidth: true

            clip: true

            model: geoMapProvider.filteredWaypointObjects(textInput.displayText)
            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Label {
                anchors.fill: wpList
                anchors.topMargin: Qt.application.font.pixelSize*2

                visible: (wpList.count === 0)
                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: (textInput.text === "")
                      ? qsTr("<h3>Sorry!</h3><p>No waypoints available. Please make sure that an aviation map is installed.</p>")
                      : qsTr("<h3>Sorry!</h3><p>No waypoints match your filter criteria.</p>")
                onLinkActivated: Qt.openUrlExternally(link)
            }

        }

    }
} // Page
