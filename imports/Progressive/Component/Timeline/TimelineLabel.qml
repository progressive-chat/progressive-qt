// SPDX-License-Identifier: GPL-3.0-only
// Rich text label for the timeline (Qt 5.6 safe).
// Replaces the Controls 2 Label based version (no padding/background there).
import QtQuick 2.6

import Progressive.Setting 0.1
import Progressive.Style 0.1

Item {
    property alias text: label.text
    property alias font: label.font
    property color foreground: PPalette.foreground
    property bool coloredBackground: false
    property var background

    implicitWidth: label.implicitWidth
    implicitHeight: label.implicitHeight

    onBackgroundChanged: {
        if (background) {
            background.parent = root
            background.anchors.fill = root
        }
    }

    id: root

    Text {
        id: label

        anchors.fill: parent

        color: root.coloredBackground ? "white" : root.foreground
        wrapMode: Text.Wrap
        linkColor: root.coloredBackground ? "white" : PPalette.accent
        textFormat: Text.RichText

        onLinkActivated: Qt.openUrlExternally(link)
    }
}
