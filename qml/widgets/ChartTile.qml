import QtQuick
import Rotorboard
import "../MetricUtils.js" as MetricUtils

Rectangle {
    id: root

    required property int motorId
    required property string metric
    required property var history
    required property bool isStale
    required property int warningLevel
    required property bool chartsFrozen

    property var frozenHistory: []
    property int heldWarningLevel: warningLevel
    property bool heldIsStale: isStale

    readonly property int shownWarningLevel: chartsFrozen ? heldWarningLevel : warningLevel
    readonly property bool shownIsStale: chartsFrozen ? heldIsStale : isStale

    onChartsFrozenChanged: {
        if (chartsFrozen) {
            frozenHistory = MetricUtils.copySeries(history)
            heldWarningLevel = warningLevel
            heldIsStale = isStale
        } else {
            frozenHistory = []
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
