import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.2

import Progressive.Component 2.0
import Progressive.Style 0.1

import Progressive 0.1

Rectangle {
    property alias image: headerImage.image
    property alias topic: headerTopicLabel.text
    signal clicked()

    id: header

    color: PPalette.accent

    PItemDelegate {
        anchors.fill: parent

        id: roomHeader

        onClicked: header.clicked()

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12

            spacing: 12

            ImageItem {
                Layout.preferredWidth: height
                Layout.fillHeight: true

                id: headerImage

                hint: currentRoom ? currentRoom.displayName : "No name"
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                visible: parent.width > 64

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    text: currentRoom ? currentRoom.displayName : ""
                    color: "white"
                    font.pointSize: 12
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: headerTopicLabel

                    color: "white"
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }
            }
        }
    }

    ProgressBar {
        width: parent.width
        z: 10
        anchors.bottom: parent.bottom

        visible: currentRoom && currentRoom.busy
        indeterminate: true

        style: ProgressBarStyle {
            progress: Rectangle { color: "white" }
        }
    }
}
