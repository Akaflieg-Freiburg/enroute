/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

import QtQml
import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

// This is a variant of TextField. The main differences to the standard
// implementation are the following.
//
// - Instances of this class call PlatformAdaptor.setupImEventFilter() on
//   completion. On iOS this avoids problematic behavior where the virtual
//   keyboard pushes up the whole qml page. Details for this problem are
//   described here
//
//   https://stackoverflow.com/questions/34716462/ios-sometimes-keyboard-pushes-up-the-whole-qml-page
//
//   https://bugreports.qt.io/browse/QTBUG-80790


TextField {
    id: textField

    rightPadding: toolButton.width

    Component.onCompleted: PlatformAdaptor.setupInputMethodEventFilter(textField)

    RoundButton {
        id: toolButton

        anchors.right: parent.right
        anchors.top: parent.top

        background: Item {}

        icon.source: "/icons/material/ic_clear.svg"
        icon.width: font.pixelSize
        icon.height: font.pixelSize

        enabled: textField.displayText !== ""
        onClicked: {
            textField.clear()
            textField.onEditingFinished()
        }
    }
}
