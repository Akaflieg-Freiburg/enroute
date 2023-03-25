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

import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

Dialog {
    id: ctrDlg

    leftMargin: SafeInsets.left + font.pixelSize
    rightMargin: SafeInsets.right + font.pixelSize
    topMargin: SafeInsets.top + font.pixelSize
    bottomMargin: SafeInsets.bottom + font.pixelSize

    // We center the dialog manually, taking care of safe insets
    x: SafeInsets.left + (SafeInsets.wWidth-SafeInsets.left-SafeInsets.right-width)/2.0
    y: SafeInsets.top + (SafeInsets.wHeight-SafeInsets.top-SafeInsets.bottom-height)/2.0

    // Delays evaluation and prevents binding loops
    Binding on implicitWidth {
        value: 40*ctrDlg.font.pixelSize
        delayed: true    // Prevent intermediary values from being assigned
    }
    implicitWidth: 40*font.pixelSize
    height: Math.min(SafeInsets.wHeight-topMargin-bottomMargin, implicitHeight)

    parent: Overlay.overlay
    
} // Dialog
