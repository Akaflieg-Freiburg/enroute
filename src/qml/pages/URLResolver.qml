/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
import QtQuick.Layouts
import QtWebView

import akaflieg_freiburg.enroute
import "../items"

Page {
    id: pg
    title: qsTr("Google Map Resolver")

    required property string mapURL

    header: StandardHeader {}

    WebView {
        id: webView

        anchors.fill: parent
        anchors.bottomMargin: SafeInsets.bottom

        url: pg.mapURL

        onUrlChanged: {
            console.log(url)
            if (FileExchange.processUrlOpenRequestQuiet(url)) {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }
    }

}
