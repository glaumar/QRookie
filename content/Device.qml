import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

RowLayout {
    id: device_layout

    Layout.margins: 10
    spacing: 10

    Rectangle {
        id: device_info

        width: 300
        Layout.fillHeight: true
        Layout.bottomMargin: 10
        color: app.globalPalette.base
        radius: 5
        layer.enabled: true

        Text {
            id: device_name

            width: parent.width - 20
            anchors.margins: 10
            anchors.left: parent.left
            anchors.top: parent.top
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
            color: app.globalPalette.text
            text: app.vrp.deviceModel === "" ? "No device connected" : app.vrp.deviceModel
        }

        Text {
            id: space_usage_text

            anchors.margins: 10
            anchors.right: parent.right
            anchors.top: device_name.bottom
            color: app.globalPalette.text
            text: (isNaN(app.vrp.totalSpace) || isNaN(app.vrp.freeSpace) || app.vrp.freeSpace < 10 || app.vrp.totalSpace < 10 || app.vrp.freeSpace > app.vrp.totalSpace) ? "0.00 GB / 0.00 GB" : ((app.vrp.totalSpace - app.vrp.freeSpace) / 1024 / 1024).toFixed(2) + " GB / " + (app.vrp.totalSpace / 1024 / 1024).toFixed(2) + " GB"
        }

        ProgressBar {
            id: space_usage_bar

            anchors.margins: 10
            width: parent.width - 20
            anchors.left: parent.left
            anchors.top: space_usage_text.bottom
            value: (isNaN(app.vrp.totalSpace) || isNaN(app.vrp.freeSpace) || app.vrp.freeSpace < 10 || app.vrp.totalSpace < 10 || app.vrp.freeSpace > app.vrp.totalSpace) ? 0 : (app.vrp.totalSpace - app.vrp.freeSpace) / app.vrp.totalSpace
        }

        ComboBox {
            id: device_selector

            property int last_index: -1

            width: parent.width - 20
            anchors.margins: 10
            anchors.left: parent.left
            anchors.top: space_usage_bar.bottom
            model: app.vrp.deviceList
            onModelChanged: {
                if (device_selector.count > 0 && app.vrp.hasConnectedDevice()) {
                    let index = find(app.vrp.connectedDevice);
                    if (index >= 0) {
                        device_selector.currentIndex = index;
                        device_selector.last_index = index;
                    }
                }
            }
            onCurrentTextChanged: {
                if (device_selector.count > 0 && app.vrp.connectedDevice !== device_selector.currentText && device_selector.currentIndex !== device_selector.last_index) {
                    device_selector.last_index = device_selector.currentIndex;
                    app.vrp.connectedDevice = device_selector.currentText;
                }
            }
        }

        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 6
            verticalOffset: 6
            color: app.globalPalette.shadow
        }

    }

    GridView {
        id: apps_info

        Layout.fillWidth: true
        Layout.fillHeight: true
        snapMode: GridView.SnapToRow
        cellWidth: 315
        cellHeight: 255 + Qt.application.font.pixelSize * 4
        model: app.vrp.installedQueue

        ScrollBar.vertical: ScrollBar {
            visible: true
        }

        delegate: ApplicationDelegate {
            width: apps_info.cellWidth - 20
            height: apps_info.cellHeight - 20
            name: modelData.package_name
                        thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(modelData.package_name);
                if (path === "")
                    return "Image/matrix.png";
                else
                    return "file://" + path;
            }
            Component.onCompleted: {
                let games_info = app.vrp.find(modelData.package_name);
                if (games_info.length > 0) {
                    gameInfo = games_info[0];
                    releaseName = gameInfo.release_name;
                    status = app.vrp.getStatus(gameInfo);
                }
            }

            Connections {
                function onStatusChanged(release_name_, status_) {
                    if (releaseName === release_name_)
                        status = status_;

                }

                target: app.vrp
            }

        }

    }

}
