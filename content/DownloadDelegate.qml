import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    property var name
    property var size
    property var last_updated
    property var thumbnail_path


    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Row {
        id: game_info

        padding: 10
        spacing: 5
        height: parent.height - 20

        Image {
            id: thumbnail

            asynchronous: true
            height: parent.height
            source: thumbnail_path
            fillMode: Image.PreserveAspectFit
            layer.enabled: true

            layer.effect: OpacityMask {

                maskSource: Rectangle {
                    width: thumbnail.width
                    height: thumbnail.height
                    radius: 5
                }

            }

        }

        // Text {
        //     text: name
        //     font.bold: true
        //     wrapMode: Text.WordWrap
        //     width: thumbnail.width
        //     height: font.pixelSize * 3
        //     font.pointSize: Qt.application.font.pointSize * 1.3
        //     color: app.globalPalette.text
        // }

        // Text {
        //     anchors.right: parent.right
        //     text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
        //     font.pointSize: Qt.application.font.pointSize * 0.9
        //     color: app.globalPalette.text
        // }

        // Text {
        //     anchors.right: parent.right
        //     text: last_updated
        //     font.pointSize: Qt.application.font.pointSize * 0.8
        //     color: app.globalPalette.text
        // }

    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
