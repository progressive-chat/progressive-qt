import QtQuick 2.6
import QtQuick.Controls 1.4

Menu {
    property var row: null
    property var model: null
    property string selectedText

    readonly property bool isFile: model  && (model.eventType === "video" || model.eventType === "audio" || model.eventType === "file" || model.eventType === "image")

    id: messageContextMenu

    MenuItem {
        text: "View Source"

        onTriggered: {
            sourceDialog.sourceText = model.toolTip
            sourceDialog.open()
        }
    }
    MenuItem {
        visible: isFile
        text: "Open Externally"

        onTriggered: row.openExternally()
    }
    MenuItem {
        visible: isFile
        text: "Save As"

        onTriggered: row.saveFileAs()
    }
    MenuItem {
        text: "Reply"

        onTriggered: {
            roomPanelInput.isReply = true
            roomPanelInput.replyUserID = model.author.id
            roomPanelInput.replyEventID = model.eventId
            roomPanelInput.replyContent = selectedText != "" ? selectedText : model.message
        }
    }
    MenuItem {
        visible: model && model.author === currentRoom.localUser
        text: "Redact"

        onTriggered: currentRoom.redactEvent(model.eventId)
    }
}
