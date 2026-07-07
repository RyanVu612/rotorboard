import QtQuick
import Rotorboard

Rectangle {
    id: root
    objectName: "batterySummaryCard-" + batteryId

    required property var telemetryModel
    required property int batteryId
    required property real voltage
    required property real current
    required property real batteryRemaining
    required property real temperatureCelsius
    required property real currentConsumedMah
    required property var cellVoltages
    required property bool isStale
    required property int warningLevel
    required property bool chartsFrozen

    property real heldVoltage: voltage
    property real heldCurrent: current
    property real heldBatteryRemaining: batteryRemaining
    property real heldTemperatureCelsius: temperatureCelsius
    property real heldCurrentConsumedMah: currentConsumedMah
    property var frozenCellVoltages: []
    property int heldWarningLevel: warningLevel
    property bool heldIsStale: isStale

    readonly property int shownWarningLevel: chartsFrozen ? heldWarningLevel : warningLevel
    readonly property bool shownIsStale: chartsFrozen ? heldIsStale : isStale

    function copyCells(values) {
        if (!values)
            return []
        const copy = []
        for (let i = 0; i < values.length; ++i)
            copy.push(Number(values[i]))
        return copy
    }

    function shownVoltage() { return chartsFrozen ? heldVoltage : voltage }
    function shownCurrent() { return chartsFrozen ? heldCurrent : current }
    function shownBatteryRemaining() { return chartsFrozen ? heldBatteryRemaining : batteryRemaining }
    function shownTemperatureCelsius() { return chartsFrozen ? heldTemperatureCelsius : temperatureCelsius }
    function shownCurrentConsumedMah() { return chartsFrozen ? heldCurrentConsumedMah : currentConsumedMah }

    function formatCurrent(value) { return value === -1 ? "\u2014" : value.toFixed(1) + " A" }
    function formatRemaining(value) { return value < 0 ? "N/A" : Math.round(value) + "%" }
    function formatTemperature(value) { return isNaN(value) ? "\u2014" : value.toFixed(1) + " C" }

    onChartsFrozenChanged: {
        if (!chartsFrozen)
            return
        heldVoltage = voltage
        heldCurrent = current
        heldBatteryRemaining = batteryRemaining
        heldTemperatureCelsius = temperatureCelsius
        heldCurrentConsumedMah = currentConsumedMah
        frozenCellVoltages = copyCells(cellVoltages)
        heldWarningLevel = warningLevel
        heldIsStale = isStale
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
                text: "Battery " + root.batteryId
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

        Grid {
            width: parent.width
            columns: 2
            rowSpacing: 8
            columnSpacing: 10

            Repeater {
                model: [
                    {key: "voltage", label: "Voltage", value: root.shownVoltage().toFixed(1) + " V"},
                    {key: "current", label: "Current", value: root.formatCurrent(root.shownCurrent())},
                    {key: "remaining", label: "% Remaining", value: root.formatRemaining(root.shownBatteryRemaining())},
                    {key: "temperature", label: "Temp", value: root.formatTemperature(root.shownTemperatureCelsius())},
                    {key: "consumed", label: "Consumed mAh", value: Math.round(root.shownCurrentConsumedMah()) + " mAh"}
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
                        objectName: "batteryMetricValue-" + modelData.key
                        width: parent.width
                        text: modelData.value
                        color: root.shownIsStale ? "#879199" : "#d6dde1"
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#26323b"
        }

        Grid {
            width: parent.width
            columns: 2
            rowSpacing: 4
            columnSpacing: 8

            Repeater {
                model: root.chartsFrozen ? root.frozenCellVoltages : (root.cellVoltages || [])

                Text {
                    width: (parent.width - parent.columnSpacing) / 2
                    text: "Cell " + (index + 1) + ": " + Number(modelData).toFixed(2) + " V"
                    color: root.shownIsStale ? "#879199" : "#aeb9bf"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
        }

        Item {
            width: parent.width
            height: 14

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: root.shownIsStale ? "No recent data" : ""
                color: "#a9b2b8"
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }
    }
}
