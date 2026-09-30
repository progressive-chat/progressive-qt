import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import Progressive.Setting 0.1
import Progressive.Font 0.1

Text {
    property alias icon: materialLabel.text

    id: materialLabel

    color: PSettings.darkTheme ? "white" : "dark"
    font.pointSize: 16
    font.family: MaterialFont.name
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
