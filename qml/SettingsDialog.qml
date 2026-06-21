import QtQuick
import QtQuick.Controls

Popup {
    id: root

    required property var controller

    modal: true
    focus: true
    width: 420
    height: contentColumn.implicitHeight + 36
    padding: 18
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string draftInputMethod: "fake"
    property string draftPlaybackPath: ""
    property string draftMavlinkSerialPort: ""
    property int draftMavlinkSerialBaud: 115200
    property string draftMavlinkTcpHost: "127.0.0.1"
    property int draftMavlinkTcpPort: 5760

    function openWithCurrentSettings() {
        draftInputMethod = controller.inputMethod
        draftPlaybackPath = controller.playbackPath
        draftMavlinkSerialPort = controller.mavlinkSerialPort
        draftMavlinkSerialBaud = controller.mavlinkSerialBaud
        draftMavlinkTcpHost = controller.mavlinkTcpHost
        draftMavlinkTcpPort = controller.mavlinkTcpPort
        open()
    }

    background: Rectangle {
        radius: 6
        color: "#151b21"
        border.color: "#33414d"
    }

    contentItem: Column {
        id: contentColumn
        width: parent.width
        spacing: 16

        Text {
            text: "Settings"
            color: "#f4f7f8"
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }

        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "Input method"
                color: "#92a0a8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            Row {
                width: parent.width
                spacing: 8

                Repeater {
                    model: [
                        { label: "Pixhawk", value: "pixhawk" },
                        { label: "MAVLink TCP", value: "mavlink_tcp" },
                        { label: "CSV Replay", value: "playback" },
                        { label: "Fake Data", value: "fake" }
                    ]

                    Rectangle {
                        required property var modelData

                        width: (parent.width - 24) / 4
                        height: 34
                        radius: 5
                        color: root.draftInputMethod === modelData.value ? "#23415a" : "#182028"
                        border.color: root.draftInputMethod === modelData.value ? "#4f9bd0" : "#2c3843"

                        Text {
                            anchors.centerIn: parent
                            text: modelData.label
                            color: root.draftInputMethod === modelData.value ? "#dff1ff" : "#c9d2d8"
                            font.pixelSize: 12
                            font.weight: Font.Medium
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.draftInputMethod = modelData.value
                        }
                    }
                }
            }
        }

        Column {
            width: parent.width
            spacing: 8
            visible: root.draftInputMethod === "pixhawk"

            Text {
                text: "Pixhawk MAVLink serial"
                color: "#92a0a8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                width: parent.width
                height: 36
                text: root.draftMavlinkSerialPort
                placeholderText: "Serial port, blank for auto"
                color: "#f4f7f8"
                placeholderTextColor: "#687680"
                selectionColor: "#4f9bd0"
                selectedTextColor: "#101418"
                onTextChanged: root.draftMavlinkSerialPort = text
            }

            TextField {
                width: parent.width
                height: 36
                text: String(root.draftMavlinkSerialBaud)
                placeholderText: "Baud"
                inputMethodHints: Qt.ImhDigitsOnly
                color: "#f4f7f8"
                placeholderTextColor: "#687680"
                selectionColor: "#4f9bd0"
                selectedTextColor: "#101418"
                onTextChanged: {
                    const parsed = parseInt(text)
                    if (!isNaN(parsed))
                        root.draftMavlinkSerialBaud = parsed
                }
            }
        }

        Column {
            width: parent.width
            spacing: 8
            visible: root.draftInputMethod === "mavlink_tcp"

            Text {
                text: "MAVLink TCP"
                color: "#92a0a8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                width: parent.width
                height: 36
                text: root.draftMavlinkTcpHost
                placeholderText: "Host (e.g. 127.0.0.1)"
                color: "#f4f7f8"
                placeholderTextColor: "#687680"
                selectionColor: "#4f9bd0"
                selectedTextColor: "#101418"
                onTextChanged: root.draftMavlinkTcpHost = text
            }

            TextField {
                width: parent.width
                height: 36
                text: String(root.draftMavlinkTcpPort)
                placeholderText: "Port (e.g. 5760)"
                inputMethodHints: Qt.ImhDigitsOnly
                color: "#f4f7f8"
                placeholderTextColor: "#687680"
                selectionColor: "#4f9bd0"
                selectedTextColor: "#101418"
                onTextChanged: {
                    const parsed = parseInt(text)
                    if (!isNaN(parsed))
                        root.draftMavlinkTcpPort = parsed
                }
            }
        }

        Column {
            width: parent.width
            spacing: 8
            visible: root.draftInputMethod === "playback"

            Text {
                text: "CSV file"
                color: "#92a0a8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            TextField {
                width: parent.width
                height: 36
                text: root.draftPlaybackPath
                placeholderText: "samples/session.csv"
                color: "#f4f7f8"
                placeholderTextColor: "#687680"
                selectionColor: "#4f9bd0"
                selectedTextColor: "#101418"
                onTextChanged: root.draftPlaybackPath = text
            }
        }

        Row {
            width: parent.width
            height: 34
            spacing: 10
            layoutDirection: Qt.RightToLeft

            Rectangle {
                width: 82
                height: 34
                radius: 5
                color: "#23415a"
                border.color: "#4f9bd0"

                Text {
                    anchors.centerIn: parent
                    text: "Apply"
                    color: "#dff1ff"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.controller.inputMethod = root.draftInputMethod
                        root.controller.playbackPath = root.draftPlaybackPath
                        root.controller.mavlinkSerialPort = root.draftMavlinkSerialPort
                        root.controller.mavlinkSerialBaud = root.draftMavlinkSerialBaud
                        root.controller.mavlinkTcpHost = root.draftMavlinkTcpHost
                        root.controller.mavlinkTcpPort = root.draftMavlinkTcpPort
                        root.controller.applyTelemetrySource()
                        root.close()
                    }
                }
            }

            Rectangle {
                width: 82
                height: 34
                radius: 5
                color: "#182028"
                border.color: "#2c3843"

                Text {
                    anchors.centerIn: parent
                    text: "Cancel"
                    color: "#c9d2d8"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
            }
        }
    }
}
