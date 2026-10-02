// SPDX-License-Identifier: GPL-3.0-only
// User directory browser for starting direct chats, ported from
// Spectral (Dec 2019, StartChatDialog). Controls-1 adapted.
import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import Progressive 0.1
import Progressive.Component 2.0

ColumnLayout {
    property var connection

    signal chatRequested(string userId)

    id: root

    spacing: 8

    // Manual user-ID entry (kept from the old direct chat dialog).
    AutoTextField {
        Layout.fillWidth: true

        id: userField
        placeholderText: "@bot:matrix.org"

        Keys.onReturnPressed: {
            root.chatRequested(text)
        }
    }

    AutoTextField {
        Layout.fillWidth: true

        id: keywordField
        placeholderText: "Search users"

        Keys.onReturnPressed: {
            userDirectoryListModel.keyword = text
            userDirectoryListModel.search()
        }
    }

    AutoListView {
        Layout.fillWidth: true
        Layout.preferredHeight: 320

        id: userDirectoryListView

        clip: true
        spacing: 4

        model: UserDirectoryListModel {
            id: userDirectoryListModel

            connection: root.connection
        }

        delegate: PItemDelegate {
            width: userDirectoryListView.width
            height: 48

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4

                spacing: 8

                ImageItem {
                    Layout.preferredWidth: height
                    Layout.fillHeight: true

                    image: avatar
                    hint: name
                }

                Label {
                    Layout.fillWidth: true

                    text: name + " (" + userID + ")"
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }
            }

            onClicked: root.chatRequested(userID)
        }
    }
}
