import QtQuick 6.2

Rectangle {
    property var name
    property var size
    property var last_updated
    property var thumbnail_path

    Column {
        spacing: 10
        padding: 30
        anchors.fill: parent

        Image {
            id: thumbnail
            source: thumbnail_path
            fillMode: Image.PreserveAspectFit
        }

        Column {
            spacing: 5

            Text {
                text: name
                font.bold: true
                wrapMode: Text.WordWrap
                width: thumbnail.width
            }

            Text {
                text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
                font.pointSize: Qt.application.font.pointSize * 0.8
            }

            Text {
                text: last_updated
                font.pointSize: Qt.application.font.pointSize * 0.6
            }

        }

    }

}
