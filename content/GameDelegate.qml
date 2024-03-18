import QCoro
import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Rectangle {
    property var name
    // property var releaseName
    property var size
    property var lastUpdated
    property var thumbnailPath
    // property var versionCode
    // property var packageName
    property var progress
    property var status

    onProgressChanged: {
        if (status === VrpDownloader.Downloading && !isNaN(progress)) {
            progress_bar.value = progress;
            progress_bar.indeterminate = false;
            let downloaded = progress * size;
            let downloaded_unit = downloaded < 1024 ? "MB" : "GB";
            downloaded = downloaded < 1024 ? downloaded : downloaded / 1024;
            let total_size = size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB";
            action_button.text = downloaded.toFixed(2) + " " + downloaded_unit + " / " + total_size;
        }
    }
    onStatusChanged: function() {
        //TODO: add icon for each status
        progress_bar.value = 0;
        progress_bar.indeterminate = false;
        progress_bar.visible = false;
        action_button.enabled = false;
        action_button.icon.source = "";
        switch (status) {
        case VrpDownloader.UpdatableRemotely:
            action_button.text = qsTr("Download and Update");
            action_button.enabled = true;
            action_button.icon.source = "download";
            break;
        case VrpDownloader.Downloadable:
            action_button.text = qsTr("Download");
            action_button.enabled = true;
            action_button.icon.source = "download";
            break;
        case VrpDownloader.Queued:
            action_button.text = qsTr("Queued");
            break;
        case VrpDownloader.Downloading:
            if (isNaN(progress) || progress <= 1e-36) {
                action_button.text = qsTr("Starting Downloading");
                progress_bar.indeterminate = true;
                progress_bar.visible = true;
                action_button.icon.source = "download";
            }
            break;
        case VrpDownloader.Decompressing:
            action_button.text = qsTr("Decompressing");
            progress_bar.indeterminate = true;
            progress_bar.visible = true;
            break;
        case VrpDownloader.Local:
            action_button.text = qsTr("No Connected Device");
            progress_bar.value = 1;
            progress_bar.visible = true;
            action_button.icon.source = "install";
            break;
        case VrpDownloader.UpdatableLocally:
            action_button.text = qsTr("Update");
            action_button.enabled = true;
            action_button.icon.source = "install";
            break;
        case VrpDownloader.Installable:
            action_button.text = qsTr("Install");
            action_button.enabled = true;
            progress_bar.visible = true;
            progress_bar.value = 1;
            action_button.icon.source = "install";
            break;
        case VrpDownloader.Installing:
            action_button.text = qsTr("Installing");
            progress_bar.indeterminate = true;
            progress_bar.visible = true;
            break;
        case VrpDownloader.Installed:
            action_button.text = qsTr("Installed");
            progress_bar.value = 1;
            progress_bar.visible = true;
            action_button.icon.source = "install";
            break;
        case VrpDownloader.Error:
            action_button.text = qsTr("Error, Click to Try Again");
            action_button.enabled = true;
            action_button.icon.source = "error";
            break;
        }
    }
    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Column {
        id: game_info

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
            wrapMode: Text.WordWrap
            width: thumbnail.width
            height: font.pixelSize * 3
            font.pointSize: Qt.application.font.pointSize * 1.3
            color: app.globalPalette.text
        }

        Text {
            anchors.right: parent.right
            text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
            // font.pointSize: Qt.application.font.pointSize * 0.9
            color: app.globalPalette.text
        }

        Text {
            anchors.right: parent.right
            text: lastUpdated
            font.pointSize: Qt.application.font.pointSize * 0.8
            color: app.globalPalette.text
        }

    }

    Button {
        id: action_button

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progress_bar.top
        anchors.bottomMargin: 5
        text: qsTr("Downlad")
        width: game_info.width
        onClicked: {
            if (status === VrpDownloader.Installable || status === VrpDownloader.UpdatableLocally)
                app.vrp.installQml(modelData);
            else if (status === VrpDownloader.UpdatableRemotely || status === VrpDownloader.Error)
                app.vrp.addToDownloadQueue(modelData, true);
            else
                app.vrp.addToDownloadQueue(modelData, false);
        }
    }

    ProgressBar {
        id: progress_bar

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2
        value: (isNaN(progress) || progress <= 1e-36) ? 0 : progress
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
