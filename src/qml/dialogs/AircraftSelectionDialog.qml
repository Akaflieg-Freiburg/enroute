/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

// Quick picker for the active aircraft. Aircraft are added and edited in the
// aircraft library.
CenteringDialog {
    id: dlg

    title: qsTr("Select Aircraft")
    modal: true

    ColumnLayout {
        anchors.fill: parent

        DecoratedListView {
            id: lView

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: contentHeight

            clip: true
            visible: count > 0

            model: NameFilterProxyModel {
                sourceModel: Librarian.aircraftModel
            }

            delegate: Item {
                id: entryRow

                required property string name

                // The delegate is created before the view parents it
                width: lView.width
                implicitHeight: row.implicitHeight

                readonly property bool active: entryRow.name === Navigator.aircraftName

                // Rounded, so it also frames the edit button
                Rectangle {
                    anchors.fill: parent

                    visible: entryRow.active
                    radius: 8
                    color: entryRow.Material.listHighlightColor
                    border.width: 1
                    border.color: entryRow.Material.hintTextColor
                }

                RowLayout {
                    id: row

                    anchors.fill: parent

                    ItemDelegate {
                        Layout.fillWidth: true

                        text: entryRow.name
                        font.weight: entryRow.active ? Font.Bold : Font.Normal
                        icon.source: entryRow.active ? "/icons/material/ic_airplanemode_active.svg"
                                                     : "/icons/material/ic_airplanemode_outline.svg"

                        onClicked: {
                            PlatformAdaptor.vibrateBrief()
                            var errorString = Navigator.selectAircraft(entryRow.name)
                            if (errorString !== "")
                                Global.toast.doToast(qsTr("Error: %1").arg(errorString))
                            else
                                Global.toast.doToast(qsTr("Aircraft <strong>%1</strong> is active").arg(entryRow.name))
                            dlg.close()
                        }
                    }

                    ToolButton {
                        icon.source: "/icons/material/ic_mode_edit.svg"

                        onClicked: {
                            PlatformAdaptor.vibrateBrief()
                            dlg.close()
                            Global.stackView.push("../pages/AircraftEditor.qml", {entryName: entryRow.name})
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true

            visible: lView.count === 0
            text: qsTr("The aircraft library is empty. Open the library to add an aircraft.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }
    }

    // Explicit Cancel button: combining standardButtons with a custom
    // button may drop the standard button
    footer: DialogButtonBox {
        Button {
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            flat: true
            text: qsTr("Cancel")
        }

        Button {
            flat: true
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            text: qsTr("View Library")

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                dlg.close()
                Global.stackView.push("../pages/AircraftLibrary.qml")
            }
        }
    }
}
