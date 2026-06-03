import QtQuick

Window {
    id: root

    width: 1120
    height: 720
    minimumWidth: 760
    minimumHeight: 520
    visible: true
    title: "Rotorboard"
    color: "#101418"

    Component.onCompleted: appController.start()
    onClosing: appController.stop()

    DashboardPage {
        anchors.fill: parent
        controller: appController
    }
}
