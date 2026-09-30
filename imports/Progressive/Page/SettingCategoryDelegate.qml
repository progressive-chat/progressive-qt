import QtQuick 2.6
import QtQuick.Controls 2.0

ItemDelegate {
    text: category

    onClicked: {
        settingStackView.clear()
        settingStackView.push([accountForm, generalForm, appearanceForm, aboutForm][form])
    }
}
