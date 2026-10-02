import QtQuick 2.6

import Progressive.Setting 0.1
import Progressive.Font 0.1

Text {
    property alias icon: materialLabel.text

    id: materialLabel

    color: PSettings.darkTheme ? "white" : "black"
    font.pointSize: 16
    font.family: MaterialFont.name
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
