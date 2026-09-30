import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.0

import Progressive 0.1

Popup {
    property var textArea
    property string emojiCategory: "people"

    EmojiModel {
        id: emojiModel
        category: emojiCategory
    }

    ColumnLayout {
        anchors.fill: parent

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            cellWidth: 36
            cellHeight: 36

            boundsBehavior: Flickable.DragOverBounds

            clip: true

            model: emojiModel.model

            delegate: Text {
                width: 36
                height: 36

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                font.pointSize: 20
                font.family: "Emoji"
                text: modelData

                MouseArea {
                    anchors.fill: parent
                    onClicked: textArea.insert(textArea.cursorPosition, modelData)
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2

            color: Material.accent
        }

        Row {
            EmojiButton { text: "😏"; category: "people" }
            EmojiButton { text: "🌲"; category: "nature" }
            EmojiButton { text: "🍛"; category: "food"}
            EmojiButton { text: "🚁"; category: "activity" }
            EmojiButton { text: "🚅"; category: "travel" }
            EmojiButton { text: "💡"; category: "objects" }
            EmojiButton { text: "🔣"; category: "symbols" }
            EmojiButton { text: "🏁"; category: "flags" }
        }
    }
}
