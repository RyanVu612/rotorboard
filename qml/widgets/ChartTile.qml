import QtQuick
import Rotorboard
import "../MetricUtils.js" as MetricUtils

Rectangle {
    id: root

    required property var telemetryModel
    required property int motorId
    required property string metric
    required property bool isStale
    required property int warningLevel
    required property bool chartsFrozen

    // Live history for this (motor, metric). Pulled directly from the model when
    // this motor's history changes — deliberately NOT bound to the global
    // sampleRevision/telemetryRevision, so unrelated motor updates do not force
    // this tile to recopy its ring buffer or repaint.
    property var history: []
    property var frozenHistory: []
    property int heldWarningLevel: warningLevel
    property bool heldIsStale: isStale

    // Coalesces repaints to at most ~30 fps even if telemetry arrives faster.
    property bool _historyDirty: false

    readonly property int shownWarningLevel: chartsFrozen ? heldWarningLevel : warningLevel
    readonly property bool shownIsStale: chartsFrozen ? heldIsStale : isStale

    function refreshHistory() {
        if (!telemetryModel || !metric) {
            history = []
            return
        }
        const h = telemetryModel.historyForMetric(motorId, metric)
        history = (h === undefined || h === null) ? [] : h
    }

    onMotorIdChanged: refreshHistory()
    onMetricChanged: refreshHistory()
    Component.onCompleted: refreshHistory()

    Connections {
        target: root.telemetryModel
        function onMotorHistoryChanged(changedMotorId) {
            if (changedMotorId !== root.motorId)
                return
            // Don't refresh while frozen — the frozen snapshot must hold.
            if (root.chartsFrozen)
                return
            root._historyDirty = true
            if (!refreshThrottle.running)
                refreshThrottle.start()
        }
    }

    Timer {
        id: refreshThrottle
        interval: 33 // ~30 fps cap
        repeat: false
        onTriggered: {
            if (root._historyDirty && !root.chartsFrozen) {
                root._historyDirty = false
                root.refreshHistory()
            }
        }
    }

    onChartsFrozenChanged: {
        if (chartsFrozen) {
            frozenHistory = MetricUtils.copySeries(history)
            heldWarningLevel = warningLevel
            heldIsStale = isStale
        } else {
            frozenHistory = []
            // Pull whatever accumulated while frozen.
            refreshHistory()
        }
    }

    clip: true
    radius: 8
    color: shownWarningLevel === 3 ? "#191d21" : "#161c22"
    border.width: 1
    border.color: {
        if (shownWarningLevel === 3) return "#6b7480"
        if (shownWarningLevel === 2) return "#d05246"
        if (shownWarningLevel === 1) return "#c58b2c"
        return "#2d3a43"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 6

        Text {
            width: parent.width
            text: "M" + root.motorId + " " + MetricUtils.label(root.metric)
            color: "#c9d2d8"
            font.pixelSize: 12
            font.weight: Font.Medium
            elide: Text.ElideRight
        }

        Sparkline {
            width: parent.width
            height: Math.max(40, parent.height - 24)
            values: root.chartsFrozen ? root.frozenHistory : (root.history || [])
            strokeColor: MetricUtils.strokeColor(root.metric)
            dimmed: root.shownIsStale
        }
    }
}
