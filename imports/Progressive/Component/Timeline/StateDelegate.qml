// SPDX-License-Identifier: GPL-3.0-only
// Centered state/event label (Qt 5.6 safe, pure QtQuick).
// Replaces the Controls 2 Label based version (padding / background).
import QtQuick 2.6
import QtQuick.Layouts 1.2

import Progressive.Setting 0.1

Item {
    property alias text: label.text

    Layout.alignment: Qt.AlignHCenter

    implicitWidth: label.implicitWidth + 16
    implicitHeight: label.implicitHeight + 8

    id: root

    Rectangle {
        anchors.fill: parent
        color: PSettings.darkTheme ? "#484848" : "grey"
    }

    Text {
        id: label

        anchors.fill: parent
        anchors.margins: 8

        color: "white"
        wrapMode: Text.Wrap
        textFormat: PSettings.richText ? Text.RichText : Text.StyledText

        onLinkActivated: Qt.openUrlExternally(link)
    }
}
