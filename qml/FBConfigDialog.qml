
import QtQuick
import QtQuick.Window
import QtQuick.Controls

import QtQuick.Layouts
import Firebird.UIComponents 1.0

Window {
    id: window
    title: qsTr("Firebird Emu Configuration")
    minimumHeight: 400
    minimumWidth: 500
    height: 420
    width: 540

    SystemPalette {
        id: paletteActive
    }

    color: paletteActive.window

    ConfigPages {
        anchors {
            bottom: actionRow.top
            right: parent.right
            left: parent.left
            top: parent.top
            margins: 5
        }
        model: ConfigPagesModel {
        }
    }

    RowLayout {
        id: actionRow

        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
            margins: 5
            topMargin: 0
        }

        FBLabel {
            Layout.fillWidth: true
            text: qsTr("Changes are saved automatically")
            font.italic: true
            color: Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.6)
        }

        Button {
            text: qsTr("Ok")
            onClicked: window.visible = false
        }
    }
}
