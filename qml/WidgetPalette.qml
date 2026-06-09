import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    required property var controller
    required property var dashboardGrid

    width: 280
    radius: 8
    color: "#182028"
    border.color: "#2c3843"
    border.width: 1
    visible: controller.editMode

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Text {
            text: "Widget palette"
            color: "#f4f7f8"
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        Text {
            width: parent.width
            text: "Motor"
            color: "#92a0a8"
            font.pixelSize: 11
        }

        SpinBox {
            id: motorSpinBox
            from: 1
            to: 32
            value: dashboardGrid ? dashboardGrid.pendingMotorId : 1
            onValueChanged: if (dashboardGrid) dashboardGrid.pendingMotorId = value
        }

        Text {
            width: parent.width
            text: "Metric"
            color: "#92a0a8"
            font.pixelSize: 11
        }

        ComboBox {
            id: metricCombo
            width: parent.width
            model: ["rpm", "voltage", "current", "temperatureCelsius", "pwm"]
            onActivated: if (dashboardGrid) dashboardGrid.pendingMetric = currentText
            Component.onCompleted: if (dashboardGrid) dashboardGrid.pendingMetric = currentText
        }

        Repeater {
            model: [
                {label: "Add value tile", type: "value"},
                {label: "Add chart", type: "chart"},
                {label: "Add motor summary", type: "motorSummary"}
            ]

            Rectangle {
                width: parent.width
                height: 32
                radius: 5
                color: dashboardGrid && dashboardGrid.pendingWidgetType === modelData.type ? "#2a3640" : "#1f2830"
                border.color: "#3d4d59"

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: "#e2e8ec"
                    font.pixelSize: 12
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!dashboardGrid)
                            return
                        dashboardGrid.pendingWidgetType =
                            dashboardGrid.pendingWidgetType === modelData.type ? "" : modelData.type
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 32
            radius: 5
            color: "#4d211d"
            border.color: "#d05246"
            visible: dashboardGrid && dashboardGrid.selectedWidgetId !== ""

            Text {
                anchors.centerIn: parent
                text: "Delete selected"
                color: "#ffb8b2"
                font.pixelSize: 12
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (!dashboardGrid || dashboardGrid.selectedWidgetId === "")
                        return
                    controller.layoutModel.removeWidget(dashboardGrid.selectedWidgetId)
                    dashboardGrid.selectedWidgetId = ""
                }
            }
        }

        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            text: dashboardGrid && dashboardGrid.pendingWidgetType !== ""
                ? "Click a grid cell to place the widget."
                : "Drag widgets by the top handle. Resize from the bottom-right corner."
            color: "#7e8b93"
            font.pixelSize: 11
        }
    }
}
