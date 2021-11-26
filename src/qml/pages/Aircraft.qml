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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: aircraftPage
    title: qsTr("Aircraft")

    // Static objects, used to call static functions
    property var staticSpeed: global.navigator().aircraft.maxValidSpeed
    property var staticVolumeFlow: global.navigator().aircraft.maxValidFuelConsumption

    header: ToolBar {

        Material.foreground: "white"
        height: 60

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
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
                global.mobileAdaptor().vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                MenuItem {
                    text: qsTr("Open from library …")
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteOpenDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    text: qsTr("Save to library …")
                    enabled: (global.navigator().flightRoute.size > 0)
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteSaveDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    text: qsTr("View Library …")
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        highlighted = false
                        stackView.push("FlightRouteLibrary.qml")
                    }
                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Import …")
                    enabled: Qt.platform.os !== "android"
                    height: Qt.platform.os !== "android" ? undefined : 0

                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        highlighted = false

                        global.mobileAdaptor().importContent()
                    }
                }

                AutoSizingMenu {
                    title: Qt.platform.os === "android" ? qsTr("Share …") : qsTr("Export …")
                    enabled: (global.navigator().flightRoute.size > 0)

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            global.mobileAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = global.mobileAdaptor().exportContent(global.navigator().flightRoute.toGeoJSON(), "application/geo+json", global.navigator().flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Flight route shared"))
                            else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… to GPX file")
                        onTriggered: {
                            headerMenuX.close()
                            global.mobileAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = global.mobileAdaptor().exportContent(global.navigator().flightRoute.toGpx(), "application/gpx+xml", global.navigator().flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Flight route shared"))
                            else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }
                }

                AutoSizingMenu {
                    title: qsTr("Open in other app …")
                    enabled: (global.navigator().flightRoute.size > 0)

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            global.mobileAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = global.mobileAdaptor().viewContent(global.navigator().flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Flight route opened in other app"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… in GPX format")

                        onTriggered: {
                            global.mobileAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = global.mobileAdaptor().viewContent(global.navigator().flightRoute.toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Flight route opened in other app"))
                        }
                    }

                }

            }
        }

    }


    ScrollView {
        id: acftTab
        anchors.fill: parent

        contentWidth: width
        clip: true

        GridLayout {
            id: aircraftTab
            anchors.left: parent.left
            anchors.leftMargin: Qt.application.font.pixelSize
            anchors.right: parent.right
            anchors.rightMargin: Qt.application.font.pixelSize

            columns: 4


            Rectangle {
                Layout.columnSpan: 4
                height: Qt.application.font.pixelSize
            }
            Label {
                text: qsTr("Name")
                Layout.columnSpan: 4
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
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
                Layout.minimumWidth: Qt.application.font.pixelSize*5
                KeyNavigation.tab: horizontalUOM

                onEditingFinished: {
                    global.navigator().aircraft.name = text
                    horizontalUOM.focus = true
                }
                text: global.navigator().aircraft.name
                placeholderText: qsTr("undefined")
            }

            Label {
                text: qsTr("Units")
                Layout.columnSpan: 4
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
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

                currentIndex: global.navigator().aircraft.horizontalDistanceUnit
                onCurrentIndexChanged: global.navigator().aircraft.horizontalDistanceUnit = currentIndex

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

                currentIndex: global.navigator().aircraft.verticalDistanceUnit
                onCurrentIndexChanged: global.navigator().aircraft.verticalDistanceUnit = currentIndex

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

                currentIndex: global.navigator().aircraft.fuelConsumptionUnit
                onCurrentIndexChanged: global.navigator().aircraft.fuelConsumptionUnit = currentIndex

                model: [ qsTr("Liters"), qsTr("U.S. Gallons") ]
            }


            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("True Airspeed")
                Layout.columnSpan: 4
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label {
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
                id: cruiseSpeed
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: Qt.application.font.pixelSize*5
                KeyNavigation.tab: descentSpeed

                validator: DoubleValidator {
                    bottom: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        global.navigator().aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        global.navigator().aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        global.navigator().aircraft.cruiseSpeed = aircraftPage.staticSpeed.fromMPS(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? Material.foreground : "red")
                text: {
                    if (!global.navigator().aircraft.cruiseSpeed.isFinite()) {
                        return ""
                    }
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(global.navigator().aircraft.cruiseSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(global.navigator().aircraft.cruiseSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(global.navigator().aircraft.cruiseSpeed.toMPH()).toString()
                    }

                }
                placeholderText: qsTr("undefined")
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
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
                    global.navigator().aircraft.cruiseSpeed = speed.fromKN(-1)
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
                Layout.minimumWidth: Qt.application.font.pixelSize*5
                KeyNavigation.tab: minimumSpeed

                validator: DoubleValidator {
                    bottom: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation

                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        global.navigator().aircraft.descentSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        global.navigator().aircraft.descentSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        global.navigator().aircraft.descentSpeed = aircraftPage.staticSpeed.fromMPS(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? Material.foreground : "red")
                text: {
                    if (!global.navigator().aircraft.descentSpeed.isFinite()) {
                        return ""
                    }
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(global.navigator().aircraft.descentSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(global.navigator().aircraft.descentSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(global.navigator().aircraft.descentSpeed.toMPH()).toString()
                    }
                }
                placeholderText: qsTr("undefined")
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
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
                    global.navigator().aircraft.descentSpeed = speed.fromKN(-1)
                    descentSpeed.clear()
                }
            }

            Label {
                text: qsTr("Minumum")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
                id: minimumSpeed
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: Qt.application.font.pixelSize*5
                KeyNavigation.tab: fuelConsumption

                validator: DoubleValidator {
                    bottom: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.minValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.minValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.minValidSpeed.toMPH()
                        }
                    }
                    top: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return global.navigator().aircraft.maxValidSpeed.toKN()
                        case Aircraft.Kilometer:
                            return global.navigator().aircraft.maxValidSpeed.toKMH()
                        case Aircraft.StatuteMile :
                            return global.navigator().aircraft.maxValidSpeed.toMPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        global.navigator().aircraft.minimumSpeed = aircraftPage.staticSpeed.fromKN(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.Kilometer:
                        global.navigator().aircraft.minimumSpeed = aircraftPage.staticSpeed.fromKMH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.StatuteMile :
                        global.navigator().aircraft.minimumSpeed = aircraftPage.staticSpeed.fromMPS(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? Material.foreground : "red")
                text: {
                    if (!global.navigator().aircraft.minimumSpeed.isFinite()) {
                        return ""
                    }
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
                    case Aircraft.NauticalMile:
                        return Math.round(global.navigator().aircraft.minimumSpeed.toKN()).toString()
                    case Aircraft.Kilometer:
                        return Math.round(global.navigator().aircraft.minimumSpeed.toKMH()).toString()
                    case Aircraft.StatuteMile :
                        return Math.round(global.navigator().aircraft.minimumSpeed.toMPH()).toString()
                    }
                }
                placeholderText: qsTr("undefined")
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(global.navigator().aircraft.horizontalDistanceUnit) {
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
                    global.navigator().aircraft.descentSpeed = speed.fromKN(-1)
                    descentSpeed.clear()
                }
            }


            Label { Layout.fillHeight: true }
            Label {
                text: qsTr("Fuel Consumption")
                Layout.columnSpan: 4
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label {
                text: qsTr("Cruise")
                Layout.alignment: Qt.AlignBaseline
            }
            TextField {
                id: fuelConsumption
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: Qt.application.font.pixelSize*5
                KeyNavigation.tab: name

                validator: DoubleValidator {
                    bottom: {
                        switch(global.navigator().aircraft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return global.navigator().aircraft.minValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return global.navigator().aircraft.minValidFuelConsumption.toGPH()
                        }
                    }
                    top: {
                        switch(global.navigator().aircraft.fuelConsumptionUnit) {
                        case Aircraft.LiterPerHour:
                            return global.navigator().aircraft.maxValidFuelConsumption.toLPH()
                        case Aircraft.GallonPerHour:
                            return global.navigator().aircraft.maxValidFuelConsumption.toGPH()
                        }
                    }
                    notation: DoubleValidator.StandardNotation
                }
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: {
                    switch(global.navigator().aircraft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        global.navigator().aircraft.fuelConsumption = aircraftPage.staticVolumeFlow.fromLPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    case Aircraft.GallonPerHour:
                        global.navigator().aircraft.fuelConsumption = aircraftPage.staticVolumeFlow.fromGPH(Number.fromLocaleString(Qt.locale(), text))
                        return
                    }
                }
                color: (acceptableInput ? Material.foreground : "red")
                text: {
                    if (!global.navigator().aircraft.fuelConsumption.isFinite()) {
                        return ""
                    }
                    switch(global.navigator().aircraft.fuelConsumptionUnit) {
                    case Aircraft.LiterPerHour:
                        return global.navigator().aircraft.fuelConsumption.toLPH().toLocaleString(Qt.locale(), 'f', 1)
                    case Aircraft.GallonPerHour:
                        return global.navigator().aircraft.fuelConsumption.toGPH().toLocaleString(Qt.locale(), 'f', 1)
                    }
                }
                placeholderText: qsTr("undefined")
            }
            Label {
                Layout.alignment: Qt.AlignBaseline
                text: {
                    switch(global.navigator().aircraft.fuelConsumptionUnit) {
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
                    aircraft.fuelConsumptionInLPH = -1
                    fuelConsumption.clear()
                }
            }

            Label { Layout.fillHeight: true }
        }

    }

} // Page
