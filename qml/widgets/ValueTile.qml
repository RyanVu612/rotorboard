import QtQuick
import "../MetricUtils.js" as MetricUtils

Rectangle {
    id: root

    required property int motorId
    required property string metric
    required property real value
    required property string statusText
    required property bool isStale
    required property int warningLevel
    required property bool chartsFrozen

    property real heldValue: value
    property string heldStatus: statusText
    property int heldWarningLevel: warningLevel
    property bool heldIsStale: isStale

    readonly property int shownWarningLevel: chartsFrozen ? heldWarningLevel : warningLevel
    readonly property bool shownIsStale: chartsFrozen ? heldIsStale : isStale

    onChartsFrozenChanged: {
        if (chartsFrozen) {
            heldValue = value
            heldStatus = statusText
            heldWarningLevel = warningLevel
            heldIsStale = isStale
        }
    }

    clip: true
    radius: 6
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
        anchors.margins: 8
        spacing: 4

        Text {
            width: parent.width
            text: "M" + root.motorId + " " + MetricUtils.label(root.metric)
            color: "#77858e"
            font.pixelSize: 10
            font.capitalization: Font.AllUppercase
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.metric === "status"
                ? (root.shownIsStale ? "No recent data" : (root.chartsFrozen ? root.heldStatus : root.statusText))
                : MetricUtils.formatValue(root.metric, root.chartsFrozen ? root.heldValue : root.value)
            color: root.shownIsStale ? "#879199" : "#f4f7f8"
            font.pixelSize: 18
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }
}
