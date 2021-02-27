import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

Dialog {
    id: simStatusDialog

    title: qsTr("Simulation Interface Status")

    // Size is chosen so that the dialog does not cover the parent in full
    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    standardButtons: Dialog.Ok

    ScrollView {
        id: view
        clip: true
        anchors.fill: parent

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        GridLayout {
            id: gl
            columnSpacing: 30
            columns: 2

            Label { text: qsTr("Simulaion Interface Status") }
            Label {
                font.weight: Font.Bold
                text: satNav.statusAsString
                color: (satNav.status === SatNav.OK) ? "green" : "red"
                wrapMode: Text.Wrap
            }

            Label { text: qsTr("Simulator") }
            Label { text: satNav.sourceName }

            Label { text: qsTr("Last Fix") }
            Label { text: satNav.timestampAsString }

            Label { text: qsTr("Mode") }
            Label { text: satNav.isInFlight ? qsTr("Flight") : qsTr("Ground") }

            Label {
                font.pixelSize: Qt.application.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                text: qsTr("Horizontal")
                font.weight: Font.Bold
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Latitude") }
            Label { text: satNav.latitudeAsString }

            Label { text: qsTr("Longitude") }
            Label { text: satNav.longitudeAsString }

            Label { text: qsTr("Error") }
            Label { text: satNav.horizontalPrecisionInMetersAsString }

            Label { text: qsTr("GS") }
            Label { text: globalSettings.useMetricUnits ? satNav.groundSpeedInKMHAsString : satNav.groundSpeedInKnotsAsString }

            Label { text: qsTr("TT") }
            Label { text: satNav.trackAsString }

            Label {
                font.pixelSize: Qt.application.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                text: qsTr("Vertical")
                font.weight: Font.Bold
                Layout.columnSpan: 2
            }

            Label { text: qsTr("ALT") }
            Label { text: satNav.altitudeInFeetAsString }
        } // GridLayout

    } // Scrollview

    onAccepted: {
        // Give feedback
        mobileAdaptor.vibrateBrief()
        close()
    }
} // Dialog
