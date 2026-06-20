// Stepped color scale legend: one colored rectangle per bin, boundary labels below.
import QtQuick
import QtQuick.Controls

Item {
    id: root

    // N colors + N+1 boundaries define N bins.
    // When boundaries is empty, falls back to evenly-spaced bins across [vmin, vmax].
    property var    colors:     []
    property var    boundaries: []   // preferred: actual bin-edge values
    property double vmin:       0    // fallback when no boundaries
    property double vmax:       1    // fallback when no boundaries
    property string units:      ""
    // Optional label formatter: function(value) → string
    property var    labelFunc:  null

    implicitHeight: bar.height + labels.height + 2
    implicitWidth:  200

    // Resolved boundaries (always N+1 entries for N color bins)
    readonly property var _bounds: {
        if (boundaries && boundaries.length >= 2)
            return boundaries
        // Fallback: evenly spaced
        var n = colors.length || 4
        var step = (vmax - vmin) / n
        var b = []
        for (var i = 0; i <= n; i++) b.push(vmin + i * step)
        return b
    }

    readonly property int _nBins: Math.max(0, _bounds.length - 1)

    function _fmt(v) {
        if (root.labelFunc) return root.labelFunc(v)
        if (v >= 10000) return Math.round(v / 1000) + "k"
        if (v >= 1000)  return Math.round(v).toString()
        if (v < 1)      return v.toFixed(1)
        return Math.round(v).toString()
    }

    // Colored bin rectangles
    Row {
        id: bar
        width: root.width
        height: 10
        spacing: 0

        Repeater {
            model: root._nBins
            Rectangle {
                width:  index === root._nBins - 1
                            ? bar.width - Math.floor(bar.width / root._nBins) * (root._nBins - 1)
                            : Math.floor(bar.width / root._nBins)
                height: bar.height
                color:  root.colors[index] ?? "transparent"
                radius: index === 0 ? 2 : (index === root._nBins - 1 ? 2 : 0)
            }
        }
    }

    // Boundary labels: one label per boundary, positioned at the bin edge
    Item {
        id: labels
        anchors { top: bar.bottom; topMargin: 2 }
        width: root.width
        height: boundaryRepeater.itemAt(0) ? boundaryRepeater.itemAt(0).implicitHeight : 12

        Repeater {
            id: boundaryRepeater
            model: root._bounds.length

            Label {
                x: Math.round((index / (root._bounds.length - 1)) * (labels.width - width))
                font.pixelSize: 9
                opacity: 0.7
                text: {
                    var v = root._bounds[index]
                    var s = root._fmt(v)
                    // Append units only to the last label
                    if (index === root._bounds.length - 1 && root.units)
                        return s + " " + root.units
                    return s
                }
                horizontalAlignment: index === 0                         ? Text.AlignLeft
                                   : index === root._bounds.length - 1  ? Text.AlignRight
                                   :                                       Text.AlignHCenter
            }
        }
    }
}
