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
import QtQuick.Layouts 1.15

import enroute 1.0

Rectangle {   
    color: "#AA000000"
    height: grid.rri.isValid ? grid.implicitHeight : 0
    visible: grid.rri.isValid

    GridLayout {
        id: grid

        anchors.fill: parent

        columns: 4

        property var rri: global.navigator().remainingRouteInfo

        Label {
            text: grid.rri.nextWP.name
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.nextWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: "ETE %1 h".arg(grid.rri.nextWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: "ETA %1".arg(grid.rri.nextWP_ETA.timeAsUTCString)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: grid.rri.finalWP.name
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.finalWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: "ETE %1 h".arg(grid.rri.finalWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: "ETA %1".arg(grid.rri.finalWP_ETA.timeAsUTCString)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
        }

    }

} // Rectangle
