/***************************************************************************
 *   Copyright (C) 2020-2024 by Stefan Kebekus                             *
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
import "../items"

Page {
    id: pg
    title: qsTr("Bug Report")

    header: StandardHeader {}

    StackView {
        id: stack

        anchors.fill: parent
        anchors.bottomMargin: footer.visible ? 0 : SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        initialItem: mainPage
    }

    footer: Footer {
        visible: stack.depth > 1

        ToolButton {
            text: qsTr("Go back in bug report")
            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked:  {
                PlatformAdaptor.vibrateBrief()
                stack.pop()
            }
        }
    }


    Component {
        id: mainPage

        DecoratedScrollView {
            id: sv

            width: stack.width
            height: stack.height
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            bottomPadding: font.pixelSize + SafeInsets.bottom
            leftPadding: font.pixelSize + SafeInsets.left
            rightPadding: font.pixelSize + SafeInsets.right
            topPadding: font.pixelSize

            ColumnLayout {
                width: sv.availableWidth

                anchors.bottomMargin: font.pixelSize*1
                anchors.topMargin: font.pixelSize*1
                anchors.leftMargin: font.pixelSize*0.5
                anchors.rightMargin: font.pixelSize*0.5

                id: cL

                Label {
                    Layout.fillWidth: true

                    text: "<h3>"
                          + qsTr("Report a bug or make a suggestion for improvement")
                          + "</h3>"
                          + "<p>"
                          + qsTr("We aim to provide high-quality software.") + " "
                          + qsTr("Fixing errors is therefore always our first priority.") + " "
                          + qsTr("We are grateful for every report that we get, and we would also like to hear your suggestions for improvement.")
                          + "</p>"
                          + "<p>"
                          + qsTr("Please choose one of the buttons below, depending on whether you would like to report an issue with the app, or with the aviation data presented by the app.")
                          + "</p>"
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Main application")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        PlatformAdaptor.vibrateBrief()
                        stack.push(mainAppPage)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Aviation Data")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked:  {
                        PlatformAdaptor.vibrateBrief()
                        stack.push(openAIPNNonAirspace)
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("
<h4>Thank you for your help!</h4>
")
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                }
            }
        } // DecoratedScrollView
    }

    Component {
        id: mainAppPage

        DecoratedScrollView {
            id: sv

            width: stack.width
            height: stack.height
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            bottomPadding: font.pixelSize + SafeInsets.bottom
            leftPadding: font.pixelSize + SafeInsets.left
            rightPadding: font.pixelSize + SafeInsets.right
            topPadding: font.pixelSize

            ColumnLayout {
                id: cL

                width: sv.availableWidth

                anchors.bottomMargin: font.pixelSize*1
                anchors.topMargin: font.pixelSize*1
                anchors.leftMargin: font.pixelSize*0.5
                anchors.rightMargin: font.pixelSize*0.5

                Label {
                    Layout.fillWidth: true

                    text: qsTr("
<h3>Report a bug or make a suggestion for improvement</h3>

<h4>Issue in the main application</h4>

<p>Like many other software projects, the developers of
<strong>Enroute Flight Navigation</strong> use the web
service <a href='https://github.com'>GitHub</a> to
coordinate their work. We request that you use GitHub to
submit your report. This ensures that your report will
reach the right people, that your report will not be
forgotten and that you will be notified about any
progress.</p>

<p>Please use the button below to go to our GitHub Issue
Page and check if the problem has already been reported. If
not, please open a new issue. If you prefer to work on
your desktop computer, you can also send yourself a link to
GitHub by e-mail.</p>
")
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Open GitHub Issue Page")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        Qt.openUrlExternally("https://github.com/Akaflieg-Freiburg/enroute/issues")
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Send link by e-mail")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        Qt.openUrlExternally(qsTr("mailto:?subject=Enroute Flight Navigation, Issue Report &body=Link to GitHub: https://github.com/Akaflieg-Freiburg/enroute/issues"))
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("
<p>If you have difficulties with GitHub, you can contact
<a href='mailto:ms@squawk-vfr.de?subject=Enroute Flight
Navigation, Issue Report'>Markus Sachs</a> by e-mail.
Markus has kindly volunteered to help our users in his
spare time. He speaks English and German.</p>

<p>Please note that <strong>Enroute Flight
Navigation</strong> is written by a very small group of
flight enthusiasts in their spare time, as a service to
the community. Development of quality software takes time,
and we ask for your understanding that that we are not
able to implement all feature requests. Bugfixing always
comes first!</p>

<h3>Thank you for your help!</h3>
")
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }

        }

    }

    Component {
        id: openAIPNNonAirspace

        DecoratedScrollView {
            id: sv

            width: stack.width
            height: stack.height
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            bottomPadding: font.pixelSize + SafeInsets.bottom
            leftPadding: font.pixelSize + SafeInsets.left
            rightPadding: font.pixelSize + SafeInsets.right
            topPadding: font.pixelSize

            ColumnLayout {
                id: cL

                width: sv.availableWidth

                anchors.bottomMargin: font.pixelSize*1
                anchors.topMargin: font.pixelSize*1
                anchors.leftMargin: font.pixelSize*0.5
                anchors.rightMargin: font.pixelSize*0.5

                Label {
                    Layout.fillWidth: true

                    text: "<h3>"
                          + qsTr("Report a bug or make a suggestion for improvement")
                          + "</h3>"
                          + "<h4>"
                          + qsTr("Aviation Data")
                          + "</h4>"
                          + "<p>"
                          + qsTr("<strong>Enroute Flight Navigation</strong> displays aviation data provided by the projects <a href='http://openaip.net'>openAIP</a> and <a href='http://openflightmaps.org'>open flightmaps</a>.") + " "
                          + qsTr("The authors of <strong>Enroute Flight Navigation</strong> do not have write access to any of these databases.") + " "
                          + "</p>"
                          + "<p>"
                          + qsTr("Please contact <a href='mailto:peter.kemme@openflightmaps.org?subject=Enroute Flight Navigation, Issue Report'>Peter Kemme</a>.") + " "
                          + qsTr("Peter is active in both projects and  has kindly volunteered to help our users in his spare time.") + " "
                          + qsTr("Peter speaks English and German.")
                          + "</p>"
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                }
                Button {
                    Layout.fillWidth: true
                    text: qsTr("Send E-Mail")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        Qt.openUrlExternally("mailto:peter.kemme@openflightmaps.org?subject=Enroute Flight Navigation, Issue Report")
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: "<h3>" + qsTr("Thank you for your help!") + "</h3>"
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
    }

} // Page
