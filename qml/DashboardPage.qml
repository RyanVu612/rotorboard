import QtQuick
import QtCore

Rectangle {
    id: root

    required property var controller

    color: "#101418"

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
                    width: pauseLabel.implicitWidth + 20
                    height: 34
                    radius: 5
                    color: root.controller.chartsFrozen ? "#493316" : "#182028"
                    border.color: root.controller.chartsFrozen ? "#c58b2c" : "#2c3843"

                    Text {
                        id: pauseLabel
                        anchors.centerIn: parent
                        text: root.controller.chartsFrozen ? "Resume" : "Pause"
                        color: root.controller.chartsFrozen ? "#ffd18a" : "#c9d2d8"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.controller.toggleChartsFrozen()
                    }
                }

                Rectangle {
                    width: editLabel.implicitWidth + 20
                    height: 34
                    radius: 5
                    color: root.controller.editMode ? "#2a3640" : "#182028"
                    border.color: root.controller.editMode ? "#3d4d59" : "#2c3843"

                    Text {
                        id: editLabel
                        anchors.centerIn: parent
                        text: root.controller.editMode ? "Done" : "Edit layout"
                        color: "#c9d2d8"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.controller.editMode = !root.controller.editMode
                            if (!root.controller.editMode && dashboardGrid)
                                dashboardGrid.pendingWidgetType = ""
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

        Row {
            width: parent.width
            height: parent.height - y
            spacing: 16

            DashboardGrid {
                id: dashboardGrid
                width: root.controller.editMode ? parent.width - widgetPalette.width - parent.spacing : parent.width
                height: parent.height
                telemetryModel: root.controller.telemetryModel
                layoutModel: root.controller.layoutModel
                chartsFrozen: root.controller.chartsFrozen
                editMode: root.controller.editMode
            }

            WidgetPalette {
                id: widgetPalette
                height: parent.height
                controller: root.controller
                dashboardGrid: dashboardGrid
            }
        }
    }
}
