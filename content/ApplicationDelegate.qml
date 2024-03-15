import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    property var name
    property var thumbnail_path

    // signal downloadClicked()
    // signal uninstallClicked()

    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Column {
        id: app_info

        padding: 10
        spacing: 5
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
            wrapMode: Text.WrapAnywhere
            width: thumbnail.width
            height: font.pixelSize * 3
            font.pointSize: Qt.application.font.pointSize * 1.3
            color: app.globalPalette.text
        }

    }

    Button {
        id: action_button

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: app_info.width
        text: qsTr("Update")
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
