import QtQuick

Rectangle {
    id: root

    required property var controller

    color: "#101418"

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        Row {
            width: parent.width
            height: 48
            spacing: 16

            Column {
                width: parent.width - 180
                spacing: 3

                Text {
                    text: "Rotorboard"
                    color: "#f4f7f8"
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Live propulsion telemetry"
                    color: "#92a0a8"
                    font.pixelSize: 13
                }
            }

            Rectangle {
                width: 164
                height: 34
                radius: 5
                color: "#182028"
                border.color: "#2c3843"

                Text {
                    anchors.centerIn: parent
                    text: "Fake source"
                    color: "#c9d2d8"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }
            }
        }

        MotorGrid {
            width: parent.width
            height: parent.height - y
            model: root.controller.telemetryModel
        }
    }
}
