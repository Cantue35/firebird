
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

    // Read the real system palette for each color group.
    SystemPalette { id: sysActive;   colorGroup: SystemPalette.Active }
    SystemPalette { id: sysInactive; colorGroup: SystemPalette.Inactive }
    SystemPalette { id: sysDisabled; colorGroup: SystemPalette.Disabled }

    // Force this window + all child controls to use the same palette as the OS.
    // Window.palette propagates to child items/controls and overrides defaults.
    palette {
        active.window: sysActive.window
        active.windowText: sysActive.windowText
        active.base: sysActive.base
        active.alternateBase: sysActive.alternateBase
        active.text: sysActive.text
        active.button: sysActive.button
        active.buttonText: sysActive.buttonText
        active.highlight: sysActive.highlight
        active.highlightedText: sysActive.highlightedText

        inactive.window: sysInactive.window
        inactive.windowText: sysInactive.windowText
        inactive.base: sysInactive.base
        inactive.alternateBase: sysInactive.alternateBase
        inactive.text: sysInactive.text
        inactive.button: sysInactive.button
        inactive.buttonText: sysInactive.buttonText
        inactive.highlight: sysInactive.highlight
        inactive.highlightedText: sysInactive.highlightedText

        disabled.window: sysDisabled.window
        disabled.windowText: sysDisabled.windowText
        disabled.base: sysDisabled.base
        disabled.alternateBase: sysDisabled.alternateBase
        disabled.text: sysDisabled.text
        disabled.button: sysDisabled.button
        disabled.buttonText: sysDisabled.buttonText
        disabled.highlight: sysDisabled.highlight
        disabled.highlightedText: sysDisabled.highlightedText
    }

    // Use the same palette for the window background.
    color: active ? palette.active.window : palette.inactive.window  //

    ConfigPages {
        anchors {
            bottom: actionRow.top
            right: parent.right
            left: parent.left
            top: parent.top
            margins: 5
        }
        model: ConfigPagesModel { }
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
            // Tie to the window palette so it stays readable in dark mode.
            color: Qt.rgba(
                (window.active ? window.palette.active.windowText : window.palette.inactive.windowText).r,
                (window.active ? window.palette.active.windowText : window.palette.inactive.windowText).g,
                (window.active ? window.palette.active.windowText : window.palette.inactive.windowText).b,
                0.6
            )
        }

        Button {
            text: qsTr("Ok")
            onClicked: window.visible = false
        }
    }
}
