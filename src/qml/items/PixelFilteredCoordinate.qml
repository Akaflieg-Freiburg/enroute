/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

import QtPositioning
import QtQuick

// Follows "source", but holds back changes that would move a map symbol by
// less than one pixel, such as the GPS jitter of a parked aircraft. Each
// change of a map symbol's coordinate starts a one-second animation, and Qt
// renders at full frame rate for as long as any animation runs.
//
// While inactive, "coordinate" is not updated at all: a map item requests a
// new frame on every coordinate change, even while it is hidden.
QtObject {
    id: filter

    property geoCoordinate source
    property real pixelPer10km
    property bool active: true

    // The filtered coordinate
    property geoCoordinate coordinate

    // True while "coordinate" jumps to the current position after the filter
    // was inactive. Behaviors on "coordinate" should not animate then, because
    // the previous value is stale.
    property bool snapping: false

    function sync() {
        if (!filter.active)
            return
        if (!filter.source.isValid || !filter.coordinate.isValid || !(filter.pixelPer10km > 0)) {
            filter.coordinate = filter.source
            return
        }
        if (filter.source.distanceTo(filter.coordinate)*filter.pixelPer10km >= 10000.0)
            filter.coordinate = filter.source
    }

    onSourceChanged: sync()
    onPixelPer10kmChanged: sync()
    onActiveChanged: {
        if (!filter.active)
            return
        filter.snapping = true
        filter.coordinate = filter.source
        filter.snapping = false
    }
}
