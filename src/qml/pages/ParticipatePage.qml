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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

Page {
    id: pg
    title: qsTr("Participate")

    header: StandardHeader {}

    DecoratedScrollView {
        id: sView

        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        bottomPadding: SafeInsets.bottom
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
            
        Label {
                id: lbl1
                textFormat: Text.StyledText
                text: qsTr("
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
opening a GitHub issue. We are grateful for any help we
can get.</p>
")
                width: sView.availableWidth
                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
    }

} // Page
