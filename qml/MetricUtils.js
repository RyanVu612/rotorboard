.pragma library

function label(metric) {
    switch (metric) {
    case "rpm": return "RPM"
    case "voltage": return "Voltage"
    case "current": return "Current"
    case "temperatureCelsius": return "Temp"
    case "pwm": return "PWM"
    case "status": return "Status"
    default: return metric
    }
}

function formatValue(metric, value) {
    if (metric === "status")
        return value ? value.toString() : ""
    if (metric === "rpm")
        return Math.round(value).toString() + " rpm"
    if (metric === "voltage")
        return Number(value).toFixed(1) + " V"
    if (metric === "current")
        return Number(value).toFixed(1) + " A"
    if (metric === "temperatureCelsius")
        return Number(value).toFixed(1) + " C"
    if (metric === "pwm")
        return Math.round(value).toString()
    return value ? value.toString() : ""
}

function strokeColor(metric) {
    switch (metric) {
    case "rpm": return "#6eb5d8"
    case "voltage": return "#7a9eb8"
    case "current": return "#d4a05c"
    case "temperatureCelsius": return "#c97d6f"
    case "pwm": return "#7a9eb8"
    default: return "#5a8fae"
    }
}

function hasHistory(metric) {
    return metric !== "status"
}

function copySeries(series) {
    if (!series || series.length === 0)
        return []
    var copy = []
    for (var i = 0; i < series.length; ++i)
        copy.push(series[i])
    return copy
}
