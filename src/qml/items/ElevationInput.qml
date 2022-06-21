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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15


StackLayout {

    property double valueMeter: NaN

    function setTexts() {
        if (isNaN(valueMeter)) {
            ft_d.text = ""
            m_d.text = ""
            return
        }

        ft_d.text = Math.round(valueMeter*3.281)
        m_d.text = Math.round(valueMeter)
    }

    Component.onCompleted: setTexts()
    onCurrentIndexChanged: setTexts()
    onValueMeterChanged: setTexts()

    RowLayout { // feet
        id: ft

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        TextField {
            id: ft_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            placeholderText: qsTr("undefined")
            validator: IntValidator {
                bottom: -3000
                top: 24000
            }
            color: (acceptableInput ? Material.foreground : "red")

            onEditingFinished: {
                if (ft_d.acceptableInput)
                    valueMeter = Number.fromLocaleString(Qt.locale(), ft_d.text)/3.281
                else
                    valueMeter = NaN
            }
        }
        Label { text: "ft" }
    }

    RowLayout { // meter
        id: d
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        TextField {
            id: m_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            placeholderText: qsTr("undefined")
            validator: IntValidator {
                bottom: -1000
                top: 8000
            }
            color: (acceptableInput ? Material.foreground : "red")

            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                if (ft_d.acceptableInput)
                    valueMeter = Number.fromLocaleString(Qt.locale(), m_d.text)
                else
                    valueMeter = NaN
            }
        }
        Label { text: "m" }
    }
}
