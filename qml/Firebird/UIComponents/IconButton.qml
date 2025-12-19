import QtQuick
import QtQuick.Controls

// Small helper button that shows only an icon.
// Important on macOS: native QuickControls style does not support overriding
// contentItem/background. So we use the built-in icon API instead of inserting
// custom children.
Button {
    id: root

    // Backwards-compatible external API for our project.
    // ("source" is used instead of "icon" to avoid colliding with Button.icon.)
    property url source: ""

    display: AbstractButton.IconOnly
    icon.source: source
    icon.width: 24
    icon.height: 24

    padding: 4
    flat: true
}
