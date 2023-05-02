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
    required property var dialogLoader

    // Static objects, used to call static functions
    property speed staticSpeed
    property volumeFlow staticVolumeFlow

    header: ColoredToolBar {

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
                    headerMenuX.popup()
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
                            aircraftPage.dialogLoader.active = false
                            aircraftPage.dialogLoader.source = "dialogs/AircraftSaveDialog.qml"
                            aircraftPage.dialogLoader.active = true
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
                contentItem.contentY = activeFocusControl.y
            }
        }

        GridLayout {
            id: aircraftTab
            anchors.left: parent.left
            anchors.leftMargin: acftTab.font.pixelSize
            anchors.right: parent.right
            anchors.rightMargin: acftTab.font.pixelSize

            columns: 4


            Rectangle {
                Layout.columnSpan: 4
                Layout.preferredHeight: acftTab.font.pixelSize
            }
            Label {
                text: qsTr("Name")
                Layout.columnSpan: 4
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
//                color: Material.accent
            }


            Label {
                text: qsTr("Name")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
                id: name
                Layout.columnSpan: 3
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
                Layout.columnSpan: 4
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
//                color: Material.accent
            }

            Label {
                text: qsTr("Horizontal")
                Layout.alignment: Qt.AlignBaseline
            }
            ComboBox {
                id: horizontalUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: verticalUOM


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
            }
            ComboBox {
                id: verticalUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: volumeUOM

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
            }
            ComboBox {
                id: volumeUOM
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: cruiseSpeed

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
                Layout.columnSpan: 4
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
//                color: Material.accent
            }

            Label {
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
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
                color: (acceptableInput ? "" : "red")
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
            ToolButton {
                icon.source: "/icons/material/ic_clear.svg"
                Layout.alignment: Qt.AlignVCenter
                enabled: cruiseSpeed.text !== ""
                onClicked: {
                    Navigator.aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromKN(-1)
                    cruiseSpeed.clear()
                }
            }

            Label {
                text: qsTr("Descent")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
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
                color: (acceptableInput ? "" : "red")
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
            ToolButton {
                icon.source: "/icons/material/ic_clear.svg"
                Layout.alignment: Qt.AlignVCenter
                enabled: descentSpeed.text !== ""
                onClicked: {
                    Navigator.aircraft.descentSpeed = aircraftPage.staticSpeed.fromKN(-1)
                    descentSpeed.clear()
                }
            }

            Label {
                text: qsTr("Minimum")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
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
                color: (acceptableInput ? "" : "red")
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
            ToolButton {
                icon.source: "/icons/material/ic_clear.svg"
                Layout.alignment: Qt.AlignVCenter
                enabled: minimumSpeed.text !== ""
                onClicked: {
                    Navigator.aircraft.minimumSpeed = aircraftPage.staticSpeed.fromKN(-1)
                    minimumSpeed.clear()
                }
            }


            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("Fuel Consumption")
                Layout.columnSpan: 4
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
//                color: Material.accent
            }

            Label {
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
                id: fuelConsumption
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: name

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
                color: (acceptableInput ? "" : "red")
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
            ToolButton {
                icon.source: "/icons/material/ic_clear.svg"
                Layout.alignment: Qt.AlignVCenter
                enabled: fuelConsumption.text !== ""
                onClicked: {
                    Navigator.aircraft.fuelConsumption = aircraftPage.staticVolumeFlow.fromLPH(-1)
                    fuelConsumption.clear()
                }
            }

        }

    }

} // Page
