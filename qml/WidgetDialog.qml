import QtQuick
import QtQuick.Controls

Popup {
    id: root

    // "add" opens ghost placement on confirm; "edit" applies to an existing widget.
    property string mode: "add"
    property string widgetId: ""

    readonly property var widgetTypes: [
        {label: "Value tile", type: "value"},
        {label: "Chart", type: "chart"},
        {label: "Motor summary", type: "motorSummary"}
    ]
    readonly property var metrics: ["rpm", "voltage", "current", "temperatureCelsius", "pwm"]

    signal confirmed(string mode, string widgetId, string widgetType, int motorId, string metric)

    function openForAdd() {
        mode = "add"
        widgetId = ""
        typeCombo.currentIndex = 0
        motorSpinBox.value = 1
        metricCombo.currentIndex = 0
        open()
    }

    function openForEdit(editWidgetId, widgetType, motorId, metric) {
        mode = "edit"
        widgetId = editWidgetId
        let typeIndex = 0
        for (let i = 0; i < widgetTypes.length; ++i) {
            if (widgetTypes[i].type === widgetType)
                typeIndex = i
        }
        typeCombo.currentIndex = typeIndex
        motorSpinBox.value = motorId
        let metricIndex = 0
        for (let j = 0; j < metrics.length; ++j) {
            if (metrics[j] === metric)
                metricIndex = j
        }
        metricCombo.currentIndex = metricIndex
        open()
    }

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 16

    background: Rectangle {
        radius: 8
        color: "#182028"
        border.color: "#2c3843"
        border.width: 1
    }

    contentItem: Column {
        spacing: 10
        width: 240

        Text {
            text: root.mode === "edit" ? "Edit widget" : "Add widget"
            color: "#f4f7f8"
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        Text {
            text: "Type"
            color: "#92a0a8"
            font.pixelSize: 11
        }

        ComboBox {
            id: typeCombo
            width: parent.width
            model: root.widgetTypes
            textRole: "label"
        }

        Text {
            text: "Motor"
            color: "#92a0a8"
            font.pixelSize: 11
        }

        SpinBox {
            id: motorSpinBox
            from: 1
            to: 32
        }

        Text {
            text: "Metric"
            color: "#92a0a8"
            font.pixelSize: 11
            visible: metricCombo.visible
        }

        ComboBox {
            id: metricCombo
            width: parent.width
            model: root.metrics
            visible: typeCombo.currentIndex >= 0
                     && root.widgetTypes[typeCombo.currentIndex].type !== "motorSummary"
        }

        Row {
            spacing: 8
            anchors.right: parent.right

            Rectangle {
                width: cancelLabel.implicitWidth + 20
                height: 30
                radius: 5
                color: "#1f2830"
                border.color: "#3d4d59"

                Text {
                    id: cancelLabel
                    anchors.centerIn: parent
                    text: "Cancel"
                    color: "#c9d2d8"
                    font.pixelSize: 12
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
            }

            Rectangle {
                width: confirmLabel.implicitWidth + 20
                height: 30
                radius: 5
                color: "#2a3640"
                border.color: "#6eb5d8"

                Text {
                    id: confirmLabel
                    anchors.centerIn: parent
                    text: root.mode === "edit" ? "Apply" : "Add"
                    color: "#e2e8ec"
                    font.pixelSize: 12
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        const widgetType = root.widgetTypes[typeCombo.currentIndex].type
                        const metric = widgetType === "motorSummary" ? "" : metricCombo.currentText
                        root.confirmed(root.mode, root.widgetId, widgetType,
                                       motorSpinBox.value, metric)
                        root.close()
                    }
                }
            }
        }
    }
}
