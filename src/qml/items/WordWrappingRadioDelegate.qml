import QtQuick
import QtQuick.Controls

// This is a version of RadioDelegate that does word wrapping in the text

RadioDelegate {
    id: itemDelegate
    contentItem: Label { // Text

        rightPadding: itemDelegate.indicator.width + itemDelegate.spacing
        text: itemDelegate.text
        font: itemDelegate.font
        wrapMode: Text.Wrap
    }
}
