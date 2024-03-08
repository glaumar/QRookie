import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    property var name
    property var size
    property var last_updated
    property var thumbnail_path
    // property bool downloaded
    // property bool installed
    // property bool decompressed

    signal downloadClicked

    radius: 5
    border.color: "grey"
    border.width: 2
    layer.enabled: true

    Column {
        id: game_info

        padding: 10
        width: parent.width - 20

        Image {
            id: thumbnail

            asynchronous: true
            width: parent.width
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
            text: name
            font.bold: true
            wrapMode: Text.WordWrap
            width: thumbnail.width
            height: font.pixelSize * 3
        }

        Text {
            anchors.right: parent.right
            text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
            font.pointSize: Qt.application.font.pointSize * 0.9
        }

        Text {
            anchors.right: parent.right
            text: last_updated
            font.pointSize: Qt.application.font.pointSize * 0.8
        }

        Button {
            id: download

            width: parent.width
            text: qsTr("Downlad")
            onClicked: {
                downloadClicked();
            }
        }

    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: "lightgrey"
    }

}
