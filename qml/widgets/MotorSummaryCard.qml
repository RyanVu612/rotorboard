import QtQuick
import Rotorboard
import "../MetricUtils.js" as MetricUtils

Rectangle {
    id: root

    required property var telemetryModel
    required property int motorId
    required property real rpm
    required property real voltage
    required property real current
    required property real temperatureCelsius
    required property real pwm
    required property string status
    required property bool isStale
    required property int warningLevel
    required property bool chartsFrozen

    // Live history for each metric, pulled directly from the model when this
    // motor's history changes — deliberately NOT bound to the global
    // sampleRevision/telemetryRevision, so unrelated motor updates do not force
    // this card to recopy five ring buffers or repaint. Same pattern as ChartTile.
    property var rpmHistory: []
    property var voltageHistory: []
    property var currentHistory: []
    property var temperatureHistory: []
    property var pwmHistory: []

    // Coalesces refreshes to at most ~30 fps even if telemetry arrives faster.
    property bool _historyDirty: false

    property real heldRpm: rpm
    property real heldVoltage: voltage
    property real heldCurrent: current
    property real heldTemperatureCelsius: temperatureCelsius
    property real heldPwm: pwm
    property string heldStatus: status
    property var frozenRpmHistory: []
    property var frozenVoltageHistory: []
    property var frozenCurrentHistory: []
    property var frozenTemperatureHistory: []
    property var frozenPwmHistory: []
    property int heldWarningLevel: warningLevel
    property bool heldIsStale: isStale

    readonly property int shownWarningLevel: chartsFrozen ? heldWarningLevel : warningLevel
    readonly property bool shownIsStale: chartsFrozen ? heldIsStale : isStale

    function refreshHistory() {
        if (!telemetryModel) {
            rpmHistory = []
            voltageHistory = []
            currentHistory = []
            temperatureHistory = []
            pwmHistory = []
            return
        }
        rpmHistory = telemetryModel.historyForMetric(motorId, "rpm") || []
        voltageHistory = telemetryModel.historyForMetric(motorId, "voltage") || []
        currentHistory = telemetryModel.historyForMetric(motorId, "current") || []
        temperatureHistory = telemetryModel.historyForMetric(motorId, "temperatureCelsius") || []
        pwmHistory = telemetryModel.historyForMetric(motorId, "pwm") || []
    }

    onMotorIdChanged: refreshHistory()
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
        if (!chartsFrozen) {
            // Pull whatever accumulated while frozen.
            refreshHistory()
            return
        }
        heldRpm = rpm
        heldVoltage = voltage
        heldCurrent = current
        heldTemperatureCelsius = temperatureCelsius
        heldPwm = pwm
        heldStatus = status
        heldWarningLevel = warningLevel
        heldIsStale = isStale
        frozenRpmHistory = MetricUtils.copySeries(rpmHistory)
        frozenVoltageHistory = MetricUtils.copySeries(voltageHistory)
        frozenCurrentHistory = MetricUtils.copySeries(currentHistory)
        frozenTemperatureHistory = MetricUtils.copySeries(temperatureHistory)
        frozenPwmHistory = MetricUtils.copySeries(pwmHistory)
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
        anchors.margins: 12
        spacing: 8

        Row {
            width: parent.width
            height: 24

            Text {
                width: parent.width - 78
                text: "Motor " + root.motorId
                color: "#f3f6f7"
                font.pixelSize: 19
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            StatusBadge {
                width: 78
                height: 24
                warningLevel: root.shownWarningLevel
            }
        }

        Text {
            width: parent.width
            text: Math.round(root.chartsFrozen ? root.heldRpm : root.rpm).toLocaleString(Qt.locale()) + " rpm"
            color: root.shownIsStale ? "#879199" : "#f4f7f8"
            font.pixelSize: 28
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Sparkline {
            width: parent.width
            height: 20
            values: root.chartsFrozen ? root.frozenRpmHistory : (root.rpmHistory || [])
            strokeColor: "#6eb5d8"
            dimmed: root.shownIsStale
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#26323b"
        }

        Grid {
            width: parent.width
            columns: 2
            rowSpacing: 8
            columnSpacing: 10

            Repeater {
                model: [
                    {label: "Voltage", value: (root.chartsFrozen ? root.heldVoltage : root.voltage).toFixed(1) + " V", history: root.chartsFrozen ? root.frozenVoltageHistory : root.voltageHistory, color: "#7a9eb8"},
                    {label: "Current", value: (root.chartsFrozen ? root.heldCurrent : root.current).toFixed(1) + " A", history: root.chartsFrozen ? root.frozenCurrentHistory : root.currentHistory, color: "#d4a05c"},
                    {label: "Temp", value: (root.chartsFrozen ? root.heldTemperatureCelsius : root.temperatureCelsius).toFixed(1) + " C", history: root.chartsFrozen ? root.frozenTemperatureHistory : root.temperatureHistory, color: "#c97d6f"},
                    {label: "PWM", value: Math.round(root.chartsFrozen ? root.heldPwm : root.pwm).toString(), history: root.chartsFrozen ? root.frozenPwmHistory : root.pwmHistory, color: "#7a9eb8"}
                ]

                Column {
                    width: (parent.width - parent.columnSpacing) / 2
                    spacing: 2

                    Text {
                        width: parent.width
                        text: modelData.label
                        color: "#77858e"
                        font.pixelSize: 10
                        font.capitalization: Font.AllUppercase
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: modelData.value
                        color: "#d6dde1"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }

                    Sparkline {
                        width: parent.width
                        height: 14
                        visible: modelData.history !== null
                        values: modelData.history || []
                        strokeColor: modelData.color
                        dimmed: root.shownIsStale
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: 14

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: root.shownIsStale ? "No recent data" : (root.chartsFrozen ? root.heldStatus : root.status)
                color: root.shownIsStale ? "#a9b2b8" : "#7ed39a"
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }
    }
}
