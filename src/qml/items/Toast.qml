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

import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

// Short, auto-hiding message at the lower part of the window. The instance
// lives in main.qml and is reachable everywhere as Global.toast.
Label {
    id: toast

    width: Math.min(parent.width-4*toast.font.pixelSize, 40*toast.font.pixelSize)
    x: (parent.width-width)/2.0
    y: parent.height*(3.0/4.0)-height/2.0

    text: "Lirum Larum, Löffelstiel"
    wrapMode: Text.Wrap

    color: "white"
    bottomInset: -5
    topInset: -5
    leftInset: -5
    rightInset: -5

    horizontalAlignment: Text.AlignHCenter
    background: Rectangle {
        color: "teal"
        radius: 5
    }

    opacity: 0
    SequentialAnimation {
        id: seqA

        NumberAnimation { target: toast; property: "opacity"; to: 1; duration: 400 }
        PauseAnimation { duration: 1000 }
        NumberAnimation { target: toast; property: "opacity"; to: 0; duration: 400 }
    }

    function doToast(string) {
        if (seqA.running) {
            toast.text = string + " • " + toast.text
        } else
            toast.text = string
        seqA.start()
    }
}
