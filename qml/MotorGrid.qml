import QtQuick

GridView {
    id: root

    property int layoutMode: 1

    readonly property int effectiveMinCardWidth: layoutMode === 0 ? 200 : 250
    readonly property int effectiveCellHeight: layoutMode === 0 ? 220 : 310
    readonly property int columns: Math.max(1, Math.floor(width / effectiveMinCardWidth))

    clip: true
    cellWidth: Math.floor(width / columns)
    cellHeight: effectiveCellHeight
    boundsBehavior: Flickable.StopAtBounds

    delegate: MotorCard {
        required property var model

        width: root.cellWidth - 12
        height: root.cellHeight - 12
        layoutMode: root.layoutMode
        motorId: model.motorId
        rpm: model.rpm
        voltage: model.voltage
        current: model.current
        temperatureCelsius: model.temperatureCelsius
        pwm: model.pwm
        status: model.status
        isStale: model.isStale
        warningLevel: model.warningLevel
        rpmHistory: model.rpmHistory
        currentHistory: model.currentHistory
        temperatureHistory: model.temperatureHistory
    }

    Text {
        anchors.centerIn: parent
        visible: root.count === 0
        text: "Waiting for telemetry"
        color: "#7e8b93"
        font.pixelSize: 16
    }
}
