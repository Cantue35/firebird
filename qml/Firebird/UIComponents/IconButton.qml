import QtQuick
import QtQuick.Controls
/* A push button with a symbol instead of text.
 * ToolButton and <img/> in Label don't size correctly,
 * so do it manually.
 * With QQC2, button icons have a better default size
 * and it can also be specified explicitly. */

Button {
    id: control
    // Qt 6: Button already has an `icon` property-group and it is FINAL.
    // Do not redeclare/alias it (that breaks component instantiation).
    // Callers should set: `icon.source: "..."`.

    implicitHeight: TextMetrics.normalSize * 2.5
    implicitWidth: implicitHeight

    contentItem: Image {
        id: image
        source: control.icon.source
        height: Math.round(parent.height * 0.6)
        width: height
        anchors.centerIn: parent

        fillMode: Image.PreserveAspectFit
        mipmap: true
    }
}
