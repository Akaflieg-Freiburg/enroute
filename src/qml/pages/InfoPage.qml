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
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import "../items"

Page {
    id: pg
    title: qsTr("About Enroute")


    header: StandardHeader {}

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: sv.currentIndex

        TabButton {
            text: "Enroute"
        }
        TabButton {
            text: qsTr("Author")
        }
        TabButton {
            text: qsTr("License")
        }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex
        
        ScrollView {
            clip: true

            // The Label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl1.implicitHeight
                width: pg.width

                Label {
                    id: lbl1
                    text: librarian.getStringFromRessource(":text/info_enroute.html")
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
        
        ScrollView {
            clip: true

            // The Label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl2.implicitHeight
                width: pg.width

                Label {
                    id: lbl2
                    text: "
<h3>Authors</h3>

<br>

<table>
  <tr>
    <td>
      <p>The app <strong>enroute flight navigation</strong> was written by Stefan Kebekus, flight enthusiast since 1986 and member of the Akaflieg Freiburg flight club. Stefan flies gliders and motor planes.</p>
      <h4>Address</h4>
      Stefan Kebekus<br>
      Wintererstraße 77<br>
      79104 Freiburg<br>
      Germany<br>
      <br>
      <a href='mailto:stefan.kebekus@gmail.com'>e-mail</a>
    </td>
    <td>
      <p align='center'>&nbsp;<img src='/icons/kebekus.jpg' alt='Stefan Kebekus' height='140'><br>Stefan Kebekus<br>Pic: Patrick Seeger</p>
    </td>
  </tr>
  <tr>
    <td>
      <br>
      <h3>Contributing Authors</h3>
      <br>
    </td>
  </tr>
  <tr>
    <td>
      <p>Heiner Tholen enjoys building things, analog and digital, airborne as well as ground-based. He uses enroute as a pilot of ultralight planes. Heiner joined the enroute team mid 2020 and contributes to the C++/QML codebase.</p>
      <br>
      <br>
      <a href='mailto:ul@heinertholen.com'>e-mail</a>
    </td>
    <td>
      <p align='center'>&nbsp;<img src='/icons/tholen.jpg' alt='Heiner Tholen' height='140'><br>Heiner Tholen</p>
    </td>
  </tr>
  <tr>
    <td>
      <p>Johannes Zellner joined the development in 2020.  He contributes to the C++ and QML code base of the app and helps with bug fixing.</p>
      <br>
      <br>
      <a href='mailto:johannes@zellner.org'>e-mail</a>
    </td>
    <td>
      <p align='center'>&nbsp;<img src='/icons/zellner.jpg' alt='Johannes Zellner' height='140'><br>Johannes Zellner</p>
    </td>
  </tr>
</table>

<h3>Translations</h3>

<p>German: Markus Sachs, <a href='mailto:ms@squawk-vfr.de'>E-Mail</a>. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</p>
<p></p>
"
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

        ScrollView {
            clip: true

            // The Label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl3.implicitHeight
                width: pg.width

                Label {
                    id: lbl3
                    text: librarian.getStringFromRessource(":text/info_license.html")
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

    } // StackView
} // Page
