import QtQuick

Rectangle {
    id: root

    required property int warningLevel

    radius: 5
    color: {
        if (warningLevel === 3) return "#303842"
        if (warningLevel === 2) return "#4d211d"
        if (warningLevel === 1) return "#493316"
        return "#183525"
    }
    border.width: 1
    border.color: {
        if (warningLevel === 3) return "#687482"
        if (warningLevel === 2) return "#d05246"
        if (warningLevel === 1) return "#c58b2c"
        return "#3aa869"
    }

    Text {
        anchors.centerIn: parent
        text: {
            if (root.warningLevel === 3) return "STALE"
            if (root.warningLevel === 2) return "CRIT"
            if (root.warningLevel === 1) return "WARN"
            return "OK"
        }
        color: {
            if (root.warningLevel === 3) return "#d0d7dc"
            if (root.warningLevel === 2) return "#ffb8b2"
            if (root.warningLevel === 1) return "#ffd18a"
            return "#9ff0b8"
        }
        font.pixelSize: 11
        font.weight: Font.DemiBold
    }
}
