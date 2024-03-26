import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Rectangle {
    property var name
    property var thumbnailPath
    property var gameInfo
    property var releaseName
    property var status: VrpDownloader.Unknown

    onStatusChanged: function() {
        action_button.enabled = false;
        action_button.icon.source = "install";
        switch (status) {
        case VrpDownloader.UpdatableRemotely:
            action_button.text = qsTr("Download and Update");
            action_button.enabled = true;
            break;
        case VrpDownloader.UpdatableLocally:
            action_button.text = qsTr("Update");
            action_button.enabled = true;
            break;
        // case VrpDownloader.Error:
        //     action_button.text = qsTr("Error, Click to Try Again");
        //     action_button.enabled = true;
        //     action_button.icon.source = "error";
        //     break;
        case VrpDownloader.Queued:
            action_button.text = qsTr("Queued...");
            break;
        case VrpDownloader.Downloading:
            action_button.text = qsTr("Downloading...");
            break;
        case VrpDownloader.Decompressing:
            action_button.text = qsTr("Decompressing...");
            break;
        case VrpDownloader.Installing:
            action_button.text = qsTr("Installing...");
            break;
        default:
            action_button.text = qsTr("Installed");
            break;
        }
    }
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
            source: thumbnailPath
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
        text: qsTr("Installed")
        onClicked: {
            if (status === VrpDownloader.UpdatableRemotely || status === VrpDownloader.Error)
                app.vrp.addToDownloadQueue(gameInfo);
            else if (status === VrpDownloader.UpdatableLocally)
                app.vrp.installQml(gameInfo);
        }
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
