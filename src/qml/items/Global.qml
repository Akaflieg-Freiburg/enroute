/***************************************************************************
 *   Copyright (C) 2023-2026 by Stefan Kebekus                             *
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

pragma Singleton

import QtCore
import QtQuick
import QtQuick.Controls.Material

import akaflieg_freiburg.enroute

Item {
    id: global

    // Objects from main.qml that are needed all over the user interface.
    // They are set from main.qml when the objects have been created.
    property ApplicationWindow appWindow
    property Loader dialogLoader
    property DialogLoader textDialogLoader
    property Drawer drawer
    property StackView stackView
    property Toast toast

    // Open the manual page pageUrl (relative to the manual root). This uses
    // the external browser where possible and an in-app viewer otherwise.
    function openManual(pageUrl) {

        if ((Qt.platform.os === "ios") ||
                ((Qt.platform.os === "android") && (Qt.application.version < "6.7.0")))
        {
            stackView.push(Qt.resolvedUrl("../pages/Manual.qml"), {"fileName": pageUrl})
            return
        }
        Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual/"+pageUrl)
    }
    property vac currentVAC
    property vac defaultVAC

    //
    // GUI Warnings
    //
    property bool warnNOTAMLocation: true
    property bool warnMETARPerformance: true
    property bool showMETARPerformanceExplanation: true
    Settings {
        category: "GUIWarnings"
        property alias warnNOTAMLocation: global.warnNOTAMLocation
        property alias warnMETARPerformance: global.warnMETARPerformance
        property alias showMETARPerformanceExplanation: global.showMETARPerformanceExplanation
    }

    //
    // Parameters for the moving map display
    //

    property int mapBearingPolicy: 0
    readonly property int mapBearingPolicyRect: {
        // Rectified version, always returns sane values
        if (!mapBearingPolicy || (mapBearingPolicy < 0) || (mapBearingPolicy > 2)) {
            return MFM.NUp
        }
        return mapBearingPolicy
    }
    onMapBearingPolicyChanged: {
        if (mapBearingPolicy != MFM.UserDefinedBearingUp)
        {
            mapBearingRevertPolicy = mapBearingPolicy
        }

        if (toast) {
            if (global.mapBearingPolicy === MFM.NUp) {
                toast.doToast(qsTr("Map Mode: North Up"))
            } else if (Global.mapBearingPolicy === MFM.TTUp) {
                toast.doToast(qsTr("Map Mode: Track Up"))
            } else {
                toast.doToast(qsTr("Map Mode: User Defined Direction Up"))
            }
        }
    }
    property int mapBearingRevertPolicy: 0
    readonly property int mapBearingRevertPolicyRect: {
        // Rectified version, always returns sane values
        if (!mapBearingRevertPolicy || (mapBearingRevertPolicy < 0) || (mapBearingRevertPolicy > 2)) {
            return MFM.NUp
        }
        return mapBearingRevertPolicy
    }
    property bool followGPS: true
    property real mapZoomLevelMax: 17
    property real mapZoomLevelMin: 7.0001 // When setting 7 precisely, MapBox is looking for tiles of zoom 6, which we do not have…
    property real mapZoomLevel: 12
    readonly property real mapZoomLevelRect: {
        // Rectified version, always returns sane values
        if (!mapZoomLevel || !isFinite(mapZoomLevel) || (mapZoomLevel < mapZoomLevelMin) || (mapZoomLevel > mapZoomLevelMax)) {
            return 12
        }
        return mapZoomLevel
    }
    property real mapBearing: 0
    readonly property real mapBearingRect: {
        // Rectified version, always returns sane values
        if (!mapBearing || !isFinite(mapBearing) || (mapBearing < 0) || (mapBearing > 360)) {
            return 0
        }
        return mapBearing
    }
    property var mapCenter
    readonly property var mapCenterRect: {
        // Rectified version, always returns sane values
        if (!mapCenter || !mapCenter.isValid) {
            return PositionProvider.lastValidCoordinate
        }
        return mapCenter
    }
    Settings {
        category: "MovingMap"
        property alias mapBearingPolicy: global.mapBearingPolicy
        property alias mapBearingRevertPolicy: global.mapBearingRevertPolicy
        property alias mapFollowGPSPolicy: global.followGPS
        property alias mapZoomLevel: global.mapZoomLevel
        property alias mapBearing: global.mapBearing
        property alias mapCenter: global.mapCenter
    }

    //
    // Aviation map color palette
    //
    // Shared by the moving map (FlightMap) and by dialogs that render airspace
    // color legends (e.g. WaypointDescription). The saturated day colors glare on
    // the dark night base map and on dark-themed dialogs, so night mode uses
    // muted, desaturated hues. "airspaceNeutral" is the near-black/near-white
    // color used for TMZ outlines and thin separators that must read on either
    // background.
    //
    readonly property string airspaceBlue:    GlobalSettings.nightMode ? "#4f7bad" : "blue"
    readonly property string airspaceRed:     GlobalSettings.nightMode ? "#c05a52" : "red"
    readonly property string airspaceGreen:   GlobalSettings.nightMode ? "#579f66" : "green"
    readonly property string airspaceYellow:  GlobalSettings.nightMode ? "#b8a63c" : "yellow"
    readonly property string airspaceNeutral: GlobalSettings.nightMode ? "#e0e0e0" : "black"

    //
    // Drawing style per airspace category
    //
    // Shared by the airspace views of WaypointDescription: the color legend of
    // AirspaceList and the boxes of AirspaceStack draw the same category in the
    // same way. Kept next to the palette above, so that a new category has to
    // be added in one place only.
    //
    // The outline style is reported as a boolean rather than a ShapePath value,
    // so that this singleton does not have to import QtQuick.Shapes.
    //
    function airspaceOutlineColor(CAT: string) : color {
        switch(CAT) {
        case "A":
        case "B":
        case "C":
        case "D":
        case "E":
        case "F":
        case "G":
        case "CTR":
        case "ATZ":
        case "RMZ":
        case "TIZ":
        case "TIA":
            return global.airspaceBlue;
        case "GLD":
            return global.airspaceYellow;
        case "DNG":
        case "P":
        case "PJE":
        case "R":
        case "SUA":
            return global.airspaceRed;
        case "TMZ":
            return global.airspaceNeutral;
        case "FIR":
        case "FIS":
        case "NRA":
            return global.airspaceGreen;
        }
        return "transparent"
    }

    function airspaceOutlineIsSolid(CAT: string) : bool {
        switch(CAT) {
        case "A":
        case "B":
        case "C":
        case "D":
        case "E":
        case "F":
        case "G":
        case "GLD":
        case "NRA":
            return true;
        }
        return false
    }

    function airspaceDashPattern(CAT: string) : var {
        switch(CAT) {
        case "TMZ":
            return [4, 2, 1, 2];
        case "FIR":
        case "FIS":
            return [4, 0];
        }
        return [4, 4]
    }

    // The wide, translucent band that some categories draw inside their outline
    function airspaceBandColor(CAT: string) : color {
        switch(CAT) {
        case "A":
        case "B":
        case "C":
        case "D":
        case "ATZ":
        case "RMZ":
        case "TIZ":
        case "TIA":
            return Qt.alpha(global.airspaceBlue, 0.25);
        case "DNG":
        case "P":
        case "R":
            return Qt.alpha(global.airspaceRed, 0.25);
        case "NRA":
            return Qt.alpha(global.airspaceGreen, 0.25);
        }
        return "transparent"
    }

    function airspaceFillColor(CAT: string) : color {
        switch(CAT) {
        case "CTR":
            return Qt.alpha(global.airspaceRed, 0.25);
        case "GLD":
            return Qt.alpha(global.airspaceYellow, 0.25);
        case "ATZ":
        case "RMZ":
        case "TIZ":
        case "TIA":
            return Qt.alpha(global.airspaceBlue, 0.25);
        }
        return "transparent"
    }


    //
    // Traffic label colors
    //
    // Shared by the traffic labels on the moving map (TrafficLabel, and the
    // label for nondirectional traffic in FlightMap). By day, the labels are
    // bright chips tinted in the alarm color, with black text; at night, dark
    // chips that keep the alarm tint, with light text. Text and frame colors
    // are set explicitly because the Material theme text color (white in
    // night mode) is unreadable on the tinted backgrounds.
    //
    readonly property color trafficLabelTextColor:  GlobalSettings.nightMode ? "#e0e0e0" : "black"
    readonly property color trafficLabelFrameColor: GlobalSettings.nightMode ? "#e0e0e0" : "black"
    function trafficLabelBackgroundColor(alarmColor) {
        return GlobalSettings.nightMode ? Qt.darker(alarmColor, 2.5) : Qt.lighter(alarmColor, 1.9)
    }


    //
    // UI chrome colors
    //
    // Semantic colors for the application shell (pages, dialogs, the navigation
    // drawer) as opposed to the aviation-map palette above. Both track the
    // night-mode setting, which also drives Material.theme (see
    // +Material/AppWindow.qml), so hardcoding "white"/"black" at call sites is
    // avoided in favor of these roles.
    //
    // "pageBackgroundColor" is the opaque surface painted behind full-page
    // overlay messages (e.g. the "no charts installed" notice) and the
    // "▲ more ▲" fade labels. It is deliberately left to pair with the Material
    // theme's default text color (light in night mode), which is why the two
    // must be flipped together. "dividerColor" is the near-black/near-white hue
    // for thin separators, menu dividers, focus frames and outline borders that
    // must read on either background (cf. airspaceNeutral).
    //
    readonly property color pageBackgroundColor: GlobalSettings.nightMode ? "black" : "white"
    readonly property color dividerColor:        GlobalSettings.nightMode ? "#e0e0e0" : "black"


    //
    // Hyperlink color
    //
    // Labels with textFormat Text.RichText ignore the linkColor property; Qt
    // renders anchors in the hardcoded default blue, which is unreadable on the
    // night-mode background. Per Qt documentation, the only way to color
    // RichText links is markup inside the text itself. withLinkColor() wraps
    // the content of every anchor in a ready-made HTML string accordingly; use
    // it in the text binding of any RichText label that contains links. The
    // colors below equal the Material accent shades that StyledText labels get
    // via the Material style, so links look the same in both text formats.
    //
    readonly property color linkColor: GlobalSettings.nightMode ? "#f48fb1" : "#e91e63"
    function withLinkColor(html) {
        return html.replace(/<a\s([^>]*)>/g, "<a $1><font color='" + linkColor + "'>")
                   .replace(/<\/a>/g, "</font></a>")
    }


    //
    // Permissions
    //

    property LocationPermission locationPermission: LocationPermission {
        id: locationPermission

        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse

        onStatusChanged: PositionProvider.startUpdates()
        Component.onCompleted: PositionProvider.startUpdates()
    }


    //
    // Connections
    //
    Connections {
        target: DemoRunner

        function onRequestFollowGPS(newVal) {
            global.followGPS = newVal
        }

        function onRequestMapBearingPolicy(newBearing) {
            global.mapBearingPolicy = newBearing
        }
    }

}
