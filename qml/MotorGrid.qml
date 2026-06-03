import QtQuick

GridView {
    id: root

    property int minimumCardWidth: 250

    clip: true
    cellWidth: Math.max(minimumCardWidth, Math.floor(width / Math.max(1, Math.floor(width / minimumCardWidth))))
    cellHeight: 254
    boundsBehavior: Flickable.StopAtBounds

    delegate: MotorCard {
        width: root.cellWidth - 12
        height: root.cellHeight - 12
        motorId: model.motorId
        rpm: model.rpm
        voltage: model.voltage
        current: model.current
        temperatureCelsius: model.temperatureCelsius
        pwm: model.pwm
        status: model.status
        isStale: model.isStale
        warningLevel: model.warningLevel
    }

    Text {
        anchors.centerIn: parent
        visible: root.count === 0
        text: "Waiting for telemetry"
        color: "#7e8b93"
        font.pixelSize: 16
    }
}
