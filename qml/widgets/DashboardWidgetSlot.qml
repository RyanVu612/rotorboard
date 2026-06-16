import QtQuick

Item {
    id: slotRoot
    objectName: "widgetSlot-" + widgetId

    required property string widgetId
    required property string widgetType
    required property int motorId
    required property string metric
    required property int gridCol
    required property int gridRow
    required property int gridColSpan
    required property int gridRowSpan
    required property var grid

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
    readonly property bool resizing: grid.resizingWidgetId === slotRoot.widgetId

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

    HoverHandler {
        id: hoverHandler
    }

    MouseArea {
        id: bodyArea
        objectName: "widgetSlotBodyArea"
        anchors.fill: parent
        z: 2
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        preventStealing: true

        onPressed: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                mouse.accepted = true
                return
            }
            mouse.accepted = true
            grid.beginCardDrag()
            dragging = true
            originDragX = lockedX
            originDragY = lockedY
            dragX = lockedX
            dragY = lockedY
            const pos = bodyArea.mapToItem(grid, mouse.x, mouse.y)
            pressGridX = pos.x
            pressGridY = pos.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed || !dragging || !mouse)
                return
            const pos = bodyArea.mapToItem(grid, mouse.x, mouse.y)
            dragX = originDragX + pos.x - pressGridX
            dragY = originDragY + pos.y - pressGridY
        }

        onReleased: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                if (!dragging && !slotRoot.resizing)
                    grid.openWidgetMenu(slotRoot.widgetId,
                                        slotRoot.widgetType,
                                        slotRoot.motorId,
                                        slotRoot.metric,
                                        bodyArea.mapToItem(grid, mouse.x, mouse.y))
                return
            }
            if (!dragging)
                return
            const col = grid.snapColFromTopLeft(dragX, slotRoot.gridColSpan)
            const row = grid.snapRowFromTopLeft(dragY, slotRoot.gridRowSpan)
            dragging = false
            grid.placeWidget(slotRoot.widgetId, col, row)
            grid.endCardDrag()
        }

        onCanceled: {
            if (!dragging)
                return
            dragging = false
            dragX = lockedX
            dragY = lockedY
            grid.endCardDrag()
        }
    }

    MouseArea {
        id: resizeArea
        objectName: "widgetSlotResizeArea"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 16
        height: 16
        z: 3
        cursorShape: Qt.SizeFDiagCursor

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 2
            color: "#4a5862"
            visible: hoverHandler.hovered || slotRoot.resizing
        }

        onPressed: function(mouse) {
            mouse.accepted = true
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
}
