import QtQuick
import QtCore

Rectangle {
    id: root

    required property var controller

    readonly property int layoutModeCompact: 0
    readonly property int layoutModeDetailed: 1

    color: "#101418"

    Settings {
        id: dashboardSettings
        category: "Dashboard"
        property int layoutMode: root.layoutModeDetailed
    }

    property alias layoutMode: dashboardSettings.layoutMode

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        Row {
            width: parent.width
            height: 48
            spacing: 16

            Column {
                width: Math.max(0, parent.width - rightControls.width - parent.spacing)
                spacing: 3

                Text {
                    text: "Rotorboard"
                    color: "#f4f7f8"
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Live propulsion telemetry"
                    color: "#92a0a8"
                    font.pixelSize: 13
                }
            }

            Row {
                id: rightControls
                height: 34
                spacing: 12

                Rectangle {
                    width: layoutToggleRow.width + 8
                    height: 34
                    radius: 5
                    color: "#182028"
                    border.color: "#2c3843"

                    Row {
                        id: layoutToggleRow
                        anchors.centerIn: parent
                        spacing: 2

                        Repeater {
                            model: [
                                {label: "Compact", mode: root.layoutModeCompact},
                                {label: "Detailed", mode: root.layoutModeDetailed}
                            ]

                            Rectangle {
                                width: layoutLabel.implicitWidth + 20
                                height: 28
                                radius: 4
                                color: root.layoutMode === modelData.mode ? "#2a3640" : "transparent"
                                border.color: root.layoutMode === modelData.mode ? "#3d4d59" : "transparent"

                                Text {
                                    id: layoutLabel
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    color: root.layoutMode === modelData.mode ? "#e2e8ec" : "#8b98a1"
                                    font.pixelSize: 12
                                    font.weight: root.layoutMode === modelData.mode ? Font.Medium : Font.Normal
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.layoutMode = modelData.mode
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: Math.min(164, Math.max(120, sourceLabelText.implicitWidth + 24))
                    height: 34
                    radius: 5
                    color: "#182028"
                    border.color: "#2c3843"

                    Text {
                        id: sourceLabelText
                        anchors.centerIn: parent
                        text: root.controller.sourceLabel
                        color: "#c9d2d8"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                    }
                }
            }
        }

        MotorGrid {
            width: parent.width
            height: parent.height - y
            model: root.controller.telemetryModel
            layoutMode: root.layoutMode
        }
    }
}
