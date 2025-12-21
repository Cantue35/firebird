
import QtQuick
import QtQuick.Controls

import QtQuick.Layouts
import Firebird.UIComponents 1.0

ToolButton {
    property alias icon: image.source
    property alias title: label.text

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(palette.highlight.r, palette.highlight.g, palette.highlight.b, 0.22)
        visible: parent.pressed
    }

    FBLabel {
        id: label
        x: 12
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        horizontalAlignment: Text.AlignHCenter
        fontSizeMode: Text.HorizontalFit
        font.pixelSize: UiMetrics.normalSize
        width: parent.width
    }

    Image {
        id: image
        anchors {
            top: label.bottom
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }

        fillMode: Image.PreserveAspectFit
    }
}
