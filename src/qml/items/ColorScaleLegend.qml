// Stepped color scale legend: one colored rectangle per bin, boundary labels below.
import QtQuick
import QtQuick.Controls

Item {
    id: root

    // N colors + N+1 boundaries define N bins.
    // When boundaries is empty, falls back to evenly-spaced bins across [vmin, vmax].
    property var    colors:     []
    property var    boundaries: []   // preferred: actual bin-edge values
    property var    ticks:      []   // subset of boundary values to label; [] means label all
    property double vmin:       0    // fallback when no boundaries
    property double vmax:       1    // fallback when no boundaries
    property string units:      ""
    property var    labelFunc:  null // optional: function(value) → string

    implicitHeight: bar.height + labels.height + 2
    implicitWidth:  200

    // Resolved boundaries (always N+1 entries for N color bins)
    readonly property var _bounds: {
        if (boundaries && boundaries.length >= 2)
            return boundaries
        var n = colors.length || 4
        var step = (vmax - vmin) / n
        var b = []
        for (var i = 0; i <= n; i++) b.push(vmin + i * step)
        return b
    }

    readonly property int _nBins: Math.max(0, _bounds.length - 1)

    function _fmt(v) {
        if (root.labelFunc) return root.labelFunc(v)
        if (v >= 1000) return Math.round(v).toString()
        if (v < 1)     return v.toFixed(1)
        return Math.round(v).toString()
    }

    // Which boundary values get a label
    readonly property var _activeTicks: {
        if (!ticks || ticks.length === 0) return _bounds
        return _bounds.filter(function(b) {
            for (var i = 0; i < ticks.length; i++)
                if (Math.abs(ticks[i] - b) < 1e-9) return true
            return false
        })
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

    // Labels positioned by value within [_bounds.first, _bounds.last]
    Item {
        id: labels
        anchors { top: bar.bottom; topMargin: 2 }
        width: root.width
        height: tickRepeater.itemAt(0) ? tickRepeater.itemAt(0).implicitHeight : 12

        readonly property double _lo:    root._bounds.length ? root._bounds[0] : 0
        readonly property double _hi:    root._bounds.length ? root._bounds[root._bounds.length - 1] : 1
        readonly property double _range: (_hi > _lo) ? (_hi - _lo) : 1

        Repeater {
            id: tickRepeater
            model: root._activeTicks.length

            Label {
                readonly property double _v: root._activeTicks[index]
                // Position by bin-boundary index so labels align with equal-width bins
                readonly property int _boundIdx: {
                    for (var i = 0; i < root._bounds.length; i++)
                        if (Math.abs(root._bounds[i] - _v) < 1e-9) return i
                    return 0
                }
                readonly property double _frac: root._bounds.length > 1
                    ? _boundIdx / (root._bounds.length - 1) : 0
                x: Math.round(Math.min(_frac * labels.width, labels.width - implicitWidth))
                font.pixelSize: 9
                opacity: 0.7
                text: {
                    var s = root._fmt(_v)
                    if (index === root._activeTicks.length - 1 && root.units)
                        return s + " " + root.units
                    return s
                }
            }
        }
    }
}
