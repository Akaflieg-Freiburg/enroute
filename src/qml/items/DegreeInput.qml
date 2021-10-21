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
import QtQuick.Layouts 1.15


StackLayout {

    property double value
    property double minValue
    property double maxValue

    function minutes() {
        absValue = Math.abs(value)
        fracValue = absValue-Math.floor(absValue)
        return Math.floor(fracValue*60.0)
    }

    function seconds() {
        absValue = Math.abs(value)
        fracValue = absValue-Math.floor(absValue)-minutes()/60.0
        return fracValue*60.0*60.0
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            text: value.toLocaleString(Qt.locale(), 'f', 10)
            validator: DoubleValidator {
                bottom: minValue
                top: maxValue
                notation: DoubleValidator.StandardNotation
            }
            onEditingFinished: value = text
        }
        Label { text: "°" }
    } // Degree

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            text: Math.trunc(value)
        }
        Label { text: "°" }

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            text: (value-Math.trunc(value))*60.0
        }
        Label { text: "'" }

    } // Degree and Minute

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
        }
        Label { text: "°" }

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
        }
        Label { text: "'" }

        TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
        }
        Label { text: "''" }
    } // Degree and Minute
}
