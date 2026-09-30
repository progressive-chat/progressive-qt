import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import Progressive.Panel 2.0
import Progressive.Component 2.0
import Progressive.Effect 2.0

import Progressive 0.1
import Progressive.Setting 0.1

Item {
    property alias connection: roomListModel.connection
    property alias enteredRoom: roomListForm.enteredRoom
    property alias filter: roomListForm.filter

    id: page

    RoomListModel {
        id: roomListModel

        onNewMessage: if (!window.active) progressiveController.postNotification(roomId, eventId, roomName, senderName, text, icon, iconPath)
    }

    SplitView {
        anchors.fill: parent

        RoomListPanel {
            width: page.width * 0.35
            Layout.minimumWidth: 64

            id: roomListForm

            listModel: roomListModel

            onWidthChanged: {
                if (width < 240) width = 64
            }

            ElevationEffect {
                anchors.fill: source
                z: source.z - 1

                source: parent
                elevation: 4
            }
        }

        RoomPanel {
            Layout.fillWidth: true
            Layout.minimumWidth: 480

            id: roomForm

            currentRoom: roomListForm.enteredRoom
        }
    }

    function goToEvent(eventID) {
        roomForm.goToEvent(eventID)
    }
}
