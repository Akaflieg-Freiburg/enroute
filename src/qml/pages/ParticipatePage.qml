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

Page {
    id: pg
    title: qsTr("Help to make Enroute better")

    ScrollView {
        clip: true
        anchors.fill: parent
        
        // The Label that we really want to show is wrapped into an Item. This allows
        // to set implicitHeight, and thus compute the implicitHeight of the Dialog
        // without binding loops
        Item {
            implicitHeight: lbl1.implicitHeight
            width: pg.width
            
            Label {
                id: lbl1
                text: participateText
                textFormat: Text.RichText
                width: pg.width
                wrapMode: Text.Wrap
                topPadding: Qt.application.font.pixelSize*1
                leftPadding: Qt.application.font.pixelSize*0.5
                rightPadding: Qt.application.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            } // Label
        } // Item
    } // ScrollView
} // Page
