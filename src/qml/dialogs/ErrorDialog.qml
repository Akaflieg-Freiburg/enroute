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
import QtQuick.Controls.Material 2.15

Dialog {
    id: dlg

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)
    height: Math.min(parent.height-view.font.pixelSize, implicitHeight)

    modal: true
    title: dialogLoader.title
    standardButtons: Dialog.Ok
    
    ScrollView{
        id: sv
        anchors.fill: parent

        contentHeight: lbl.height
        contentWidth: dlg.availableWidth

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        clip: true

        Label {
            id: lbl
            text: dialogLoader.text
            width: dlg.availableWidth
            textFormat: Text.StyledText
            linkColor: Material.accent
            wrapMode: Text.Wrap
            onLinkActivated: Qt.openUrlExternally(link)
        } // Label
    } // ScrollView

} // Dialog
