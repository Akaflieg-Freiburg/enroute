/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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
import QtQuick.Controls.Material

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound

/* Vertical profile of the flight route: departure at the left edge, destination
   at the right edge, horizontal axis proportional to the distance along the
   route. */

Item {
    id: root

    // Same colors as the side view
    readonly property color skyColor: GlobalSettings.nightMode ? "#101820" : "lightblue"
    readonly property color markerColor: GlobalSettings.nightMode ? "#e0e0e0" : "black"

    // Gives access to the static factory functions of distance
    property distance staticDistance

    // Vertical scale. Kept outside the profile, where its labels would collide
    // with the waypoint labels at the edges.
    Label {
        id: fontReference
        visible: false
    }

    TextMetrics {
        id: scaleMetrics

        font: fontReference.font
        text: Navigator.aircraft.verticalDistanceToString(root.staticDistance.fromFT(88888)) // widest label
    }

    Item {
        id: scale

        width: scaleMetrics.width + scaleMetrics.height
        height: parent.height

        Repeater {
            model: routeProfile.altitudeTicks

            Item {
                id: tick

                required property var modelData

                y: modelData.y
                width: scale.width

                Rectangle {
                    anchors.right: parent.right
                    width: scaleMetrics.height/3
                    height: 1
                    color: tickLabel.color
                }

                Label {
                    id: tickLabel

                    anchors.right: parent.right
                    anchors.rightMargin: scaleMetrics.height/2

                    // Centered on the tick, but kept inside the view
                    y: Math.max(-tick.y, Math.min(-height/2, scale.height - tick.y - height))

                    text: tick.modelData.label
                }
            }
        }
    }

    RouteProfileQuickItem {
        id: routeProfile

        anchors.left: scale.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        clip: true
        minimumTickSpacing: 2.5*scaleMetrics.height

        Rectangle {
            anchors.fill: parent
            color: root.skyColor
        }

        ProfileShape {
            terrain: routeProfile.terrain // qmllint disable unresolved-type
            airspaces: routeProfile.airspaces
        }

        Repeater {
            model: routeProfile.waypoints

            Item {
                id: marker

                required property var modelData
                required property int index

                x: modelData.x
                height: routeProfile.height

                Rectangle {
                    x: -width/2
                    width: 1
                    height: marker.height
                    color: root.markerColor
                }

                Label {
                    id: label

                    text: marker.modelData.label
                    visible: text !== ""

                    // Centered on the line, but kept inside the view. Every other label
                    // is moved down, so that labels of close waypoints do not overlap.
                    x: Math.max(-marker.x, Math.min(-width/2, routeProfile.width - marker.x - width))
                    y: font.pixelSize/4 + ((marker.index % 2) ? height : 0)

                    leftPadding: font.pixelSize/4
                    rightPadding: font.pixelSize/4
                    background: Rectangle {
                        color: root.skyColor
                        opacity: 0.8
                    }
                }
            }
        }

        Label {
            anchors.centerIn: parent

            text: routeProfile.error
            visible: routeProfile.error !== ""
            width: 0.66*routeProfile.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter

            leftPadding: font.pixelSize/2.0
            rightPadding: font.pixelSize/2.0
            bottomPadding: font.pixelSize/4.0
            topPadding: font.pixelSize/4.0

            background: Pane { Material.elevation: 1 }
        }
    }
}
