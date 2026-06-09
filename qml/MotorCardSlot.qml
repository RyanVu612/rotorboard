import QtQuick

Item {
    id: slotRoot

    required property int motorId
    required property int gridCol
    required property int gridRow
    required property var grid
    required property int layoutMode

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

    readonly property real lockedX: (gridCol - 1) * grid.cellWidth + grid.cardMargin
    readonly property real lockedY: (gridRow - 1) * grid.cellHeight + grid.cardMargin

    property bool dragging: false
    property real dragX: lockedX
    property real dragY: lockedY
    property real pressGridX: 0
    property real pressGridY: 0
    property real originDragX: 0
    property real originDragY: 0

    width: grid.cellWidth - (grid.cardMargin * 2)
    height: grid.cellHeight - (grid.cardMargin * 2)
    z: dragging ? 100 : gridCol + gridRow
    x: dragging ? dragX : lockedX
    y: dragging ? dragY : lockedY

    Behavior on x {
        enabled: !dragging
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    Behavior on y {
        enabled: !dragging
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    onLockedXChanged: {
        if (!dragging)
            dragX = lockedX
    }

    onLockedYChanged: {
        if (!dragging)
            dragY = lockedY
    }

    MotorCard {
        anchors.fill: parent
        layoutMode: slotRoot.layoutMode
        motorId: slotRoot.motorId
        rpm: slotRoot.rpm
        voltage: slotRoot.voltage
        current: slotRoot.current
        temperatureCelsius: slotRoot.temperatureCelsius
        pwm: slotRoot.pwm
        status: slotRoot.status
        isStale: slotRoot.isStale
        warningLevel: slotRoot.warningLevel
        rpmHistory: slotRoot.rpmHistory
        currentHistory: slotRoot.currentHistory
        temperatureHistory: slotRoot.temperatureHistory
    }

    MouseArea {
        id: handleArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 28
        z: 1
        hoverEnabled: true
        preventStealing: true
        cursorShape: Qt.SizeAllCursor

        onPressed: function(mouse) {
            mouse.accepted = true
            grid.beginCardDrag()
            dragging = true
            originDragX = lockedX
            originDragY = lockedY
            dragX = lockedX
            dragY = lockedY
            const pos = handleArea.mapToItem(grid, mouse.x, mouse.y)
            pressGridX = pos.x
            pressGridY = pos.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed || !mouse)
                return

            const pos = handleArea.mapToItem(grid, mouse.x, mouse.y)
            dragX = originDragX + pos.x - pressGridX
            dragY = originDragY + pos.y - pressGridY
        }

        onReleased: function(mouse) {
            const centerX = dragX + slotRoot.width / 2
            const centerY = dragY + slotRoot.height / 2
            const col = grid.clamp(Math.floor(centerX / grid.cellWidth) + 1, 1, grid.gridColumns)
            const row = grid.clamp(Math.floor(centerY / grid.cellHeight) + 1, 1, grid.gridRows)
            dragging = false
            grid.placeMotor(motorId, col, row)
            grid.endCardDrag()
        }

        onCanceled: {
            dragging = false
            dragX = lockedX
            dragY = lockedY
            grid.endCardDrag()
        }
    }
}
