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

import QtGraphicalEffects 1.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

// This is a version of ItemDelegate that does word wrapping in the text

ItemDelegate {
    id: itemDelegate

    contentItem: RowLayout {
        height: Math.max(a.height, b.height)

        Image { // Icon
            id: a
            source: itemDelegate.icon.source
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24

            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Material.foreground
            }
        }

        Item { // Spacer
            width: 5
        }

        Label { // Text
            id: b
            Layout.fillWidth: true
            text: itemDelegate.text
            wrapMode: Text.Wrap
        }
    }

}
