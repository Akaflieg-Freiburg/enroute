/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import enroute 1.0

Dialog {
    id: dlg
    title: qsTr("Set Altimeter")
    focus: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    standardButtons: Dialog.Apply|Dialog.Cancel|Dialog.Reset

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        // The Label that we really want to show is wrapped into an Item. This allows
        // to set implicitHeight, and thus compute the implicitHeight of the Dialog
        // without binding loops
        Item {
            implicitHeight: lbl1.implicitHeight
            width: dlg.availableWidth

            Label {
                id: lbl1
                text: qsTr("If you have good satellite reception and if know your altitude precisely, you can set the satellite altimeter here.")
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignJustify
                wrapMode: Text.Wrap
                width: dlg.availableWidth
            }
        }

        // Input Line
        RowLayout {
            Label { text: qsTr("Altitude") }

            TextField {
                id: textField
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator{bottom: -1000; top: 20000;}
                Layout.fillWidth: true
                onAccepted: dlg.onApplied()
            }

            Label { text: qsTr("ft AMSL") }
        } // RowLayout

        // The Label that we really want to show is wrapped into an Item. This allows
        // to set implicitHeight, and thus compute the implicitHeight of the Dialog
        // without binding loops
        Item {
            implicitHeight: lbl2.implicitHeight
            width: dlg.availableWidth

            Label {
                id: lbl2
                text: satNav.hasAltitude ? qsTr("The current raw altimeter reading as reported by the satellite navigation system is %1 AMSL. "+
                                                "The corrected altitude is %2 AMSL.").arg(satNav.rawAltitudeInFeetAsString).arg(satNav.altitudeInFeetAsString)
                                         : qsTr("Insufficient satellite reception. Altimeter cannot be set.")
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignJustify
                wrapMode: Text.Wrap
                width: dlg.availableWidth
            }
        }
    }

    onApplied: {
        // Give feedback
        mobileAdaptor.vibrateBrief()

        // If we have no acceptable input, simply return
        if (!textField.acceptableInput)
            return

        // If we have no satellite altitude, complain and return
        if (!satNav.hasAltitude) {
            noAltimeterReading.open()
            return
        }

        // If the difference between the raw altitude and the user's setting is large, ask if we really want that.
        var newAlt = parseInt(textField.text)
        if (Math.abs(satNav.rawAltitudeInFeet-newAlt) > 1000) {
            largeDifference.open()
            return
        }

        // If all else fails, set the altitude
        satNav.altitudeInFeet = parseInt(textField.text)
        close()
    }

    onReset: {
        // Give feedback
        mobileAdaptor.vibrateBrief()

        // If we have no satellite altitude, complain and return
        if (!satNav.hasAltitude) {
            noAltimeterReading.open()
            return
        }

        satNav.altitudeInFeet = satNav.rawAltitudeInFeet
        close()
    }

    onRejected: {
        // Give feedback
        mobileAdaptor.vibrateBrief()
        close()
    }

    Component.onCompleted: {
        textField.text = satNav.altitudeInFeet
    }

    Connections {
        target: sensorGesture
        onDetected: close()
    }

    Dialog {
        id: noAltimeterReading

        anchors.centerIn: parent

        title: qsTr("Altimeter cannot be set")

        Label {
            text: qsTr("Insufficient satellite reception. Please try again once reception becomes better.")
            width: largeDifference.availableWidth
            wrapMode: Text.Wrap
        }

        standardButtons: Dialog.Ok

        modal: true

        onAccepted: {
            // Give feedback
            mobileAdaptor.vibrateBrief()
            close()
        }
    }


    Dialog {
        id: largeDifference

        anchors.centerIn: parent
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

        modal: true
        standardButtons: Dialog.Yes|Dialog.No
        title: qsTr("Really set %1 ft?").arg(parseInt(textField.text))

        Label {
            text: qsTr("The altitude reported by the satellite navigation system is %1 AMSL. That is a big difference.").arg(satNav.rawAltitudeInFeet)
            width: largeDifference.availableWidth
            wrapMode: Text.Wrap
        }

        onAccepted: {
            // Give feedback
            mobileAdaptor.vibrateBrief()
            satNav.altitudeInFeet = parseInt(textField.text)
            close()
            dlg.close()
        }

        onRejected: {
            // Give feedback
            mobileAdaptor.vibrateBrief()
            close()
        }
    }

} // Dialog
