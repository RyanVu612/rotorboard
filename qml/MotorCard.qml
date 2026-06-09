import QtQuick

Rectangle {
    id: root

    required property int motorId
    required property real rpm
    required property real voltage
    required property real current
    required property real temperatureCelsius
    required property real pwm
    required property string status
    required property bool isStale
    required property int warningLevel
    required property var rpmHistory
    required property var currentHistory
    required property var temperatureHistory
    required property int layoutMode

    readonly property bool compact: layoutMode === 0

    radius: 8
    color: warningLevel === 3 ? "#191d21" : "#161c22"
    border.width: 1
    border.color: {
        if (warningLevel === 3) return "#6b7480"
        if (warningLevel === 2) return "#d05246"
        if (warningLevel === 1) return "#c58b2c"
        return "#2d3a43"
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: root.compact ? 8 : 13

        Row {
            width: parent.width
            height: 28

            Text {
                width: parent.width - 86
                text: "Motor " + root.motorId
                color: "#f3f6f7"
                font.pixelSize: 20
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            StatusBadge {
                width: 86
                height: 26
                warningLevel: root.warningLevel
            }
        }

        Text {
            width: parent.width
            text: Math.round(root.rpm).toLocaleString(Qt.locale()) + " rpm"
            color: root.isStale ? "#879199" : "#f4f7f8"
            font.pixelSize: root.compact ? 26 : 32
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Sparkline {
            width: parent.width
            visible: !root.compact
            values: root.rpmHistory
            strokeColor: "#6eb5d8"
            dimmed: root.isStale
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#26323b"
        }

        Grid {
            width: parent.width
            columns: 2
            rowSpacing: root.compact ? 8 : 12
            columnSpacing: 12

            Repeater {
                model: [
                    {label: "Voltage", value: root.voltage.toFixed(1) + " V", history: null, color: "#7a9eb8"},
                    {label: "Current", value: root.current.toFixed(1) + " A", history: root.currentHistory, color: "#d4a05c"},
                    {label: "Temp", value: root.temperatureCelsius.toFixed(1) + " C", history: root.temperatureHistory, color: "#c97d6f"},
                    {label: "PWM", value: Math.round(root.pwm).toString(), history: null, color: "#7a9eb8"}
                ]

                Column {
                    width: (parent.width - parent.columnSpacing) / 2
                    spacing: 3

                    Text {
                        width: parent.width
                        text: modelData.label
                        color: "#77858e"
                        font.pixelSize: 11
                        font.capitalization: Font.AllUppercase
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: modelData.value
                        color: "#d6dde1"
                        font.pixelSize: root.compact ? 16 : 18
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }

                    Sparkline {
                        width: parent.width
                        height: 24
                        visible: !root.compact && modelData.history !== null
                        values: modelData.history || []
                        strokeColor: modelData.color
                        dimmed: root.isStale
                    }
                }
            }
        }

        Text {
            width: parent.width
            text: root.isStale ? "No recent data" : root.status
            color: root.isStale ? "#a9b2b8" : "#7ed39a"
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
}
