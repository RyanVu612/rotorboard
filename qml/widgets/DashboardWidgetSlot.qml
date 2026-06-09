import QtQuick

Item {
    id: slotRoot

    required property string widgetId
    required property string widgetType
    required property int motorId
    required property string metric
    required property int gridCol
    required property int gridRow
    required property int gridColSpan
    required property int gridRowSpan
    required property var grid
    required property bool editMode
    required property bool selected

    property bool dragging: false
    property real dragX: lockedX
    property real dragY: lockedY
    property real pressGridX: 0
    property real pressGridY: 0
    property real originDragX: 0
    property real originDragY: 0

    readonly property real lockedX: (gridCol - 1) * grid.cellWidth + grid.cardMargin
    readonly property real lockedY: (gridRow - 1) * grid.cellHeight + grid.cardMargin
    readonly property real lockedWidth: gridColSpan * grid.cellWidth - (grid.cardMargin * 2)
    readonly property real lockedHeight: gridRowSpan * grid.cellHeight - (grid.cardMargin * 2)

    width: lockedWidth
    height: lockedHeight
    z: dragging ? 200 : gridCol + gridRow
    x: dragging ? dragX : lockedX
    y: dragging ? dragY : lockedY

    default property alias content: contentItem.data

    Item {
        id: contentItem
        anchors.fill: parent
    }

    Behavior on x {
        enabled: !dragging
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    Behavior on y {
        enabled: !dragging
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    onLockedXChanged: { if (!dragging) dragX = lockedX }
    onLockedYChanged: { if (!dragging) dragY = lockedY }

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: "transparent"
        border.width: slotRoot.selected && slotRoot.editMode ? 2 : 0
        border.color: "#6eb5d8"
        visible: slotRoot.editMode
    }

  MouseArea {
        id: handleArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: slotRoot.editMode ? 24 : 0
        visible: slotRoot.editMode
        z: 2
        hoverEnabled: true
        preventStealing: true
        cursorShape: Qt.SizeAllCursor

        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 12
            height: 4
            radius: 2
            color: "#4a5862"
        }

        onPressed: function(mouse) {
            mouse.accepted = true
            grid.selectWidget(slotRoot.widgetId)
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
            const col = grid.snapColFromTopLeft(dragX, slotRoot.gridColSpan)
            const row = grid.snapRowFromTopLeft(dragY, slotRoot.gridRowSpan)
            dragging = false
            grid.placeWidget(slotRoot.widgetId, col, row)
            grid.endCardDrag()
        }

        onCanceled: {
            dragging = false
            dragX = lockedX
            dragY = lockedY
            grid.endCardDrag()
        }
    }

    MouseArea {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: slotRoot.editMode ? 16 : 0
        height: slotRoot.editMode ? 16 : 0
        visible: slotRoot.editMode
        z: 3
        cursorShape: Qt.SizeFDiagCursor

        onPressed: function(mouse) {
            mouse.accepted = true
            grid.selectWidget(slotRoot.widgetId)
            grid.beginResize(slotRoot.widgetId)
        }

        onPositionChanged: function(mouse) {
            if (!pressed)
                return
            const pos = mapToItem(grid, mouse.x, mouse.y)
            const col = grid.clamp(Math.floor(pos.x / grid.cellWidth) + 1, gridCol, grid.gridColumns)
            const row = grid.clamp(Math.floor(pos.y / grid.cellHeight) + 1, gridRow, grid.gridRows)
            grid.resizeWidget(slotRoot.widgetId, col - gridCol + 1, row - gridRow + 1)
        }

        onReleased: grid.endResize()
        onCanceled: grid.endResize()
    }

    MouseArea {
        anchors.fill: parent
        enabled: slotRoot.editMode
        z: 1
        onClicked: grid.selectWidget(slotRoot.widgetId)
    }
}
