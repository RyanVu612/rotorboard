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
                    id: addButton
                    width: addLabel.implicitWidth + 20
                    height: 34
                    radius: 5
                    color: "#182028"
                    border.color: "#2c3843"

                    Text {
                        id: addLabel
                        anchors.centerIn: parent
                        text: "+ Add widget"
                        color: "#c9d2d8"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (dashboardGrid.ghostActive)
                                dashboardGrid.cancelGhost()
                            widgetDialog.openForAdd()
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

        DashboardGrid {
            id: dashboardGrid
            width: parent.width
            height: parent.height - y
            telemetryModel: root.controller.telemetryModel
            layoutModel: root.controller.layoutModel
            chartsFrozen: root.controller.chartsFrozen

            onEditRequested: function(widgetId, widgetType, motorId, metric) {
                widgetDialog.openForEdit(widgetId, widgetType, motorId, metric)
            }
        }
    }

    WidgetDialog {
        id: widgetDialog
        anchors.centerIn: parent

        onConfirmed: function(mode, widgetId, widgetType, motorId, metric) {
            if (mode === "edit")
                root.controller.layoutModel.editWidget(widgetId, widgetType, motorId, metric)
            else
                dashboardGrid.startGhost(widgetType, motorId, metric)
        }
    }

    // While a ghost placement is active this overlay owns the mouse: it tracks
    // the cursor over the grid, commits on left-click inside the grid, and
    // cancels on right-click or any click outside the grid.
    MouseArea {
        id: ghostOverlay
        anchors.fill: parent
        visible: dashboardGrid.ghostActive
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        z: 1000

        function gridPos(mouse) {
            return ghostOverlay.mapToItem(dashboardGrid, mouse.x, mouse.y)
        }

        function insideGrid(pos) {
            return pos.x >= 0 && pos.y >= 0 &&
                   pos.x < dashboardGrid.width && pos.y < dashboardGrid.height
        }

        onPositionChanged: function(mouse) {
            const pos = gridPos(mouse)
            if (insideGrid(pos))
                dashboardGrid.updateGhost(pos.x, pos.y)
            else
                dashboardGrid.ghostVisible = false
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                dashboardGrid.cancelGhost()
                return
            }
            const pos = gridPos(mouse)
            if (insideGrid(pos)) {
                dashboardGrid.updateGhost(pos.x, pos.y)
                dashboardGrid.commitGhost()
                return
            }
            dashboardGrid.cancelGhost()
            // Clicking the Add button while a ghost is active cancels the
            // ghost and reopens the popup for a fresh widget.
            const buttonPos = ghostOverlay.mapToItem(addButton, mouse.x, mouse.y)
            if (buttonPos.x >= 0 && buttonPos.y >= 0 &&
                buttonPos.x < addButton.width && buttonPos.y < addButton.height) {
                widgetDialog.openForAdd()
            }
        }
    }

    Shortcut {
        sequence: "Escape"
        enabled: dashboardGrid.ghostActive
        context: Qt.ApplicationShortcut
        onActivated: dashboardGrid.cancelGhost()
    }
}
