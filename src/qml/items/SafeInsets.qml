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

pragma Singleton

import QtQuick

/* Window-global safe-area margins, in device-independent pixels.
 *
 * This singleton is a compatibility shim that replaced the former per-platform
 * C++ implementations (issue #584). The values are sourced from Qt's SafeArea
 * attached property on the application window; main.qml pushes them into the
 * writable properties below. The bottom value additionally includes the
 * virtual keyboard on Android and iOS, matching the behavior of the old
 * platform code (see SafeArea.additionalMargins in main.qml).
 *
 * Unlike the attached SafeArea property, whose margins are relative to the
 * item it attaches to, these values are global constants for the whole
 * window. New code that styles items touching a window edge should prefer
 * Qt's SafeArea attached property directly.
 */
QtObject {
    property real bottom: 0.0
    property real left: 0.0
    property real right: 0.0
    property real top: 0.0
}
