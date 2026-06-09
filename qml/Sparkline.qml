import QtQuick

Canvas {
    id: root

    property var values: []
    property color strokeColor: "#5a8fae"
    property bool dimmed: false

    height: 32
    width: parent ? parent.width : 100

    onValuesChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onDimmedChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        if (!values || values.length < 2 || width <= 0 || height <= 0) {
            return
        }

        let minValue = values[0]
        let maxValue = values[0]
        for (let i = 1; i < values.length; ++i) {
            const value = values[i]
            if (value < minValue) {
                minValue = value
            }
            if (value > maxValue) {
                maxValue = value
            }
        }

        const range = maxValue - minValue
        const padding = 2
        const plotHeight = height - (padding * 2)
        const plotWidth = width - (padding * 2)
        const xStep = plotWidth / (values.length - 1)

        ctx.beginPath()
        ctx.lineWidth = 1.5
        ctx.strokeStyle = dimmed ? "#879199" : strokeColor

        for (let i = 0; i < values.length; ++i) {
            const normalized = range === 0 ? 0.5 : (values[i] - minValue) / range
            const x = padding + (i * xStep)
            const y = padding + plotHeight - (normalized * plotHeight)

            if (i === 0) {
                ctx.moveTo(x, y)
            } else {
                ctx.lineTo(x, y)
            }
        }

        ctx.stroke()
    }
}
