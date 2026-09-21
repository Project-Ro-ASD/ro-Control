import QtQuick

Canvas {
    id: sparkline
    property var values: []
    property color lineColor: "#818CF8"
    property color fillColor: Qt.rgba(lineColor.r, lineColor.g, lineColor.b, 0.15)
    property real maxValue: 100.0

    renderTarget: Canvas.Image
    renderStrategy: Canvas.Immediate
    visible: true

    onValuesChanged: if (visible && width > 0 && height > 0) Qt.callLater(requestPaint)
    onWidthChanged: if (visible && width > 0 && height > 0) Qt.callLater(requestPaint)
    onHeightChanged: if (visible && width > 0 && height > 0) Qt.callLater(requestPaint)
    onVisibleChanged: if (visible && width > 0 && height > 0) Qt.callLater(requestPaint)

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        var w = width;
        var h = height;
        if (w <= 0 || h <= 0 || !values || values.length < 2)
            return;

        ctx.clearRect(0, 0, w, h);

        var len = values.length;
        var step = w / (len - 1);

        ctx.beginPath();
        for (var i = 0; i < len; ++i) {
            var val = Math.max(0, Math.min(maxValue, values[i]));
            var x = i * step;
            var y = h - (val / maxValue) * (h - 4) - 2;
            if (i === 0) {
                ctx.moveTo(x, y);
            } else {
                ctx.lineTo(x, y);
            }
        }

        ctx.strokeStyle = lineColor;
        ctx.lineWidth = 1.8;
        ctx.stroke();

        ctx.lineTo(w, h);
        ctx.lineTo(0, h);
        ctx.closePath();
        ctx.fillStyle = fillColor;
        ctx.fill();
    }
}
