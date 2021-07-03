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
    id: waypointEditorDialog

    // Property waypoint, and code to handle waypoint changes
    property var waypoint: global.geoMapProvider().createWaypoint()
    property int index: -1 // Index of waypoint in flight route

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(view.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: (parent.width-width)/2.0
    y: (parent.height-height)/2.0

    modal: true
    title: qsTr("New waypoint name")

    standardButtons: Dialog.Cancel|Dialog.Ok
    focus: true

    TextField {
        id: wpNameField

        width: waypointEditorDialog.availableWidth

        text: waypoint.extendedName
        focus: true

        onEditingFinished: setName()

        placeholderText: qsTr("undefined")
    }

    onAccepted: setName()

    function setName() {
        global.navigator().flightRoute.renameWaypoint(index, wpNameField.text)
        close()
    }

} // Dialog
