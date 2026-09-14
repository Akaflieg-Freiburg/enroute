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

import QtQuick
import QtQuick.Layouts

// This is the text field that goes on top of a filterable list. It is a
// MyTextField, and therefore inherits the clear button and the iOS input method
// workaround.
//
// Bind list models to the property 'filter', never to 'displayText'. On iOS 17,
// displayText bounces many times per keystroke, and re-evaluating a filter over
// hundreds of list rows on every bounce makes the GUI stutter.

MyTextField {
    id: filterField

    placeholderText: qsTr("Filter by Name")

    // Once the field has focus or content, the Material style lifts the
    // placeholder into a small floating label that straddles the top of the
    // frame -- and draws it OUTSIDE this item, because topInset is zero unless
    // the field clips. Without room above, that label lands on whatever sits
    // there (a TabBar, say). Reserve a line's worth of space. Call sites inside
    // a Layout inherit this; those positioned by anchors set anchors.topMargin.
    Layout.topMargin: font.pixelSize

    // Debounced version of displayText.
    property string filter: ""

    // The list that this field filters. When set, pressing Return activates the
    // list's first entry, as if it had been clicked, so that typing a few
    // letters and hitting Return opens the top hit without reaching for the
    // mouse. Leave unset for lists whose rows have no single primary action.
    // A call site that needs something else on Return defines its own
    // onAccepted, which replaces the handler below.
    property DecoratedListView listView: null

    onAccepted: {
        if (filterField.listView !== null) {
            filterField.listView.activateFirstItem()
        }
    }

    // Some list models have no change notification, so that their bindings do
    // not re-evaluate when the underlying data changes. Librarian.entries() is
    // the typical example. Lists using such a model mention reloadTrigger in
    // their model binding and call reload() after adding, renaming or removing
    // an entry.
    property int reloadTrigger: 0
    function reload() { filterField.reloadTrigger += 1 }

    onDisplayTextChanged: {
        // Clearing takes effect at once. Debouncing it would leave a stale
        // filter behind for another 200ms, which is visible when a dialog
        // calls clear() on re-open.
        if (filterField.displayText === "") {
            debounceTimer.stop()
            filterField.filter = ""
        } else {
            debounceTimer.restart()
        }
    }

    Timer {
        id: debounceTimer

        interval: 200
        onTriggered: filterField.filter = filterField.displayText
    }
}
