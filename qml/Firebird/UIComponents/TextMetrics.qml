pragma Singleton
import QtQuick
Item {
    // In Qt 6, font.pixelSize may be -1 (meaning "use pointSize") and during
    // early startup it can also momentarily evaluate to undefined in bindings.
    // Keep these properties as plain ints to avoid "Unable to assign [undefined] to int".
    property int baseSize: {
        var ps = defaultFont.font.pixelSize
        if (ps === undefined || ps < 1) ps = Qt.application.font.pixelSize
        if (ps === undefined || ps < 1) ps = 14
        return Math.round(ps)
    }

    property int normalSize: baseSize
    property int title1Size: Math.round(baseSize * 1.2)
    property int title2Size: Math.round(baseSize * 1.4)

    Text {
        id: defaultFont
    }
}
