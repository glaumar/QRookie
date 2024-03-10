import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: game_info

    property var name
    property var size
    property var thumbnail_path
    property var progress

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
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2
        font.pointSize: Qt.application.font.pointSize * 1.3
        color: app.globalPalette.text
    }

    ProgressBar {
        id: progress_bar

        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.bottom: parent.bottom
        value: progress
        height: 10
        layer.enabled: true

        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 6
            verticalOffset: 6
            color: app.globalPalette.shadow
        }

    }

    Text {
        anchors.margins: 10
        anchors.right: progress_bar.right
        anchors.bottom: progress_bar.top
        text: {
            if (isNaN(progress) || progress <= 1e-36) {
                return qsTr("Queued");
            } else {
                let downloaded = progress * size;
                let downloaded_unit = downloaded < 1024? "MB": "GB";
                downloaded = downloaded < 1024? downloaded: downloaded / 1024;
                let total_size = size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
                return downloaded.toFixed(2) + " " + downloaded_unit + " / " + total_size;
            }
        }
        color: app.globalPalette.text
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
