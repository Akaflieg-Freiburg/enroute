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

import "../items"

Page {
    id: pg
    title: qsTr("Participate")

    header: StandardHeader {}

    ScrollView {
        clip: true
        anchors.fill: parent
        
        // The label that we really want to show is wrapped into an Item. This allows
        // to set implicitHeight, and thus compute the implicitHeight of the Dialog
        // without binding loops
        Item {
            implicitHeight: lbl1.implicitHeight
            width: pg.width
            
            NightAndDayLabel {
                id: lbl1
                textFormat: Qt.RichText
                nightAndDayText: qsTr("
<h3>Participate in the development</h3>

<p>We have great plans for <strong>Enroute Flight
Navigation</strong>, check our
<a href='https://github.com/Akaflieg-Freiburg/enroute/projects'>project
list at GitHub</a> to see what's coming.</p>

<h3>Translate</h3>

<p>If know how to use the GIT version control system and if
you would like to translate the app to your native
language, we would like to hear from you. Programming
experience is not necessary, but good computer skills are
required.</p>

<h3>Join the development</h3>

<p>If you are fluent in C++ and if you would like to help
with the programming, please get in touch with us by
opening a GitHub issue. We are also struggling with power
management on Android and would be grateful for any help
that we could get.</p>

<h3>Port to iOS</h3>

<p>This app should run fine on Apple devices. If you would
like to port the app to iOS and if you have development
experience with C++/Qt programming and with Apple systems,
please get in touch with us by opening a GitHub issue.</p>
")
                width: pg.width
                wrapMode: Text.Wrap
                topPadding: Qt.application.font.pixelSize*1
                leftPadding: Qt.application.font.pixelSize*0.5
                rightPadding: Qt.application.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        } // Item
    } // ScrollView
} // Page
