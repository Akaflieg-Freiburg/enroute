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

Page {
    id: pg
    title: qsTr("Donate")

    header: StandardHeader {}

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
                textFormat: Text.MarkdownText
                linkColor: Material.accent
                text: qsTr("
**Enroute Flight Navigation** is a
non-commercial project of Akaflieg Freiburg and the
University of Freiburg. The app has been written by flight
enthusiasts in their spare time, as a service to the
community. The developers do not take donations.

If you appreciate the app, please consider a donation to
Akaflieg Freiburg, a tax-privileged, not-for-profit flight
club of public utility in Freiburg, Germany.

```
IBAN:    DE35 6809 0000 0027 6409 07
BIC:     GENODE61FR1
Bank:    Volksbank Freiburg
Message: Enroute Flight Navigation
```
")
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
