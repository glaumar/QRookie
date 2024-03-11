import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Rectangle {
    id: game_info

    property var name
    property var size
    property var thumbnail_path
    property var status

    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Image {
        id: thumbnail

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
        height: parent.height - 20
        asynchronous: true
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

    Text {
        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.top: parent.top
        text: name
        width: parent.width - thumbnail.width - 30
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2
        font.pointSize: Qt.application.font.pointSize * 1.3
        color: app.globalPalette.text
    }



    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }
}
