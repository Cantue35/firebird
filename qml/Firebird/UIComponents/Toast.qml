
import QtQuick

import Firebird.UIComponents 1.0

Rectangle {
    SystemPalette { id: pal; colorGroup: SystemPalette.Active }
    property int maxWidth: parent.width * 0.9
    height: message.contentHeight + 8
    width: message.contentWidth + 10

    color: Qt.rgba(pal.dark.r, pal.dark.g, pal.dark.b, 0.85)
    border.color: pal.mid
    border.width: 0

    opacity: 0
    visible: opacity > 0

    Behavior on opacity { NumberAnimation { duration: 200 } }

    function showMessage(str) {
        message.text = str;
        opacity = 1;
        timer.restart();
    }

    FBLabel {
        id: message
        text: "Text"
        color: pal.light
        width: parent.maxWidth

        anchors.centerIn: parent

        horizontalAlignment: Text.Center
        font.pixelSize: UiMetrics.title1Size
        wrapMode: Text.WordWrap

        Timer {
            id: timer
            interval: 2000
            onTriggered: parent.parent.opacity = 0;
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            timer.stop();
            parent.opacity = 0;
        }
    }
}
