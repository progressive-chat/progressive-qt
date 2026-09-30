import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.0

import Progressive.Setting 0.1

Label {
    Layout.alignment: Qt.AlignHCenter

    text: "<b>" + author.displayName + "</b> " + display
    color: "white"

    padding: 8

    wrapMode: Label.Wrap
    linkColor: "white"
    textFormat: PSettings.richText ? Text.RichText : Text.StyledText
    onLinkActivated: Qt.openUrlExternally(link)

    background: Rectangle {
        color: PSettings.darkTheme ? "#484848" : "grey"
    }
}
