import QtQuick 2.6
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2

import Progressive 0.1
import Progressive.Setting 0.1
import Progressive.Style 0.1

import Progressive.Component 2.0

import "qrc:/js/util.js" as Util

RowLayout {
    readonly property bool avatarVisible: !sentByMe && (aboveAuthor !== author || aboveSection !== section || aboveEventType === "state" || aboveEventType === "emote" || aboveEventType === "other")
    readonly property bool highlighted: !(sentByMe || eventType === "notice" )
    readonly property bool sentByMe: author === currentRoom.localUser
    readonly property bool isText: eventType === "notice" || eventType === "message"
    // FORK-ONLY: failed-message resend/discard ported from Aug 2019.
    // 0x05 == EventStatus.SendingFailed (hex literal like RoomPanelForm,
    // so the headless harness needs no C++ type registration).
    readonly property bool failed: marks === 0x05

    signal saveFileAs()
    signal openExternally()

    z: -5

    id: messageRow

    Layout.alignment: sentByMe ? Qt.AlignRight : Qt.AlignLeft

    spacing: 6

    ImageItem {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignTop

        round: false
        visible: avatarVisible
        hint: author.displayName
        image: author.avatar
    }

    Rectangle {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignTop

        color: "transparent"
        visible: !(sentByMe || avatarVisible)
    }

    GenericBubble {
        Layout.maximumWidth: messageListView.width - (!sentByMe ? 40 + messageRow.spacing : 0)

        id: genericBubble

        highlighted: messageRow.highlighted
        colored: highlighted && (eventType === "notice" || highlight)

        contentItem: ColumnLayout {
            id: messageColumn

            spacing: 0

            TimelineLabel {
                Layout.fillWidth: true

                id: authorLabel

                visible: messageRow.avatarVisible
                text: author.displayName
                foreground: PPalette.accent
                coloredBackground: highlighted
                font.bold: true

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: roomPanelInput.insert(author.displayName)
                }
            }

            TextEdit {
                Layout.fillWidth: true

                id: contentLabel

                text: (highlighted  ? "<style>a{color: white;} .user-pill{color: white}</style>" : "<style>a{color: " + PPalette.accent + ";} .user-pill{color: " + PPalette.accent + "}</style>") + display

                visible: isText
                color: highlighted ? "white" : PPalette.foreground

                font.family: authorLabel.font.family
                font.pointSize: 10
                selectByMouse: true
                readOnly: true
                wrapMode: Text.Wrap
                selectedTextColor: highlighted ? PPalette.accent : "white"
                selectionColor: highlighted ? "white" : PPalette.accent
                textFormat: Text.RichText

                onLinkActivated: Qt.openUrlExternally(link)

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            Loader {
                sourceComponent: {
                    switch (eventType) {
                    case "image":
                        return imageComponent
                    case "file":
                        return fileComponent
                    case "audio":
                        return audioComponent
                    }
                }

                active: eventType === "image" || eventType === "file" || eventType === "audio"
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 4

                TimelineLabel {
                    visible: userMarker.length > 5
                    text: userMarker.length - 5 + "+"
                    coloredBackground: highlighted
                    foreground: "grey"
                    font.pointSize: 8
                }

                Repeater {
                    model: userMarker.length > 5 ? userMarker.slice(0, 5) : userMarker

                    ImageItem {
                        width: parent.height
                        height: parent.height

                        hint: modelData.displayName
                        image: modelData.avatar

                        MouseArea {
                            anchors.fill: parent

                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                readMarkerDialog.listModel = userMarker
                                readMarkerDialog.open()
                            }
                        }
                    }
                }

                TimelineLabel {
                    id: timeLabel

                    visible: Math.abs(time - aboveTime) > 600000 || index == 0
                    text: Qt.formatTime(time, "hh:mm")
                    coloredBackground: highlighted
                    foreground: "grey"
                    font.pointSize: 8
                }
            }

            ReactionDelegate {
                Layout.fillWidth: true
            }

            // FORK-ONLY: resend/discard ported from Aug 2019 (Controls 1
            // Label + MouseArea instead of Controls 2 hover links).
            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 8
                visible: messageRow.failed

                TimelineLabel {
                    text: "Send failed:"
                    coloredBackground: highlighted
                    foreground: "grey"
                    font.pointSize: 8
                }

                TimelineLabel {
                    text: "Resend"
                    coloredBackground: highlighted
                    foreground: PPalette.accent
                    font.pointSize: 8
                    font.underline: true

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: currentRoom.retryMessage(eventId)
                    }
                }

                TimelineLabel {
                    text: "|"
                    coloredBackground: highlighted
                    foreground: "grey"
                    font.pointSize: 8
                }

                TimelineLabel {
                    text: "Discard"
                    coloredBackground: highlighted
                    foreground: PPalette.accent
                    font.pointSize: 8
                    font.underline: true

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: currentRoom.discardMessage(eventId)
                    }
                }
            }
        }

        Component {
            id: imageComponent

            DownloadableContent {
                width: messageImage.width
                height: messageImage.height

                id: downloadable

                TimelineImage {
                    z: -4

                    id: messageImage

                    sourceSize: 128
                    source: "image://mxc/" + (content.thumbnail_url ? content.thumbnail_url : content.url)

                    onClicked: downloadAndOpen()
                }

                Component.onCompleted: {
                    messageRow.saveFileAs.connect(saveFileAs)
                    messageRow.openExternally.connect(downloadAndOpen)
                }
            }
        }

        Component {
            id: fileComponent

            TimelineLabel {
                Layout.fillWidth: true

                id: downloadDelegate

                text: "<b>File: </b>" + content.body
                coloredBackground: highlighted

                background: DownloadableContent {
                    id: downloadable

                    Component.onCompleted: {
                        messageRow.saveFileAs.connect(saveFileAs)
                        messageRow.openExternally.connect(downloadAndOpen)
                    }
                }
            }
        }

        Component {
            id: audioComponent

            TimelineLabel {
                id: downloadDelegate

                text: content.info.duration / 1000 + '"'
                coloredBackground: highlighted

                MouseArea {
                    anchors.fill: parent

                    propagateComposedEvents: true

                    onClicked: {
                        if (downloadable.downloaded)
                            progressiveController.playAudio(progressInfo.localPath)
                        else
                        {
                            playOnFinished = true
                            currentRoom.downloadFile(eventId, cacheLocation + "/" + eventId.replace(":", "_") + ".tmp")
                        }
                    }
                }

                background: DownloadableContent {
                    id: downloadable

                    onDownloadedChanged: downloaded && playOnFinished ? progressiveController.playAudio(progressInfo.localPath) : {}

                    Component.onCompleted: {
                        messageRow.saveFileAs.connect(saveFileAs)
                        messageRow.openExternally.connect(downloadAndOpen)
                    }
                }
            }
        }
    }
}
