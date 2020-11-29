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

    footer: Pane {
        width: parent.width
        Material.elevation: 3
        visible: stack.depth > 1

        ToolButton {
            text: qsTr("Go Back")
            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked:  {
                mobileAdaptor.vibrateBrief()
                stack.pop()
            }
        }
    }


    Component {
        id: mainPage

        ScrollView {
            id: sv

            clip: true

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
                    Layout.fillWidth: true

                    text: qsTr("<h3>Report a bug or make a suggestion for improvement</h3>

<p>We aim to provide high-quality software. Fixing errors is therefore always our first priority. We are grateful for every report that we get, and we would also like to hear your suggestions for improvement.</p>")
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                } // Label

                Label {
                    Layout.fillWidth: true
                    text: qsTr("<p>Use this button to report an issue in the main application.</p>")
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Main application")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        mobileAdaptor.vibrateBrief()
                        stack.push(mainAppPage)
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("<p>Use these buttons to report error in the aviation data, such as wrong frequencies, outdated traffic patterns, missing navaids, etc.</p>")
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Airfields")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        mobileAdaptor.vibrateBrief()
                        stack.push(openAIPNNonAirspace)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Airspace")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        mobileAdaptor.vibrateBrief()
                        stack.push(openAIPAirspace)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("NavAids")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        mobileAdaptor.vibrateBrief()
                        stack.push(openAIPNNonAirspace)
                    }
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
                    Layout.fillWidth: true
                    text: qsTr("<h4>Thank you for your help!</h4>")
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
            }

        } // ScrollView

    }

    Component {
        id: mainAppPage

        ColumnLayout {
            ScrollView {
                id: sv

                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

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
                        Layout.fillWidth: true

                        text: qsTr("<h3>Report a bug or make a suggestion for improvement</h3>

<h4>Issue in the main application</h4>

<p>Like many other software projects, the developers of <strong>Enroute Flight Navigation</strong> use the web service GitHub to coordinate their work. We request that you use GitHub to submit your report. This ensures that your report will reach the right people, that your report will not be forgotten and that you will be informed about any progress.</p>

<p>Please use the button below to go to our GitHub Issue Page and check if the problem has already been reported. If not, please open a new issue. If you prefer to work on your desktop computer, you also send yourself a link to GitHub by e-mail.</p>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    } // Label
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Open GitHub Issue Page")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally("https://github.com/Akaflieg-Freiburg/enroute/issues")
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Send link by e-mail")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally(qsTr("mailto:?subject=Enroute Flight Navigation, Issue Report &body=Link to GitHub: https://github.com/Akaflieg-Freiburg/enroute/issues"))
                        }

                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("<p>If you have difficulties with GitHub, you could also contact Markus Sachs by e-mail, who has kindly volunteered to help our users. Markus speaks English and German.</p>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Send e-mail to Markus Sachs")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally(qsTr("mailto:ms@squawk-vfr.de?subject=Enroute Flight Navigation, Issue Report"))
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("<p>Please note that <strong>Enroute Flight Navigation</strong> is written by a very small group of flight enthusiasts in their spare time, as a service to the community. Development of quality software takes time, and we ask for your understanding that that we are not able to implement all feature requests.  Bugfixing always comes first!</p>

<h3>Thank you for your help!</h3>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }
                }

            } // ScrollView

        }

    }

    Component {
        id: openAIPNNonAirspace

        ColumnLayout {
            ScrollView {
                id: sv

                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

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
                        Layout.fillWidth: true

                        text: qsTr("<h3>Report a bug or make a suggestion for improvement</h3>

<h4>Aviation Data</h4>

<p>The aviation data concerned in your report comes from <a href='http://openAIP.net'>openAIP.net</a>, a free aviation database operated by Garrecht Avionik GmbH in Germany.</p>

<p>To correct the data in openAIP, we ask for your help. Please go to the openAIP web site, create an account and log in. You can then suggest corrections. Once your change is approved, the correction will appear in <strong>Enroute Flight Navigation</strong> within a week.</p>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    } // Label
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Open openAIP web site")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally("https://openaip.net")
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Send link by e-mail")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally(qsTr("mailto:?subject=Enroute Flight Navigation, Issue Report &body=Link to openAIP: https//openaip.net"))
                        }

                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("<p>If you have difficulties with the openAIP web site, you could also contact Peter Kemme by e-mail, who has kindly volunteered to help our users. Peter speaks English and German.</p>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Send e-mail to Peter Kemme")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally(qsTr("mailto:peter.kemme@openflightmaps.org?subject=Enroute Flight Navigation, Issue Report"))
                        }

                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("<h3>Thank you for your help!</h3>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }

                }

            } // ScrollView

        }

    }

    Component {
        id: openAIPAirspace

        ColumnLayout {
            ScrollView {
                id: sv

                Layout.fillWidth: true
                Layout.fillHeight: true

                clip: true

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
                        Layout.fillWidth: true

                        text: qsTr("<h3>Report a bug or make a suggestion for improvement</h3>

<h4>Airspace Data</h4>

<p>Airspace data shown in **Enroute Flight Navigation** comes from <a href='http://openAIP.net'>openAIP.net</a>, a free aviation database operated by Garrecht Avionik GmbH in Germany.</p>

<p>Please contact Peter Kemme by e-mail, who has kindly volunteered to help our users. Peter speaks English and German.</p>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("Send e-mail to Peter Kemme")
                        icon.source: "/icons/material/ic_bug_report.svg"
                        onClicked: {
                            mobileAdaptor.vibrateBrief()
                            Qt.openUrlExternally(qsTr("mailto:peter.kemme@openflightmaps.org?subject=Enroute Flight Navigation, Issue Report"))
                        }

                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("<h3>Thank you for your help!</h3>")
                        textFormat: Text.RichText
                        wrapMode: Text.Wrap
                    }

                }

            } // ScrollView

        }

    }

} // Page
