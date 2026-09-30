import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0

import Progressive.Component 2.0
import Progressive.Effect 2.0

import Progressive.Setting 0.1

Control {
    property bool highlighted: false
    property bool colored: false

    readonly property bool darkBackground: highlighted  ? true : PSettings.darkTheme

    padding: 12

    AutoMouseArea {
        anchors.fill: parent

        onSecondaryClicked: {
            messageContextMenu.row = messageRow
            messageContextMenu.model = model
            messageContextMenu.selectedText = contentLabel.selectedText
            messageContextMenu.popup()
        }
    }

    background: Rectangle {
        color: colored ? Material.accent : highlighted ? Material.primary : Material.background

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 1
        }
    }
}
