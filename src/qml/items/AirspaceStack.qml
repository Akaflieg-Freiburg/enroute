/***************************************************************************
 *   Copyright (C) 2026 by Simon Schneider, Stefan Kebekus                 *
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
import QtQuick.Layouts
import QtQuick.Shapes

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound


/* Graphical view of the airspaces above one point, used by the dialog
 * WaypointDescription.
 *
 * Every airspace is drawn as a box, in the styling that the moving map uses for
 * its category. Boxes are arranged on a grid of altitude bands: the bounds of
 * all airspaces cut the diagram into bands, and every airspace covers the bands
 * between its own floor and ceiling. Airspaces that overlap in altitude sit in
 * different columns, so an airspace that contains others shows up as one tall
 * box beside the stack it encloses. FIR and FIS are kept in the rightmost
 * columns.
 *
 * The bands are of equal height: this is not an altitude scale, and the height
 * of a box says nothing about how much airspace it covers. Bounds are shown
 * exactly as published, and nothing derived from them is ever displayed.
 *
 * A box carries its category and its two bounds, nothing else. The names - and
 * with them any frequency to monitor - live in AirspaceList, the other tab of
 * the airspace section, where a row has the full width of the dialog and no
 * text can be cut.
 *
 * No text in this view is elided or cut either. The band height is therefore
 * measured, not chosen: for every box, invisible copies of its texts are laid
 * out at the width that the box will have, and the band height is the largest
 * height that any box needs, divided by the number of bands that the box
 * covers. A bound that has to wrap in a narrow column therefore still fits.
 *
 * The diagram ends at the airspace altitude limit from the settings, the same
 * limit that the moving map applies. Whenever that limit actually cuts the
 * diagram - because an airspace reaches beyond it, or lies entirely above it -
 * the limit is drawn as a dashed line with its value, so that the view is never
 * silently truncated.
 *
 * Arranging the boxes does need to know which airspace lies above which, and
 * that ordering is taken from the published numbers alone: flight levels as
 * hundreds of feet, AGL and MSL figures at face value. That is a heuristic. It
 * needs neither QNH nor terrain elevation, and it is used for the layout only.
 * Airspaces whose bounds cannot be read that way are listed underneath the
 * diagram rather than placed at a height that would be a guess.
 */

ColumnLayout {
    id: stack

    /*! \brief Airspaces to show
     *
     * Airspaces at one position, as delivered by
     * GeoMapProvider.airspacesAtPosition().
     */
    required property var airspaces

    // Text metric for spacings
    FontMetrics {
        id: fm
    }

    // Height of one line of text, measured on a real Label rather than computed
    // from font metrics, which do not include the line spacing that a Label
    // adds.
    Label {
        id: lineMetric

        visible: false
        text: "Wg"
    }

    readonly property real lineHeight: lineMetric.implicitHeight

    // Padding inside a box. Clears the 6px band that the box draws as its
    // border.
    readonly property real boxPadding: Math.max(fm.height*0.3, 7)

    // Gap between neighbouring columns. Boxes above each other deliberately
    // touch: a shared edge is a shared bound.
    readonly property real columnGap: fm.height*0.4

    // Upper end of the diagram, in feet, taken from the airspace altitude limit
    // in the settings. A non-finite limit means that all airspaces are shown.
    readonly property real altitudeCapFT: {
        const limit = GlobalSettings.airspaceAltitudeLimit
        if (limit.isFinite()) {
            return limit.toFeet()
        }
        return Number.POSITIVE_INFINITY
    }

    readonly property var plan: stack.planFor(stack.airspaces, stack.altitudeCapFT)

    readonly property real columnWidth: stack.width/Math.max(stack.plan.columnCount, 1)

    // Width available for text inside a box of the diagram, and inside a
    // full-width box below it. Rounded down, so that a real box is never
    // narrower than the width at which its text was measured.
    readonly property real boxTextWidth: Math.max(Math.floor(stack.columnWidth - stack.columnGap - 2*stack.boxPadding), 1)
    readonly property real fullTextWidth: Math.max(Math.floor(stack.width - 2*stack.boxPadding), 1)

    // A band always holds at least the three lines of a box whose texts fit on
    // one line each: ceiling, category, floor.
    readonly property real minimumBandHeight: 3*stack.lineHeight + 2*stack.boxPadding

    // Height of one altitude band, written by updateBandHeight() from the
    // measured texts.
    property real bandHeight: stack.minimumBandHeight

    onBoxTextWidthChanged: stack.updateBandHeight()

    spacing: fm.height*0.25

    /* Recomputes bandHeight from the measured texts.
     *
     * Called whenever a measurement changes. The result is computed from
     * scratch every time, so that the bands also become flatter again when the
     * dialog grows wider or the airspaces change.
     */
    function updateBandHeight() {
        let needed = stack.minimumBandHeight
        for (var i = 0; i < textMetrics.count; i++) {
            const metric = textMetrics.itemAt(i)
            const box = stack.plan.boxes[i]
            if ((metric === null) || (box === undefined)) {
                continue
            }
            // A box that covers several bands may spread its text over them
            needed = Math.max(needed, (metric.implicitHeight + 2*stack.boxPadding)/box.rowSpan)
        }
        stack.bandHeight = needed
    }

    /* Position of a bound on the layout axis, in feet, or NaN if the bound
     * cannot be read. Mirrors the bound syntax of
     * GeoMaps::Airspace::estimateBoundMSL(), but without terrain elevation and
     * QNH: the result orders airspaces, it is never shown.
     */
    function boundRank(bound: string) : real {
        const text = bound.replace(/\s+/g, " ").trim().toUpperCase()

        if (text === "UNL" || text === "UNLIMITED") {
            return Number.MAX_SAFE_INTEGER
        }
        if (text.startsWith("FL")) {
            return 100*parseFloat(text.substring(2))
        }
        if (text.endsWith("AGL")) {
            return parseFloat(text.slice(0, -3))
        }
        if (text.endsWith("GND") || text === "SFC") {
            return 0
        }
        return parseFloat(text)
    }

    /* Categories that enclose the others and are therefore kept to the right of
     * the diagram.
     */
    function isEnclosingCategory(CAT: string) : bool {
        return (CAT === "FIR") || (CAT === "FIS")
    }

    /* Puts an airspace into the leftmost column at or right of firstColumn in
     * which it overlaps no other airspace, and records the column in the entry.
     */
    function assignColumn(columns: var, entry: var, firstColumn: int) {
        let column = firstColumn
        while (true) {
            if (columns[column] === undefined) {
                columns[column] = []
            }
            if (columns[column].every((other) => (entry.lower >= other.upper) || (entry.upper <= other.lower))) {
                columns[column].push(entry)
                entry.column = column
                return
            }
            column++
        }
    }

    /* Turns airspaces into the boxes of the diagram.
     *
     * Returns an object with
     * - bandCount:   number of altitude bands in the diagram,
     * - columnCount: number of columns needed,
     * - boxes:       one entry {airspace, row, rowSpan, column} per airspace,
     * - unplaceable: airspaces whose bounds could not be read,
     * - capped:      true if capFT cut the diagram, so that the limit needs to
     *                be drawn.
     */
    function planFor(airspaceList: var, capFT: real) : var {
        let entries = []
        let unplaceable = []
        let capped = false
        for (var i = 0; i < airspaceList.length; i++) {
            const airspace = airspaceList[i]
            const lower = stack.boundRank(airspace.lowerBound)
            const upper = stack.boundRank(airspace.upperBound)
            if (!Number.isFinite(lower) || !Number.isFinite(upper) || (upper <= lower)) {
                unplaceable.push(airspace)
                continue
            }
            if (lower >= capFT) {
                // Entirely above the limit, just as on the moving map
                capped = true
                continue
            }
            if (upper > capFT) {
                capped = true
                entries.push({airspace: airspace, lower: lower, upper: capFT, column: 0})
                continue
            }
            entries.push({airspace: airspace, lower: lower, upper: upper, column: 0})
        }

        // Every bound cuts the diagram. The bands are the gaps in between.
        let levels = []
        for (const entry of entries) {
            if (!levels.includes(entry.upper)) {
                levels.push(entry.upper)
            }
            if (!levels.includes(entry.lower)) {
                levels.push(entry.lower)
            }
        }
        levels.sort((a, b) => b - a)

        // An airspace joins the first column in which it overlaps nothing;
        // airspaces that merely touch may share a column. Assigning them from
        // the ground up keeps the number of columns as small as possible.
        //
        // FIR and FIS are assigned last, so that they always sit in the
        // rightmost columns: they enclose everything else, and putting them to
        // the side keeps the airspaces that matter for flying on the left.
        const byFloor = (a, b) => (a.lower - b.lower) || (a.upper - b.upper)
        const enclosing = entries.filter((entry) => stack.isEnclosingCategory(entry.airspace.CAT)).sort(byFloor)
        const others = entries.filter((entry) => !stack.isEnclosingCategory(entry.airspace.CAT)).sort(byFloor)

        let columns = []
        for (const entry of others) {
            stack.assignColumn(columns, entry, 0)
        }
        const firstEnclosingColumn = columns.length
        for (const entry of enclosing) {
            stack.assignColumn(columns, entry, firstEnclosingColumn)
        }

        let boxes = []
        for (const entry of entries) {
            const row = levels.indexOf(entry.upper)
            boxes.push({airspace: entry.airspace,
                        row: row,
                        rowSpan: levels.indexOf(entry.lower) - row,
                        column: entry.column})
        }

        return {bandCount: Math.max(levels.length-1, 0),
                columnCount: columns.length,
                boxes: boxes,
                unplaceable: unplaceable,
                capped: capped}
    }

    // One bound of an airspace, as published, in the unit that the user has
    // chosen. Wraps instead of eliding: a bound must never be cut.
    component BoundLabel: Label {
        id: boundLabel

        required property var airspace
        required property bool upper

        // The width comes from the anchors of the use site: Label rejects a
        // write to implicitWidth, and an explicit width would be undefined
        // behaviour wherever a layout manages the item.
        horizontalAlignment: Text.AlignRight
        wrapMode: Text.WordWrap

        text: {
            if (Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters) {
                return boundLabel.upper ? boundLabel.airspace.upperBoundMetric : boundLabel.airspace.lowerBoundMetric
            }
            return boundLabel.upper ? boundLabel.airspace.upperBound : boundLabel.airspace.lowerBound
        }
    }

    // Category of an airspace, in the middle of its box. The name is not shown
    // here: it belongs to the list view, which has room for it.
    component CatLabel: Label {
        id: catLabel

        required property var airspace

        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        font.bold: true

        text: catLabel.airspace.CAT
    }

    // The texts of one box, stacked up, at a given width. Used invisibly to
    // measure how much height a box needs; the box itself uses the same labels,
    // but pins them to its edges.
    component AirspaceText: Item {
        id: airspaceText

        required property var airspace
        required property real lineWidth

        implicitWidth: airspaceText.lineWidth
        implicitHeight: upperText.height + catText.height + lowerText.height

        BoundLabel {
            id: upperText

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            airspace: airspaceText.airspace
            upper: true
        }

        CatLabel {
            id: catText

            anchors.top: upperText.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            airspace: airspaceText.airspace
        }

        BoundLabel {
            id: lowerText

            anchors.top: catText.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            airspace: airspaceText.airspace
            upper: false
        }
    }

    // A single airspace. Outline, band and fill are the ones that the legend
    // box of the former airspace list used, so the boxes keep matching the
    // moving map.
    component AirspaceBox: Item {
        id: box

        required property var airspace

        /*! \brief Padding between the box and its text
         *
         * Set from the outside: deriving the size of the box from the text
         * inside it, and the size of the text from the box, is what makes Qt
         * Quick Layouts recurse.
         */
        required property real padding

        // Drawing style of the category, shared with the legend of AirspaceList
        // through the Global singleton, so that both match the moving map.
        readonly property color outlineColor: Global.airspaceOutlineColor(box.airspace.CAT)
        readonly property int outlineStyle: Global.airspaceOutlineIsSolid(box.airspace.CAT) ? ShapePath.SolidLine : ShapePath.DashLine
        readonly property var outlineDashPattern: Global.airspaceDashPattern(box.airspace.CAT)
        readonly property color bandColor: Global.airspaceBandColor(box.airspace.CAT)
        readonly property color fillColor: Global.airspaceFillColor(box.airspace.CAT)

        Shape {
            anchors.fill: parent

            ShapePath {
                strokeWidth: 2
                fillColor: "transparent"
                strokeColor: box.outlineColor
                strokeStyle: box.outlineStyle
                dashPattern: box.outlineDashPattern

                startX: 1; startY: 1
                PathLine { x: 1;           y: box.height-1 }
                PathLine { x: box.width-1; y: box.height-1 }
                PathLine { x: box.width-1; y: 1 }
                PathLine { x: 1;           y: 1 }
            }
        }

        Rectangle {
            anchors.fill: parent

            border.color: box.bandColor
            border.width: 6
            color: box.fillColor
        }

        // The ceiling sits at the upper edge and the floor at the lower edge,
        // because a bound belongs to the edge that it describes. The category
        // takes the space in between, which is never less than it needs: the
        // band height was measured from these very texts.
        Item {
            anchors.fill: parent
            anchors.margins: box.padding

            BoundLabel {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                airspace: box.airspace
                upper: true
            }

            CatLabel {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                airspace: box.airspace
            }

            BoundLabel {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                airspace: box.airspace
                upper: false
            }
        }
    }

    // Measures the texts of every box at the width that the box will have.
    // These items are invisible: they measure, they never render, and Qt Quick
    // Layouts leaves them out of the layout.
    Repeater {
        id: textMetrics

        model: stack.plan.boxes

        AirspaceText {
            id: textMetric

            required property var modelData

            visible: false
            airspace: textMetric.modelData.airspace
            lineWidth: stack.boxTextWidth

            onImplicitHeightChanged: stack.updateBandHeight()
            Component.onCompleted: stack.updateBandHeight()
        }
    }

    ColumnLayout { // Upper end of the diagram, whenever the limit cuts it
        id: capMarker

        Layout.fillWidth: true
        spacing: 0
        visible: stack.plan.capped

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            text: qsTr("Airspaces up to %1").arg(Navigator.aircraft.verticalDistanceToString(GlobalSettings.airspaceAltitudeLimit))
        }

        Item {
            // The Shape gets its width from the anchors, not from its own
            // implicit size: a Shape whose path spans the width of its parent
            // layout would make that layout's implicit width depend on itself.
            id: capLine

            Layout.fillWidth: true
            implicitHeight: 2

            Shape {
                anchors.fill: parent

                ShapePath {
                    strokeWidth: 2
                    strokeColor: Global.airspaceNeutral
                    strokeStyle: ShapePath.DashLine
                    dashPattern: [4, 4]
                    fillColor: "transparent"

                    startX: 0; startY: 1
                    PathLine { x: capLine.width; y: 1 }
                }
            }
        }
    }

    Item { // The diagram: bands run top-down, overlapping airspaces sit in columns
        id: diagram

        Layout.fillWidth: true
        implicitHeight: stack.plan.bandCount*stack.bandHeight

        Repeater {
            model: stack.plan.boxes

            AirspaceBox {
                id: placedBox

                required property var modelData

                airspace: placedBox.modelData.airspace
                padding: stack.boxPadding

                x: placedBox.modelData.column*stack.columnWidth + stack.columnGap/2
                y: placedBox.modelData.row*stack.bandHeight
                width: stack.columnWidth - stack.columnGap
                height: placedBox.modelData.rowSpan*stack.bandHeight
            }
        }
    }

    Repeater { // Airspaces whose bounds could not be read
        model: stack.plan.unplaceable

        AirspaceBox {
            id: unplacedBox

            required property var modelData

            airspace: unplacedBox.modelData
            padding: stack.boxPadding

            // These boxes stand on their own, so each one is as tall as its own
            // text: no band has to line up with them.
            Layout.fillWidth: true
            Layout.preferredHeight: unplacedText.implicitHeight + 2*stack.boxPadding

            AirspaceText {
                id: unplacedText

                visible: false
                airspace: unplacedBox.modelData
                lineWidth: stack.fullTextWidth
            }
        }
    }
}
