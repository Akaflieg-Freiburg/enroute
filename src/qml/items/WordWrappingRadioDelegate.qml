import QtQuick
import QtQuick.Controls.Material

import akaflieg_freiburg.enroute

// This is a version of RadioDelegate that does word wrapping in the text

RadioDelegate {
    id: itemDelegate

    onClicked: PlatformAdaptor.vibrateBrief()
    contentItem: Label { // Text

        rightPadding: itemDelegate.indicator.width + itemDelegate.spacing
        text: itemDelegate.text
        font: itemDelegate.font
        wrapMode: Text.Wrap
    }
}
