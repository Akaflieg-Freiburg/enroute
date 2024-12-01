/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute

/* This is a dialog with detailed information about a weather station. To use this dialog, all you have to do is to set a WeatherStation in the property "weatherStation" and call open(). */


CenteringDialog {
    id: weatherReportDialog

    required property WeatherStation weatherStation
    onWeatherStationChanged: sv.ScrollBar.vertical.position = 0.0 // Reset scroll bar if station changes

    modal: true
    standardButtons: Dialog.Close
    title: weatherStation.ICAOCode + " • " + weatherStation.extendedName

    ColumnLayout {
        anchors.fill: parent

        Label { // Second header line with distance and QUJ
            text: Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), weatherReportDialog.weatherStation.coordinate)
            visible: PositionProvider.receivingPositionInfo
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        DecoratedScrollView { // DecoratedScrollView with METAR & TAF information
            id: sv

            Layout.fillHeight: true
            Layout.fillWidth: true

            contentHeight: co.height
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            ColumnLayout {
                id: co
                width: parent.width

                Label { // title: "METAR"
                    id: metarTitleLine

                    visible: weatherReportDialog.weatherStation.metar.isValid
                    text: "METAR " + Clock.describeTimeDifference(weatherReportDialog.weatherStation.metar.observationTime, Clock.time)
                    font.bold: true
                    font.pixelSize: 1.2*weatherReportDialog.font.pixelSize
                }

                Label { // raw METAR text
                    visible: metarTitleLine.visible
                    text: weatherReportDialog.weatherStation.metar.rawText
                    Layout.fillWidth: true
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*font.pixelSize
                    topPadding: 0.2*font.pixelSize
                    leftPadding: 0.2*font.pixelSize
                    rightPadding: 0.2*font.pixelSize

                    leftInset: -4
                    rightInset: -4

                    // Background color according to METAR/FAA flight category
                    background: Rectangle {
                        border.color: "black"
                        color: weatherReportDialog.weatherStation.metar.flightCategoryColor
                        opacity: 0.2
                        radius: 4
                    }
                }

                Label { // Decoded METAR text
                    visible: metarTitleLine.visible
                    text: weatherReportDialog.weatherStation.metar.decodedText(Navigator.aircraft, Clock.time)
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                }

                Label { // Derived Data
                    Layout.fillWidth: true
                    visible: metarTitleLine.visible && text !== ""
                    text: weatherReportDialog.weatherStation.metar.derivedData(Navigator.aircraft, Global.warnMETARPerformance, Global.showMETARPerformanceExplanation)
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    bottomPadding: 0.2*font.pixelSize
                    topPadding: 0.4*font.pixelSize

                    onLinkActivated: (linkText) =>
                                     {
                                         if (linkText === "hideExplanation")
                                         Global.showMETARPerformanceExplanation = false
                                         if (linkText === "hidePerformanceWarning")
                                         Global.warnMETARPerformance = false
                                     }
                }

                Label { // title: "TAF"
                    id: tafTitleLine

                    Layout.fillWidth: true
                    visible: weatherReportDialog.weatherStation.taf.isValid
                    text: "TAF"
                    font.bold: true
                    font.pixelSize: 1.2*weatherReportDialog.font.pixelSize
                    topPadding: 0.4*font.pixelSize
                }

                Label { // raw TAF text
                    visible: tafTitleLine.visible
                    text: weatherReportDialog.weatherStation.taf.rawText
                    Layout.fillWidth: true
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*font.pixelSize
                    topPadding: 0.4*font.pixelSize
                    leftPadding: 0.2*font.pixelSize
                    rightPadding: 0.2*font.pixelSize

                    leftInset: -4
                    rightInset: -4

                    background: Rectangle {
                        border.color: "black"
                        opacity: 0.2
                        radius: 4
                    }
                }

                Label { // decoded TAF text
                    visible: tafTitleLine
                    text: weatherReportDialog.weatherStation.taf.decodedText(Navigator.aircraft, Clock.time)
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText // OK
                }
            }
        }

        Keys.onBackPressed: (event) => {
            event.accepted = true;
            weatherReportDialog.close()
        }
    }

}
