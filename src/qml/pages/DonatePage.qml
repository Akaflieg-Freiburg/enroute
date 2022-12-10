/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

Page {
    id: pg
    title: qsTr("Donate")

    header: StandardHeader {}

    ScrollView {
        id: sv

        clip: true
        anchors.fill: parent

        topPadding: view.font.pixelSize
        leftPadding: view.font.pixelSize
        rightPadding: view.font.pixelSize

        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right


        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        ScrollBar.vertical.interactive: false

        ColumnLayout {
            id: cL

            width: sv.availableWidth
            implicitWidth: sv.availableWidth

            Label {
                id: lbl1

                property string bankAccountData: qsTr("
IBAN:    DE35 6809 0000 0027 6409 07
BIC:     GENODE61FR1
Bank:    Volksbank Freiburg
Message: Enroute Flight Navigation
")

                Layout.fillWidth: true

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
%1
```

If you prefer to work on
your desktop computer, you can also send yourself an e-mail
with the bank account data.
").arg(bankAccountData)
                width: pg.width
                wrapMode: Text.Wrap
                topPadding: view.font.pixelSize*1
                leftPadding: view.font.pixelSize*0.5
                rightPadding: view.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Send e-mail")
                icon.source: "/icons/material/ic_send.svg"

                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    Qt.openUrlExternally(qsTr("mailto:?subject=Enroute Flight Navigation, Donation&body=%1").arg(lbl1.bankAccountData))
                }
            }

        }
    }

} // Page
