import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.0

import "qrc:/js/util.js" as Util

ItemDelegate {
    property var page
    property bool selected: stackView.currentItem === page
    property color highlightColor: Material.accent

    Rectangle {
        width: selected ? 4 : 0
        height: parent.height

        color: highlightColor

        Behavior on width {
            PropertyAnimation { easing.type: Easing.InOutCubic; duration: 200 }
        }
    }

    onClicked: Util.pushToStack(stackView, page)
}
