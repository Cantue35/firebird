
import QtQuick

import QtQuick.Layouts

import QtQuick.Controls

import Firebird.UIComponents 1.0

Item {
    Layout.fillHeight: true
    Layout.fillWidth: true

    Item {
    id: tabView
    anchors.fill: parent
    anchors.rightMargin: swipeBar.implicitWidth
    anchors.bottomMargin: autoSaveLabel.implicitHeight
    clip: true

    property var model: ConfigPagesModel {}

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TabBar {
            id: tabBar
            Layout.fillWidth: true

            Repeater {
                id: tabButtons
                model: tabView.model
                delegate: TabButton {
                    text: qsTranslate("ConfigPagesModel", tabButtons.model.get(index).title)
                }
            }
        }

        StackLayout {
            id: tabStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            Repeater {
                id: tabPages
                model: tabView.model
                delegate: Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Loader {
                        anchors.fill: parent
                        source: tabPages.model.get(index).file
                    }
                }
            }
        }
    }
}


    FBLabel {
        id: autoSaveLabel

        anchors {
            left: parent.left
            leftMargin: 2
            right: swipeBar.left
            bottom: parent.bottom
        }

        text: qsTr("Changes are saved automatically")
        font.italic: true
        color: "grey"
    }

    VerticalSwipeBar {
        id: swipeBar

        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }

        onClicked: {
            listView.openDrawer();
        }
    }
}
