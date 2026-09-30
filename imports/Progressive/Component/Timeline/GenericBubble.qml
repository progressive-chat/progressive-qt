// SPDX-License-Identifier: GPL-3.0-only
// Message bubble (Qt 5.6 safe, pure QtQuick).
// Replaces the Controls 2 Control based version (padding / contentItem /
// background). contentItem is reparented into the padded holder and
// stretched to the bubble width.
import QtQuick 2.6

import Progressive.Component 2.0
import Progressive.Effect 2.0

import Progressive.Setting 0.1
import Progressive.Style 0.1

Item {
    property bool highlighted: false
    property bool colored: false

    readonly property bool darkBackground: highlighted  ? true : PSettings.darkTheme

    property int padding: 12
    property var contentItem

    id: root

    onContentItemChanged: {
        if (contentItem) {
            contentItem.parent = holder
            contentItem.width = Qt.binding(function() { return holder.width })
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.colored ? PPalette.accent : root.highlighted ? PPalette.primary : PPalette.card

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 1
        }
    }

    Item {
        id: holder

        anchors.fill: parent
        anchors.margins: root.padding
    }

    AutoMouseArea {
        anchors.fill: parent

        onSecondaryClicked: {
            messageContextMenu.row = messageRow
            messageContextMenu.model = model
            messageContextMenu.selectedText = contentLabel.selectedText
            messageContextMenu.popup()
        }
    }
}
