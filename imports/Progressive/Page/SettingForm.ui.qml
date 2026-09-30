import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import Progressive.Component 2.0
import Progressive.Effect 2.0
import Progressive.Style 0.1

import Progressive 0.1
import Progressive.Setting 0.1

Item {
    property alias listModel: accountSettingsListView.model

    property alias addAccountButton: addAccountButton

    implicitWidth: 400
    implicitHeight: 300

    Item {
        id: accountForm

        parent: null

        Item {
            anchors.fill: parent
            anchors.margins: 64

            ColumnLayout {
                anchors.fill: parent

                AutoListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: accountSettingsListView

                    boundsBehavior: Flickable.DragOverBounds

                    clip: true

                    delegate: SettingAccountDelegate {}
                }

                PButton {
                    Layout.fillWidth: true

                    id: addAccountButton

                    text: "Add Account"
                    flat: true
                }
            }
        }
    }

    Item {
        id: generalForm

        parent: null

        Item {
            anchors.fill: parent
            anchors.margins: 64

            Column {
                spacing: 8

                CheckBox {
                    text: "Use press and hold instead of right click"
                    checked: PSettings.pressAndHold

                    onCheckedChanged: PSettings.pressAndHold = checked
                }

                CheckBox {
                    text: "Show tray icon"
                    checked: PSettings.showTray

                    onCheckedChanged: PSettings.showTray = checked
                }

                CheckBox {
                    text: "Confirm on Exit"
                    checked: PSettings.confirmOnExit

                    onCheckedChanged: PSettings.confirmOnExit = !checked
                }
            }
        }
    }

    Item {
        id: appearanceForm

        parent: null

        Item {
            anchors.fill: parent
            anchors.margins: 64

            Column {
                spacing: 8

                CheckBox {
                    text: "Dark theme"
                    checked: PSettings.darkTheme

                    onCheckedChanged: PSettings.darkTheme = checked
                }
            }
        }
    }

    Item {
        id: aboutForm

        parent: null

        Item {
            anchors.fill: parent
            anchors.margins: 64

            ColumnLayout {
                spacing: 16
                Image {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64

                    source: "qrc:/assets/img/icon.png"
                }
                Label {
                    text: "Progressive Chat, an IM client for the Matrix protocol."
                }
                Label {
                    text: "Released under GNU General Public License, version 3."
                }
            }
        }
    }

    Rectangle {
        width: 240
        height: parent.height
        z: 10

        id: settingDrawer

        color: PSettings.darkTheme ? "#323232" : "#f3f3f3"

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 4
        }

        Column {
            anchors.fill: parent

            Repeater {
                model: ListModel {
                    ListElement {
                        category: "Accounts"
                        form: 0
                    }
                    ListElement {
                        category: "General"
                        form: 1
                    }
                    ListElement {
                        category: "Appearance"
                        form: 2
                    }
                    ListElement {
                        category: "About"
                        form: 3
                    }
                }

                delegate: SettingCategoryDelegate {
                    width: parent.width
                }
            }
        }
    }

    PScreenStack {
        anchors.fill: parent
        anchors.leftMargin: settingDrawer.width

        id: settingStackView

        initialItem: aboutForm
    }
}
