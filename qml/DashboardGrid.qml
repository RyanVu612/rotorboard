import QtQuick
import QtQuick.Controls
import QtCore
import "widgets"

Flickable {
    id: root

    required property var telemetryModel
    required property var layoutModel
    required property bool chartsFrozen
    required property bool editMode
    property string pendingWidgetType: ""
    property int pendingMotorId: 1
    property string pendingMetric: "rpm"
    property string selectedWidgetId: ""

    readonly property int gridColumns: 16
    readonly property int gridRows: 20
    readonly property int cellWidth: 88
    readonly property int cellHeight: 72
    readonly property int cardMargin: 4
    readonly property int telemetryRevision: telemetryModel ? telemetryModel.sampleRevision : 0

    property int dragLockCount: 0
    property string resizingWidgetId: ""

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.VerticalFlick
    interactive: dragLockCount === 0
    contentWidth: gridColumns * cellWidth
    contentHeight: gridRows * cellHeight

    ScrollBar.vertical: ScrollBar {
        id: gridScrollBar
        policy: ScrollBar.AsNeeded
        contentItem: Rectangle {
            implicitWidth: 6
            radius: 3
            color: gridScrollBar.pressed ? "#5a6a75" : gridScrollBar.hovered ? "#4a5862" : "#3a4852"
        }
        background: Rectangle {
            implicitWidth: 8
            color: "#141a20"
            radius: 4
        }
    }

    function clamp(value, min, max) {
        return Math.max(min, Math.min(max, value))
    }

    function snapColFromTopLeft(topLeftX, colSpan) {
        const cellIndex = Math.round((topLeftX - cardMargin) / cellWidth)
        return clamp(cellIndex + 1, 1, gridColumns - colSpan + 1)
    }

    function snapRowFromTopLeft(topLeftY, rowSpan) {
        const cellIndex = Math.round((topLeftY - cardMargin) / cellHeight)
        return clamp(cellIndex + 1, 1, gridRows - rowSpan + 1)
    }

    function metricValue(motorId, metric) {
        telemetryRevision
        if (!telemetryModel || !metric)
            return 0
        const v = telemetryModel.valueForMetric(motorId, metric)
        if (v === undefined || v === null)
            return 0
        const n = Number(v)
        return isNaN(n) ? 0 : n
    }

    function metricHistory(motorId, metric) {
        telemetryRevision
        if (!telemetryModel || !metric)
            return []
        const h = telemetryModel.historyForMetric(motorId, metric)
        return h === undefined || h === null ? [] : h
    }

    function motorStale(motorId) {
        telemetryRevision
        if (!telemetryModel)
            return true
        return telemetryModel.isMotorStale(motorId)
    }

    function motorWarningLevel(motorId) {
        telemetryRevision
        if (!telemetryModel)
            return 0
        return telemetryModel.warningLevelForMotor(motorId)
    }

    function motorStatus(motorId) {
        telemetryRevision
        if (!telemetryModel)
            return ""
        const s = telemetryModel.statusForMotor(motorId)
        return s === undefined || s === null ? "" : s
    }

    function beginCardDrag() { dragLockCount++ }
    function endCardDrag() { dragLockCount = Math.max(0, dragLockCount - 1) }
    function beginResize(widgetId) { resizingWidgetId = widgetId }
    function endResize() { resizingWidgetId = "" }

    function selectWidget(widgetId) {
        selectedWidgetId = widgetId
    }

    function placeWidget(widgetId, col, row) {
        layoutModel.placeWidget(widgetId, col, row)
    }

    function resizeWidget(widgetId, colSpan, rowSpan) {
        layoutModel.resizeWidget(widgetId, colSpan, rowSpan)
    }

    function placePendingWidget(col, row) {
        if (!pendingWidgetType)
            return

        const metric = pendingWidgetType === "motorSummary" ? "" : pendingMetric
        layoutModel.addWidget(pendingWidgetType,
                              pendingMotorId,
                              metric,
                              col,
                              row,
                              layoutModel.defaultColSpan(pendingWidgetType),
                              layoutModel.defaultRowSpan(pendingWidgetType))
        pendingWidgetType = ""
    }

    function ensureMotorWidgets() {
        if (!telemetryModel || !layoutModel)
            return
        for (let i = 0; i < telemetryModel.rowCount(); ++i) {
            const motorId = telemetryModel.motorIdAt(i)
            if (motorId > 0 && !layoutModel.hasWidgetsForMotor(motorId))
                layoutModel.seedForMotor(motorId)
        }
    }

    Connections {
        target: telemetryModel
        function onRowsInserted() { Qt.callLater(root.ensureMotorWidgets) }
        function onModelReset() { Qt.callLater(root.ensureMotorWidgets) }
    }

    Component.onCompleted: ensureMotorWidgets()

    Item {
        width: root.contentWidth
        height: root.contentHeight

        Repeater {
            model: root.gridColumns * root.gridRows

            Rectangle {
                required property int index
                readonly property int cellCol: (index % root.gridColumns) + 1
                readonly property int cellRow: Math.floor(index / root.gridColumns) + 1

                x: (cellCol - 1) * root.cellWidth
                y: (cellRow - 1) * root.cellHeight
                width: root.cellWidth
                height: root.cellHeight
                color: "transparent"
                border.color: root.editMode ? "#24303a" : "#1a232b"
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    enabled: root.editMode && root.pendingWidgetType !== ""
                    onClicked: root.placePendingWidget(cellCol, cellRow)
                }
            }
        }

        Repeater {
            model: root.layoutModel

            DashboardWidgetSlot {
                required property var model

                grid: root
                editMode: root.editMode
                selected: root.selectedWidgetId === model.widgetId
                widgetId: model.widgetId
                widgetType: model.widgetType
                motorId: model.motorId
                metric: model.metric
                gridCol: model.col
                gridRow: model.row
                gridColSpan: model.colSpan
                gridRowSpan: model.rowSpan

                ValueTile {
                    anchors.fill: parent
                    visible: model.widgetType === "value"
                    motorId: model.motorId
                    metric: model.metric
                    value: root.metricValue(model.motorId, model.metric)
                    statusText: root.motorStatus(model.motorId)
                    isStale: root.motorStale(model.motorId)
                    warningLevel: root.motorWarningLevel(model.motorId)
                    chartsFrozen: root.chartsFrozen
                }

                ChartTile {
                    anchors.fill: parent
                    visible: model.widgetType === "chart"
                    motorId: model.motorId
                    metric: model.metric
                    history: root.metricHistory(model.motorId, model.metric)
                    isStale: root.motorStale(model.motorId)
                    warningLevel: root.motorWarningLevel(model.motorId)
                    chartsFrozen: root.chartsFrozen
                }

                MotorSummaryCard {
                    anchors.fill: parent
                    visible: model.widgetType === "motorSummary"
                    motorId: model.motorId
                    rpm: root.metricValue(model.motorId, "rpm")
                    voltage: root.metricValue(model.motorId, "voltage")
                    current: root.metricValue(model.motorId, "current")
                    temperatureCelsius: root.metricValue(model.motorId, "temperatureCelsius")
                    pwm: root.metricValue(model.motorId, "pwm")
                    status: root.motorStatus(model.motorId)
                    isStale: root.motorStale(model.motorId)
                    warningLevel: root.motorWarningLevel(model.motorId)
                    rpmHistory: root.metricHistory(model.motorId, "rpm")
                    voltageHistory: root.metricHistory(model.motorId, "voltage")
                    currentHistory: root.metricHistory(model.motorId, "current")
                    temperatureHistory: root.metricHistory(model.motorId, "temperatureCelsius")
                    pwmHistory: root.metricHistory(model.motorId, "pwm")
                    chartsFrozen: root.chartsFrozen
                }
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: !layoutModel || layoutModel.rowCount() === 0
        text: root.editMode ? "Add widgets from the palette" : "Waiting for telemetry"
        color: "#7e8b93"
        font.pixelSize: 16
    }
}
