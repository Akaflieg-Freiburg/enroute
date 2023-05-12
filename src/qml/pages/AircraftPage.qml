/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: aircraftPage
    title: qsTr("Aircraft")

    // Required Properties
    required property var stackView

    // Static objects, used to call static functions
    property speed staticSpeed
    property volumeFlow staticVolumeFlow

    header: PageHeader {

        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
                id: backButton

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter

                icon.source: "/icons/material/ic_arrow_back.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    aircraftPage.stackView.pop()
                }
            }

        Label {
                id: lbl

                anchors.verticalCenter: parent.verticalCenter

                anchors.left: parent.left
                anchors.leftMargin: 72
                anchors.right: headerMenuToolButton.left

                text: aircraftPage.stackView.currentItem.title
                elide: Label.ElideRight
                font.pixelSize: 20
                verticalAlignment: Qt.AlignVCenter
            }

        ToolButton {
                id: headerMenuToolButton

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                icon.source: "/icons/material/ic_more_vert.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    headerMenuX.open()
                }

                AutoSizingMenu {
                    id: headerMenuX
                    cascade: true

                    MenuItem {
                        text: qsTr("View Library…")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            aircraftPage.stackView.push("AircraftLibrary.qml")
                        }
                    }

                    MenuItem {
                        text: qsTr("Save to library…")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            dlgLoader.active = false
                            dlgLoader.source = "../dialogs/AircraftSaveDialog.qml"
                            dlgLoader.active = true
                        }
                    }
                }
        }
    }

    DecoratedScrollView {
        id: acftTab
        anchors.fill: parent
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right
        anchors.bottomMargin: SafeInsets.bottom

        contentWidth: width
        clip: true

        // If virtual keyboard come up, make sure that the focused element is visible
        onHeightChanged: {
            if (activeFocusControl != null) {
                contentItem.contentY = activeFocusControl.y - font.pixelSize
            }
        }

        GridLayout {
            id: aircraftTab
            anchors.left: parent.left
            anchors.leftMargin: acftTab.font.pixelSize
            anchors.right: parent.right
            anchors.rightMargin: acftTab.font.pixelSize

            columns: 3

            Rectangle {
                Layout.columnSpan: 3
                Layout.preferredHeight: acftTab.font.pixelSize
            }
            Label {
                text: qsTr("Name")
                Layout.columnSpan: 3
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }


            Label {
                text: qsTr("Name")
                Layout.alignment: Qt.AlignBaseline
            }
            MyTextField {
                id: name
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: horizontalUOM

                onEditingFinished: {
                    Navigator.aircraft.name = text
                    horizontalUOM.focus = true
                }
                text: Navigator.aircraft.name
            }

            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("Units")
                Layout.columnSpan: 3
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }

            WordWrappingItemDelegate {
                id: ios_horizontalUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                visible: (Qt.platform.os === "ios")

                text: {
                    var unitString = qsTr("Nautical Miles")
                    if (Navigator.aircraft.horizontalDistanceUnit === Aircraft.Kilometer) {
                        unitString = qsTr("Kilometers")
                    }
                    if (Navigator.aircraft.horizontalDistanceUnit === Aircraft.StatuteMile) {
                        unitString = qsTr("Statute Miles")
                    }
                    return qsTr("Horizontal Distances") +
                            '<br><font color="#606060" size="2">' +
                            qsTr("Currently using: %1").arg(unitString) +
                            '</font>'
                }
                icon.source: "/icons/material/ic_arrow_forward.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    horizontalUOMDialog.open()
                }
                CenteringDialog {
                    id: horizontalUOMDialog

                    title: qsTr("Horizontal Distances")
                    standardButtons: Dialog.Ok|Dialog.Cancel

                    DecoratedScrollView{
                        anchors.fill: parent
                        contentWidth: availableWidth // Disable horizontal scrolling


                        // Delays evaluation and prevents binding loops
                        Binding on implicitHeight {
                            value: cl.implicitHeight
                            delayed: true    // Prevent intermediary values from being assigned
                        }

                        clip: true

                        ColumnLayout {
                            id: cl
                            width: horizontalUOMDialog.availableWidth

                            Label{
                                Layout.fillWidth: true
                                text: qsTr("Choose the preferred units of measurement for this aircraft. The units also apply to horizontal speed indications.")
                                wrapMode: Text.Wrap
                            }
                            WordWrappingRadioDelegate {
                                id: a
                                Layout.fillWidth: true
                                text: qsTr("Nautical Miles")
                            }
                            WordWrappingRadioDelegate {
                                id: b
                                Layout.fillWidth: true
                                text: qsTr("Kilometers")
                            }
                            WordWrappingRadioDelegate {
                                id: c
                                Layout.fillWidth: true
                                text: qsTr("Statute Miles")
                            }
                        }
                    }

                    onAboutToShow: {
                        a.checked = Navigator.aircraft.horizontalDistanceUnit === Aircraft.NauticalMile
                        b.checked = Navigator.aircraft.horizontalDistanceUnit === Aircraft.Kilometer
                        c.checked = Navigator.aircraft.horizontalDistanceUnit === Aircraft.StatuteMile
                    }

                    onAccepted: {
                        if (a.checked)
                            Navigator.aircraft.horizontalDistanceUnit = Aircraft.NauticalMile
                        if (b.checked)
                            Navigator.aircraft.horizontalDistanceUnit = Aircraft.Kilometer
                        if (c.checked)
                            Navigator.aircraft.horizontalDistanceUnit = Aircraft.StatuteMile
                    }

                }
            }

            WordWrappingItemDelegate {
                id: ios_verticalUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                visible: (Qt.platform.os === "ios")

                text: {
                    var unitString = qsTr("Feet")
                    if (Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters) {
                        unitString = qsTr("Meters")
                    }
                    return qsTr("Vertical Distances") +
                            '<br><font color="#606060" size="2">' +
                            qsTr("Currently using: %1").arg(unitString) +
                            '</font>'
                }
                icon.source: "/icons/material/ic_arrow_upward.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    verticalUOMDialog.open()
                }
                CenteringDialog {
                    id: verticalUOMDialog

                    title: qsTr("Vertical Distances")
                    standardButtons: Dialog.Ok|Dialog.Cancel

                    DecoratedScrollView{
                        anchors.fill: parent
                        contentWidth: availableWidth // Disable horizontal scrolling


                        // Delays evaluation and prevents binding loops
                        Binding on implicitHeight {
                            value: cl1.implicitHeight
                            delayed: true    // Prevent intermediary values from being assigned
                        }

                        clip: true

                        ColumnLayout {
                            id: cl1
                            width: verticalUOMDialog.availableWidth

                            Label{
                                Layout.fillWidth: true
                                text: qsTr("Choose the preferred units of measurement for this aircraft. The units also apply to vertical speed indications.")
                                wrapMode: Text.Wrap
                            }
                            WordWrappingRadioDelegate {
                                id: a1
                                Layout.fillWidth: true
                                text: qsTr("Feet")
                            }
                            WordWrappingRadioDelegate {
                                id: b1
                                Layout.fillWidth: true
                                text: qsTr("Meters")
                            }
                        }
                    }

                    onAboutToShow: {
                        a1.checked = Navigator.aircraft.verticalDistanceUnit === Aircraft.Feet
                        b1.checked = Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters
                    }

                    onAccepted: {
                        if (a1.checked)
                            Navigator.aircraft.verticalDistanceUnit = Aircraft.Feet
                        if (b1.checked)
                            Navigator.aircraft.verticalDistanceUnit = Aircraft.Meters
                    }

                }
            }

            WordWrappingItemDelegate {
                id: ios_volumeUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                visible: (Qt.platform.os === "ios")

                text: {
                    var unitString = qsTr("Liters")
                    if (Navigator.aircraft.FuelConsumptionUnit === Aircraft.LiterPerHour) {
                        unitString = qsTr("Gallons")
                    }
                    return qsTr("Volume") +
                            '<br><font color="#606060" size="2">' +
                            qsTr("Currently using: %1").arg(unitString) +
                            '</font>'
                }
                icon.source: "/icons/material/ic_gas.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    volumeUOMDialog.open()
                }
                CenteringDialog {
                    id: volumeUOMDialog

                    title: qsTr("Volumes")
                    standardButtons: Dialog.Ok|Dialog.Cancel

                    DecoratedScrollView{
                        anchors.fill: parent
                        contentWidth: availableWidth // Disable horizontal scrolling


                        // Delays evaluation and prevents binding loops
                        Binding on implicitHeight {
                            value: cl2.implicitHeight
                            delayed: true    // Prevent intermediary values from being assigned
                        }

                        clip: true

                        ColumnLayout {
                            id: cl2
                            width: volumeUOMDialog.availableWidth

                            Label{
                                Layout.fillWidth: true
                                text: qsTr("Choose the preferred units of measurement for this aircraft.")
                                wrapMode: Text.Wrap
                            }
                            WordWrappingRadioDelegate {
                                id: a2
                                Layout.fillWidth: true
                                text: qsTr("Liters")
                            }
                            WordWrappingRadioDelegate {
                                id: b2
                                Layout.fillWidth: true
                                text: qsTr("Gallons")
                            }
                        }
                    }

                    onAboutToShow: {
                        a2.checked = Navigator.aircraft.fuelConsumptionUnit === Aircraft.LiterPerHour
                        b2.checked = Navigator.aircraft.fuelConsumptionUnit === Aircraft.GallonPerHour
                    }

                    onAccepted: {
                        if (a2.checked)
                            Navigator.aircraft.fuelConsumptionUnit = Aircraft.LiterPerHour
                        if (b2.checked)
                            Navigator.aircraft.fuelConsumptionUnit = Aircraft.GallonPerHour
                    }

                }
            }

            Label {
                text: qsTr("Horizontal")
                Layout.alignment: Qt.AlignBaseline
                visible: (Qt.platform.os !== "ios")
            }
            ComboBox {
                id: horizontalUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: verticalUOM
                visible: (Qt.platform.os !== "ios")

                Component.onCompleted: {
                    if (Navigator.aircraft.horizontalDistanceUnit === Aircraft.Kilometer) {
                        currentIndex = 1
                        return
                    }
                    if (Navigator.aircraft.horizontalDistanceUnit === Aircraft.StatuteMile) {
                        currentIndex = 2
                        return
                    }
                    currentIndex = 0
                }
                onActivated: Navigator.aircraft.horizontalDistanceUnit = currentIndex

                model: [ qsTr("Nautical Miles"), qsTr("Kilometers"), qsTr("Statute Miles") ]
            }
            Label {
                text: qsTr("Vertical")
                Layout.alignment: Qt.AlignBaseline
                visible: (Qt.platform.os !== "ios")
            }
            ComboBox {
                id: verticalUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: volumeUOM
                visible: (Qt.platform.os !== "ios")

                Component.onCompleted: {
                    if (Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters) {
                        currentIndex = 1
                        return
                    }
                    currentIndex = 0
                }
                onActivated: Navigator.aircraft.verticalDistanceUnit = currentIndex

                model: [ qsTr("Feet"), qsTr("Meters") ]
            }
            Label {
                text: qsTr("Volume")
                Layout.alignment: Qt.AlignBaseline
                visible: (Qt.platform.os !== "ios")
            }
            ComboBox {
                id: volumeUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: cruiseSpeed
                visible: (Qt.platform.os !== "ios")

                Component.onCompleted: {
                    if (Navigator.aircraft.fuelConsumptionUnit === Aircraft.GallonPerHour) {
                        currentIndex = 1
                        return
                    }
                    currentIndex = 0
                }
                onActivated: Navigator.aircraft.fuelConsumptionUnit = currentIndex

                model: [ qsTr("Liters"), qsTr("U.S. Gallons") ]
            }

            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("True Airspeed")
                Layout.columnSpan: 3
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }

            Label {
                id: colorGlean
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            MyTextField {
                id: cruiseSpeed
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: descentSpeed

                validator: DoubleValidator {
                    bottom: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        Navigator.aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        Navigator.aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        Navigator.aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!Navigator.aircraft.cruiseSpeed.isFinite()) {
                        return ""
                    }
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(Navigator.aircraft.cruiseSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(Navigator.aircraft.cruiseSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(Navigator.aircraft.cruiseSpeed.toMPH()).toString()
                    }

                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }

            Label {
                text: qsTr("Descent")
                Layout.alignment: Qt.AlignBaseline
            }
            MyTextField {
                id: descentSpeed
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: minimumSpeed

                validator: DoubleValidator {
                    bottom: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation

                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        Navigator.aircraft.descentSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        Navigator.aircraft.descentSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        Navigator.aircraft.descentSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!Navigator.aircraft.descentSpeed.isFinite()) {
                        return ""
                    }
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(Navigator.aircraft.descentSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(Navigator.aircraft.descentSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(Navigator.aircraft.descentSpeed.toMPH()).toString()
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }

            Label {
                text: qsTr("Minimum")
                Layout.alignment: Qt.AlignBaseline
            }
            MyTextField {
                id: minimumSpeed
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: fuelConsumption

                validator: DoubleValidator {
                    bottom: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Navigator.aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return Navigator.aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return Navigator.aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        Navigator.aircraft.minimumSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        Navigator.aircraft.minimumSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        Navigator.aircraft.minimumSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!Navigator.aircraft.minimumSpeed.isFinite()) {
                        return ""
                    }
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(Navigator.aircraft.minimumSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(Navigator.aircraft.minimumSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(Navigator.aircraft.minimumSpeed.toMPH()).toString()
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(Navigator.aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }

            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("Fuel Consumption")
                Layout.columnSpan: 3
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }

            Label {
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            MyTextField {
                id: fuelConsumption
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: name
                rightPadding: 30

                validator: DoubleValidator {
                    bottom: {
                        switch(Navigator.aircraft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return Navigator.aircraft.minValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return Navigator.aircraft.minValidFuelConsumption.toGPH()
                        }
                    }
                    top: {
                        switch(Navigator.aircraft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return Navigator.aircraft.maxValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return Navigator.aircraft.maxValidFuelConsumption.toGPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(Navigator.aircraft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        Navigator.aircraft.fuelConsumption = aircraftPage.staticVolumeFlow.fromLPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.GallonPerHour:
                        Navigator.aircraft.fuelConsumption = aircraftPage.staticVolumeFlow.fromGPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!Navigator.aircraft.fuelConsumption.isFinite()) {
                        return ""
                    }
                    switch(Navigator.aircraft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        return Navigator.aircraft.fuelConsumption.toLPH().toLocaleString(Qt.locale(), 'f', 1)
                    case Aircraft.GallonPerHour:
                        return Navigator.aircraft.fuelConsumption.toGPH().toLocaleString(Qt.locale(), 'f', 1)
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(Navigator.aircraft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        return "l/h";
                    case Aircraft.GallonPerHour:
                        return "gal/h";
                    }
                }
            }

            Rectangle {
                Layout.columnSpan: 3
                Layout.preferredHeight: acftTab.font.pixelSize
            }

        }

    }

    Loader {
        id: dlgLoader
        anchors.fill: parent

        onLoaded: {
            item.modal = true
            item.open()
        }
    }
} // Page
