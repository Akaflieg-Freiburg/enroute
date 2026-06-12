// Color scale legend bar with labeled tick marks at each color stop.
// Expects 4 colors at positions 0, 0.33, 0.66, 1.0 matching the server colormaps.

import QtQuick
import QtQuick.Controls

Item {
    id: root

    property var    colors: []
    property double vmin: 0
    property double vmax: 1
    property string units: ""
    // Optional custom label formatter: function(value, isLast) → string
    // When set, overrides the default fmt() + units logic
    property var    labelFunc: null

    implicitHeight: bar.height + ticks.height + 2
    implicitWidth:  200

    // Gradient bar
    Rectangle {
        id: bar
        width: root.width
        height: 10
        radius: 2
        border.color: Qt.rgba(0, 0, 0, 0.15)
        border.width: 1

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0;  color: root.colors[0] ?? "transparent" }
            GradientStop { position: 0.33; color: root.colors[1] ?? "transparent" }
            GradientStop { position: 0.66; color: root.colors[2] ?? "transparent" }
            GradientStop { position: 1.0;  color: root.colors[3] ?? "transparent" }
        }
    }

    // Tick labels: vmin | stop1 | stop2 | vmax [units]
    Row {
        id: ticks
        width: root.width
        anchors { top: bar.bottom; topMargin: 2 }

        readonly property double range: root.vmax - root.vmin
        readonly property double v1: root.vmin + range * 0.33
        readonly property double v2: root.vmin + range * 0.66

        function fmt(v, isLast) {
            if (root.labelFunc) return root.labelFunc(v)
            var s = v >= 1000 ? Math.round(v).toString()
                  : v < 1    ? v.toFixed(1)
                  :             Math.round(v).toString()
            return isLast && root.units ? s + " " + root.units : s
        }

        Label {
            width: parent.width * 0.25
            text: ticks.fmt(root.vmin, false)
            font.pixelSize: 9
            horizontalAlignment: Text.AlignLeft
            opacity: 0.7
        }
        Label {
            width: parent.width * 0.25
            text: ticks.fmt(ticks.v1, false)
            font.pixelSize: 9
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
        }
        Label {
            width: parent.width * 0.25
            text: ticks.fmt(ticks.v2, false)
            font.pixelSize: 9
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.7
        }
        Label {
            width: parent.width * 0.25
            text: ticks.fmt(root.vmax, true)
            font.pixelSize: 9
            horizontalAlignment: Text.AlignRight
            opacity: 0.7
        }
    }
}
