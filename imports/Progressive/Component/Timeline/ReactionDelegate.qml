// SPDX-License-Identifier: GPL-3.0-only
// Reaction pills under a message (Qt 5.6 safe, pure QtQuick).
// Ported from Spectral (Jul 2019); Controls 2 Control/ToolTip replaced.
import QtQuick 2.6
import QtQuick.Layouts 1.2

import Progressive.Setting 0.1
import Progressive.Style 0.1

Item {
    // Invisible when the message has no reactions.
    visible: reaction && reaction.length > 0

    implicitWidth: flow.implicitWidth
    implicitHeight: flow.implicitHeight

    id: root

    Flow {
        id: flow

        anchors.fill: parent

        spacing: 8

        Repeater {
            model: reaction

            Item {
                width: Math.min(pillLabel.implicitWidth + 12, 128)
                height: 28

                Rectangle {
                    anchors.fill: parent
                    radius: height / 2
                    color: modelData.hasLocalUser ? PPalette.accent : PPalette.secondaryText
                    opacity: modelData.hasLocalUser ? 1.0 : 0.45
                }

                Text {
                    id: pillLabel

                    anchors.fill: parent
                    anchors.margins: 6

                    text: modelData.reaction + (modelData.count > 1 ? " " + modelData.count : "")
                    color: "white"
                    font.pointSize: 11
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: currentRoom.toggleReaction(eventId, modelData.reaction)
                }
            }
        }
    }
}
