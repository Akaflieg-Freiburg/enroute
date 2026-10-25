/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: aircraftPage
    title: entryName

    // Name of the aircraft library entry that is edited. Every change is
    // written to the library right away.
    required property string entryName

    // Copy of the library entry
    property aircraft acft

    // Set once the library entry is loaded, so that loading does not write it back
    property bool loaded: false

    // Static objects, used to call static functions
    property speed staticSpeed
    property volumeFlow staticVolumeFlow

    header: StandardHeader {}

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
            if (ApplicationWindow.activeFocusControl != null) {
                contentItem.contentY = ApplicationWindow.activeFocusControl.y - font.pixelSize
            }
        }

        GridLayout {
            id: aircraftTab
            anchors.left: parent.left
            anchors.leftMargin: acftTab.font.pixelSize
            anchors.right: parent.right
            anchors.rightMargin: acftTab.font.pixelSize

            columns: 4

            // Aircraft
            Label {
                text: qsTr("Aircraft")
                Layout.columnSpan: 4
                Layout.topMargin: font.pixelSize/2
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }

            Label {
                text: qsTr("Name")
                Layout.alignment: Qt.AlignBaseline
                Layout.topMargin: font.pixelSize/2
            }
            MyTextField {
                id: name
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                Layout.topMargin: font.pixelSize/2
                KeyNavigation.tab: transponderCode
                placeholderText: qsTr("Callsign")

                onEditingFinished: aircraftPage.acft.name = text
                text: aircraftPage.acft.name
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    helpDialog.title = qsTr("Name")
                    helpDialog.text = "<p>"
                            + qsTr("Enter the callsign of your aircraft. Together with the Hex ID, the name is used to filter out your own aircraft when receiving traffic data from the Open Glider Network, preventing it from being displayed twice on the map.")
                            + "</p>"
                    helpDialog.open()
                }
            }

            Label {
                text: qsTr("Hex ID")
                Layout.alignment: Qt.AlignBaseline
                Layout.topMargin: font.pixelSize/2
            }
            MyTextField {
                id: transponderCode
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.topMargin: font.pixelSize/2
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5
                KeyNavigation.tab: cruiseSpeed
                placeholderText: qsTr("Transponder/FLARM ID")

                onEditingFinished: aircraftPage.acft.transponderCode = text.toUpperCase()
                text: aircraftPage.acft.transponderCode
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    helpDialog.title = qsTr("Hex ID")
                    helpDialog.text = "<p>"
                            + qsTr("Enter the “ICAO 24-bit address” of your transponder (e.g., “3D1C11”), the “FLARM Radio ID” of your FLARM device (e.g., “3EDE26”) or the “Open Glider Network Source ID” of your aircraft (e.g., “ICA3D1C11”, “FLR123456”). Together with the name, this identifier is used to filter out your own aircraft when receiving traffic data from the Open Glider Network, preventing it from being displayed twice on the map. If you do not use Open Glider Network traffic data, you can leave this field empty.")
                            + "</p><p>"
                            + qsTr("You can enter multiple codes separated by spaces if your aircraft has multiple identifiers. The comparison is case-insensitive.")
                            + "</p>"
                    helpDialog.open()
                }
            }

            CheckBox {
                id: pressureCheckBox

                Layout.columnSpan: 3
                Layout.fillWidth: true

                enabled: Sensors.pressureAltitude.isFinite()
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    if (checked)
                        pressureWarning.open()
                    else
                        aircraftPage.acft.cabinPressureEqualsStaticPressure = false
                }
                checked: aircraftPage.acft.cabinPressureEqualsStaticPressure
                text: qsTr("Use cabin pressure instead of static pressure for pressure altitude calculation.")
                Component.onCompleted: contentItem.wrapMode = Text.WordWrap
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    helpDialog.title = qsTr("Cabin/Altitude Pressure")
                    helpDialog.text = "<p>"
                            + qsTr("If this option is checked, Enroute Flight Navigation will use the pressure sensor of your mobile device to measure the pressure altitude and determine vertical distances to airspaces. This option is only available if your device has a pressure sensor.")
                            + "</p><p>"
                            + qsTr("If available, Enroute Flight Navigation will always use the pressure altitude provided by an external traffic data receiver instead of the pressure altitude calculated from the pressure sensor of your mobile device.")
                            + "</p>"
                    helpDialog.open()
                }
            }

            // Units
            Label {
                text: qsTr("Units")
                Layout.columnSpan: 4
                Layout.topMargin: font.pixelSize/2
                font.pixelSize: acftTab.font.pixelSize*1.2
                font.bold: true
            }

            Label {
                text: qsTr("Horizontal")
                Layout.alignment: Qt.AlignBaseline
            }
            ComboBox {
                id: horizontalUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: verticalUOM

                currentIndex: {
                    if (aircraftPage.acft.horizontalDistanceUnit === Aircraft.Kilometer) {
                        return 1
                    }
                    if (aircraftPage.acft.horizontalDistanceUnit === Aircraft.StatuteMile) {
                        return 2
                    }
                    return 0
                }
                onActivated: {
                    PlatformAdaptor.vibrateBrief()
                    aircraftPage.acft.horizontalDistanceUnit = currentIndex
                }

                model: [ qsTr("Nautical Miles"), qsTr("Kilometers"), qsTr("Statute Miles") ]
            }
            Item { }

            Label {
                text: qsTr("Vertical")
                Layout.alignment: Qt.AlignBaseline
            }
            ComboBox {
                id: verticalUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: volumeUOM

                currentIndex: {
                    if (aircraftPage.acft.verticalDistanceUnit === Aircraft.Meters) {
                        return 1
                    }
                    return 0
                }
                onActivated: {
                    PlatformAdaptor.vibrateBrief()
                    aircraftPage.acft.verticalDistanceUnit = currentIndex
                }

                model: [ qsTr("Feet"), qsTr("Meters") ]
            }
            Item { }

            Label {
                text: qsTr("Volume")
                Layout.alignment: Qt.AlignBaseline
            }
            ComboBox {
                id: volumeUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                KeyNavigation.tab: cruiseSpeed

                currentIndex: {
                    if (aircraftPage.acft.fuelConsumptionUnit === Aircraft.GallonPerHour) {
                        return 1
                    }
                    return 0
                }
                onActivated: {
                    PlatformAdaptor.vibrateBrief()
                    aircraftPage.acft.fuelConsumptionUnit = currentIndex
                }

                model: [ qsTr("Liters"), qsTr("U.S. Gallons") ]
            }
            Item { }

            // True Airspeed
            Label {
                text: qsTr("True Airspeed")
                Layout.columnSpan: 4
                Layout.topMargin: font.pixelSize
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
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Global.digitsInputHint
                onEditingFinished: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        aircraftPage.acft.cruiseSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        aircraftPage.acft.cruiseSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        aircraftPage.acft.cruiseSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!aircraftPage.acft.cruiseSpeed.isFinite()) {
                        return ""
                    }
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(aircraftPage.acft.cruiseSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(aircraftPage.acft.cruiseSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(aircraftPage.acft.cruiseSpeed.toMPH()).toString()
                    }

                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }
            Item { }

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
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation

                }
                inputMethodHints: Global.digitsInputHint
                onEditingFinished: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        aircraftPage.acft.descentSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        aircraftPage.acft.descentSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        aircraftPage.acft.descentSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!aircraftPage.acft.descentSpeed.isFinite()) {
                        return ""
                    }
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(aircraftPage.acft.descentSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(aircraftPage.acft.descentSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(aircraftPage.acft.descentSpeed.toMPH()).toString()
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }
            Item { }

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
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(aircraftPage.acft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return aircraftPage.acft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return aircraftPage.acft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return aircraftPage.acft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Global.digitsInputHint
                onEditingFinished: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        aircraftPage.acft.minimumSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        aircraftPage.acft.minimumSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        aircraftPage.acft.minimumSpeed = aircraftPage.staticSpeed.fromMPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!aircraftPage.acft.minimumSpeed.isFinite()) {
                        return ""
                    }
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(aircraftPage.acft.minimumSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(aircraftPage.acft.minimumSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(aircraftPage.acft.minimumSpeed.toMPH()).toString()
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(aircraftPage.acft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return "kn";
                    case Aircraft.Kilometer:
                        return "km/h";
                    case Aircraft.StatuteMile :
                        return "mph";
                    }
                }
            }
            Item { }

            // Fuel Consumption
            Label {
                text: qsTr("Fuel Consumption")
                Layout.columnSpan: 4
                Layout.topMargin: font.pixelSize
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
                        switch(aircraftPage.acft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return aircraftPage.acft.minValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return aircraftPage.acft.minValidFuelConsumption.toGPH()
                        }
                    }
                    top: {
                        switch(aircraftPage.acft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return aircraftPage.acft.maxValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return aircraftPage.acft.maxValidFuelConsumption.toGPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onEditingFinished: {
                    switch(aircraftPage.acft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        aircraftPage.acft.fuelConsumption = aircraftPage.staticVolumeFlow.fromLPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.GallonPerHour:
                        aircraftPage.acft.fuelConsumption = aircraftPage.staticVolumeFlow.fromGPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? colorGlean.color : "red")
                text: {
                    if (!aircraftPage.acft.fuelConsumption.isFinite()) {
                        return ""
                    }
                    switch(aircraftPage.acft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        return aircraftPage.acft.fuelConsumption.toLPH().toLocaleString(Qt.locale(), 'f', 1)
                    case Aircraft.GallonPerHour:
                        return aircraftPage.acft.fuelConsumption.toGPH().toLocaleString(Qt.locale(), 'f', 1)
                    }
                }
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(aircraftPage.acft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        return "l/h";
                    case Aircraft.GallonPerHour:
                        return "gal/h";
                    }
                }
            }
            Item { }

            Rectangle {
                Layout.columnSpan: 4
                Layout.preferredHeight: acftTab.font.pixelSize
            }
        }

    }

    Component.onCompleted: {
        var newAcft = acft.clone()
        var errorString = newAcft.loadFromJSON(Librarian.fullPath(Librarian.Aircraft, entryName))
        if (errorString !== "") {
            errorDialog.text = errorString
            errorDialog.open()
            return
        }
        acft = newAcft
        loaded = true
    }

    onAcftChanged: {
        if (!loaded)
            return
        var errorString = Librarian.saveAircraft(entryName, acft)
        if (errorString !== "") {
            errorDialog.text = errorString
            errorDialog.open()
        }
    }

    LongTextDialog {
        id: errorDialog

        modal: true
        title: qsTr("An Error Occurred…")
        standardButtons: Dialog.Ok
    }

    LongTextDialog {
        id: pressureWarning

        modal: true
        title: qsTr("Flight Safety Warning!")
        text: "<p>"
              + "<strong>" + qsTr("Know what you are doing!") + " </strong>"
              + qsTr("If you enable this option, Enroute Flight Navigation will use the pressure sensor of your mobile device to determine pressure altitude and vertical distances to airspaces.") + " "
              + qsTr("This is safety critical.") + " "
              + "</p>"
              + "<p><ul style='margin-left:-25px;'>"
              + "<li>" + qsTr("The pressure sensor of your device is probably not certified for use in aviation.") + "</li>"
              + "<li>" + qsTr("In typical GA aircraft, cabin pressure is not equal to static pressure.") + " "
              + qsTr("The precise difference depends on factors such as airspeed and configuration of ventilation and heating.") + "</li>"
              + "</ul></p><p>"
              + qsTr("Do not enable this option unless you convinced yourself that the data provided by your sensor is good enough for the intended use.") + " "
              + qsTr("Do not rely on data shown in this app.") + " "
              + qsTr("Always use an approved altimeter to judge vertical distance to airspaces.") + " "
              + "</p>"
              + "<p>"
              + qsTr("We strongly recommend connecting Enroute Flight Navigation to a proper traffic data receiver, such as a FLARM or ADS-B device, that provides static pressure data.")
              + "</p>"
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            aircraftPage.acft.cabinPressureEqualsStaticPressure = true
        }

        onRejected: {
            pressureCheckBox.checked = false
        }
    }

    LongTextDialog {
        id: helpDialog

        modal: true

        standardButtons: Dialog.Ok
    }

} // Page
