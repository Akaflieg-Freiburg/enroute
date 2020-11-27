/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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
import QtQuick.Controls.Material 2.15

import "../items"

Page {
    id: pg
    title: qsTr("Bug Report")

    header: StandardHeader {}

    StackView {
        id: stack

        anchors.fill: parent
        initialItem: mainPage
    }

    Component {
        id: mainPage

        ScrollView {
            id: sv

            clip: true
            anchors.fill: parent

            topPadding: Qt.application.font.pixelSize*1
            leftPadding: Qt.application.font.pixelSize*0.5
            rightPadding: Qt.application.font.pixelSize*0.5

            contentHeight: cL.height
            contentWidth: pg.width-leftPadding-rightPadding

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            ColumnLayout {
                id: cL

                width: pg.width-sv.leftPadding-sv.rightPadding

                Label {
                    id: lbl1
                    //                text: librarian.getStringFromRessource(":text/bugReport.html")
                    text: qsTr("
    <h3>Report a bug or make a suggestion for improvement</h3>

    <p>We aim to provide high-quality software. Fixing errors is therefore always our first priority. We are grateful for every report that we get, and we would also like to hear your suggestions for improvement.</p>

    <p>To ensure that your report reaches the right people, please tell us what part of the application (or aviation data) is concerned. Have a look at the list below and choose the appropriate item.</p>")
                    textFormat: Text.RichText

                    Layout.fillWidth: true

                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label

                Label {
                    text: "<br><h3>"+qsTr("Main Application")+"</h3>"
                }

               Button {
                    Layout.fillWidth: true
                    text: qsTr("Main application")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked: stack.push(mainAppPage)
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Translation")
                    icon.source: "/icons/material/ic_bug_report.svg"

                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Map design")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }

                Label {
                    text: "<br><h3>"+qsTr("Aviation Data")+"</h3>"
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Airfields")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Airspace")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("NavAids")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Procedures/Traffic circuits")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Reporting points")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }

                Label {
                    text: "<br><h4>"+qsTr("Thank you for your help!")+"</h4>"
                }

            }

        } // ScrollView

    }

    Component {
        id: mainAppPage

        ScrollView {
            id: sv

            clip: true
            anchors.fill: parent

            topPadding: Qt.application.font.pixelSize*1
            leftPadding: Qt.application.font.pixelSize*0.5
            rightPadding: Qt.application.font.pixelSize*0.5

            contentHeight: cL.height
            contentWidth: pg.width-leftPadding-rightPadding

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            ColumnLayout {
                id: cL

                width: pg.width-sv.leftPadding-sv.rightPadding

                Label {
                    id: lbl1

                    text: qsTr("
<h3>Bug report: Main application</h3>

<p>Like many other software projects, the developers of <strong>Enroute Flight Navigation</strong> use the web site 'GitHub' to coordinate their work.
In order to report an issue or suggest a feature, please go to our GitHub Issue Page and check if the problem has already been reported or if the suggestion
has already been made. If not, please open a new issue and explain in detail what the problem or what the suggestion is.</p>

<p>GitHub ensures that your report will reach the right people, that it will not be forgotten and that you will be informed about any progress.</p>")
                    textFormat: Text.RichText

                    Layout.fillWidth: true

                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Open GitHub Issue Page")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Send a link by e-mail")
                    icon.source: "/icons/material/ic_bug_report.svg"
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Return to starting page")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked: stack.pop()
                }
                Label {
                    text: "<br><h4>"+qsTr("Thank you for your help!")+"</h4>"
                }

            }

        } // ScrollView

    }

} // Page
