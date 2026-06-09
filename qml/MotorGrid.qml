import QtQuick
import QtCore

Flickable {
    id: root

    property var model
    property int layoutMode: 1

    readonly property int gridColumns: 4
    readonly property int gridRows: 4
    readonly property int cellWidth: Math.max(1, Math.floor(width / gridColumns))
    readonly property int cellHeight: layoutMode === 0 ? 200 : 280
    readonly property int cardMargin: 6

    property var motorPositions: ({})
    property var activeMotorIds: []
    property int dragLockCount: 0

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    interactive: false
    contentWidth: gridColumns * cellWidth
    contentHeight: gridRows * cellHeight

    Settings {
        id: gridSettings
        category: "MotorGrid"
        property string positionsJson: "{}"
    }

    function clamp(value, min, max) {
        return Math.max(min, Math.min(max, value))
    }

    function isValidCell(col, row) {
        return col >= 1 && col <= gridColumns && row >= 1 && row <= gridRows
    }

    function clonePositions(map) {
        return JSON.parse(JSON.stringify(map || {}))
    }

    function motorAtCellFromMap(map, col, row) {
        for (var key in map) {
            var pos = map[key]
            if (pos && pos.col === col && pos.row === row)
                return parseInt(key)
        }
        return null
    }

    function motorAtCell(col, row) {
        return motorAtCellFromMap(motorPositions, col, row)
    }

    function firstFreeCellFromMap(map) {
        for (var row = 1; row <= gridRows; row++) {
            for (var col = 1; col <= gridColumns; col++) {
                if (motorAtCellFromMap(map, col, row) === null)
                    return {col: col, row: row}
            }
        }
        return null
    }

    function firstFreeCell() {
        return firstFreeCellFromMap(motorPositions)
    }

    function positionForMotor(motorId) {
        var key = String(motorId)
        var pos = motorPositions[key]
        if (pos && isValidCell(pos.col, pos.row))
            return pos
        return {col: 1, row: 1}
    }

    function persistPositions() {
        gridSettings.positionsJson = JSON.stringify(motorPositions)
    }

    function loadPositions() {
        if (!gridSettings.positionsJson || gridSettings.positionsJson === "{}") {
            motorPositions = {}
            return
        }

        try {
            var parsed = JSON.parse(gridSettings.positionsJson)
            var cleaned = {}
            for (var key in parsed) {
                var pos = parsed[key]
                if (pos && isValidCell(pos.col, pos.row))
                    cleaned[key] = {col: pos.col, row: pos.row}
            }
            motorPositions = cleaned
        } catch (e) {
            motorPositions = {}
        }
    }

    function setMotorPosition(motorId, col, row) {
        var key = String(motorId)
        var next = clonePositions(motorPositions)
        next[key] = {col: col, row: row}
        motorPositions = next
        persistPositions()
    }

    function placeMotor(motorId, col, row) {
        col = clamp(col, 1, gridColumns)
        row = clamp(row, 1, gridRows)

        var occupant = motorAtCell(col, row)
        if (occupant === null || occupant === motorId) {
            setMotorPosition(motorId, col, row)
            return
        }

        var draggedOld = positionForMotor(motorId)
        var next = clonePositions(motorPositions)
        next[String(motorId)] = {col: col, row: row}
        next[String(occupant)] = {col: draggedOld.col, row: draggedOld.row}
        motorPositions = next
        persistPositions()
    }

    function beginCardDrag() {
        dragLockCount++
    }

    function endCardDrag() {
        dragLockCount = Math.max(0, dragLockCount - 1)
    }

    function registerMotor(motorId) {
        var ids = activeMotorIds.slice()
        if (ids.indexOf(motorId) < 0) {
            ids.push(motorId)
            activeMotorIds = ids
        }
        Qt.callLater(reconcilePlacements)
    }

    function reconcilePlacements() {
        var next = clonePositions(motorPositions)
        var changed = false

        for (var i = 0; i < activeMotorIds.length; i++) {
            var motorId = activeMotorIds[i]
            var key = String(motorId)
            var pos = next[key]
            if (!pos || !isValidCell(pos.col, pos.row)) {
                var free = firstFreeCellFromMap(next)
                if (free) {
                    next[key] = free
                    changed = true
                }
            }
        }

        if (changed) {
            motorPositions = next
            persistPositions()
        }
    }

    Component.onCompleted: loadPositions()

    Item {
        width: root.contentWidth
        height: root.contentHeight

        Repeater {
            model: root.gridColumns * root.gridRows

            Rectangle {
                required property int index

                readonly property int cellCol: (index % root.gridColumns) + 1
                readonly property int cellRow: Math.floor(index / root.gridColumns) + 1

                x: (cellCol - 1) * root.cellWidth
                y: (cellRow - 1) * root.cellHeight
                width: root.cellWidth
                height: root.cellHeight
                color: "transparent"
                border.color: "#1a232b"
                border.width: 1
            }
        }

        Repeater {
            model: root.model

            MotorCardSlot {
                required property var model

                grid: root
                layoutMode: root.layoutMode
                motorId: model.motorId
                gridCol: root.positionForMotor(model.motorId).col
                gridRow: root.positionForMotor(model.motorId).row
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

                Component.onCompleted: root.registerMotor(model.motorId)
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: !model || model.count === 0
        text: "Waiting for telemetry"
        color: "#7e8b93"
        font.pixelSize: 16
    }
}
