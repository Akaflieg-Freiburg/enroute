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

Dialog {
    id: dlg

    // This is the text to be shown
    property var text: ({})

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    modal: true
    
    ScrollView{
        id: sv
        anchors.fill: parent

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.vertical.policy: (height < lbl.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        ScrollBar.vertical.interactive: false

        clip: true
         
        // The Label that we really want to show is wrapped into an Item. This allows
        // to set implicitHeight, and thus compute the implicitHeight of the Dialog
        // without binding loops
        Item {
            implicitHeight: lbl.implicitHeight
            width: dlg.availableWidth

            Label {
                id: lbl
                text: dlg.text
                width: dlg.availableWidth
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignJustify
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            } // Label
        } // Item
    } // ScrollView

} // Dialog
