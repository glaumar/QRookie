import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Rectangle {
    id: game_info

    property var size
    property var releaseName
    property var thumbnailPath
    property var status

    signal installButtonClicked
    signal deleteButtonClicked

    radius: 5
    layer.enabled: true
    color: app.globalPalette.base
    onStatusChanged: function() {
        install_button.enabled = false;
        install_button.icon.source = "install";
        switch (status) {
        case VrpDownloader.Local:
            install_button.text = qsTr("No Connected Device");
            break;
        case VrpDownloader.UpdatableLocally:
            install_button.text = qsTr("Update");
            install_button.enabled = true;
            break;
        case VrpDownloader.Installable:
            install_button.text = qsTr("Install");
            install_button.enabled = true;
            break;
        case VrpDownloader.Installing:
            install_button.text = qsTr("Installing...");
            break;
        // case VrpDownloader.Installed:
        //     install_button.text = qsTr("Reinstall");
        //     install_button.enabled = true;
        //     break;
        // case VrpDownloader.Error:
        //     install_button.text = qsTr("Error, Click to Try Again");
        //     install_button.enabled = true;
        //     install_button.icon.source = "error";
        //     break;
        default:
            install_button.text = qsTr("Unknown Status");
            break;
        }
    }

    Image {
        id: thumbnail

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
        height: parent.height - 20
        asynchronous: true
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
        id: name_text

        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.top: parent.top
        text: releaseName
        width: parent.width - thumbnail.width - 30
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2.5
        font.pointSize: Qt.application.font.pointSize * 1.3
        color: app.globalPalette.text
    }

    Text {
        id: size_text

        anchors.margins: 10
        anchors.top: name_text.bottom
        anchors.left: thumbnail.right
        text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
        color: app.globalPalette.text
    }

    Button {
        id: install_button

        anchors.left: thumbnail.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        onClicked: {
            installButtonClicked();
        }
    }

    Button {
        id: delete_button

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        icon.source: "delete"
        onClicked: {
            deleteButtonClicked();
        }
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
