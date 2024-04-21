/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

CenteringDialog {
    id: dlg

    modal: true
    title: qsTr("Add Bluetooth Device")
    standardButtons: Dialog.Cancel

    property ConnectionScanner_Bluetooth btScanner: TrafficDataProvider.btScanner


    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Error") + ": " + btScanner.error
            visible: btScanner.error !== ""
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true

            text: qsTr("No Device Found")
            visible: btScanner.connectionInfos.length === 0
            wrapMode: Text.Wrap
        }

        DecoratedListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight

            clip: true

            model: btScanner.connectionInfos

            delegate: WordWrappingItemDelegate {
                width: dlg.availableWidth

                enabled: model.modelData.canAddConnection
                icon.source: model.modelData.icon
                text: model.modelData.description

                onClicked: {
                    var resultString = TrafficDataProvider.addDataSource(model.modelData)
                    Global.toast.doToast( qsTr("Added Device %1").arg(model.modelData.name) )
                    dlg.close()
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter

            text: btScanner.scanning ? qsTr("Scanning…") : qsTr("Scan for Devices")

            enabled: !btScanner.scanning
            icon.source: "/icons/material/ic_bluetooth_searching.svg"
            onClicked: btScanner.start()
            visible: btScanner.error === ""
        }

    }

}
