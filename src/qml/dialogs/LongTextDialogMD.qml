/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute
import "../items"

CenteringDialog {
    id: dialogMain

    // This is the text to be shown
    property var text: ({})

    modal: true
    
    DecoratedScrollView{
        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        // Delays evaluation and prevents binding loops
        Binding on implicitHeight {
            value: lbl.implicitHeight
            delayed: true    // Prevent intermediary values from being assigned
        }

        Label {
            id: lbl

            text: dialogMain.text
            width: dialogMain.availableWidth
            textFormat: Text.MarkdownText
            wrapMode: Text.Wrap
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
