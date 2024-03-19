import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    Layout.margins: 10

    ListView {
        id: downloading_list

        Layout.fillHeight: true
        implicitWidth: 600
        spacing: 20
        snapMode: ListView.SnapToItem
        model: app.vrp.downloadsQueue

        Text {
            anchors.centerIn: downloading_list
            text: qsTr("Empty")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
            color: app.globalPalette.text
            visible: downloading_list.count <= 0
        }

        header: Label {
            text: qsTr("Downloading")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
            color: app.globalPalette.text
        }

        delegate: DownloadDelegate {
            width: 590
            height: 160
            name: modelData.name
            size: modelData.size
                        thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(modelData.package_name);
                if (path === "")
                    return "Image/matrix.png";
                else
                    return "file://" + path;
            }
            progress: 0
            status: app.vrp.getStatus(modelData)

            Connections {
                // function onConnectedDeviceChanged() {
                //     status = app.vrp.getStatus(modelData);
                // }

                function onDownloadProgressChanged(release_name, progress_, speed_) {
                    if (modelData.release_name === release_name)
                        progress = progress_;

                }

                function onStatusChanged(release_name, status_) {
                    if (modelData.release_name === release_name)
                        status = status_;

                }

                target: app.vrp
            }

        }

    }

    ListView {
        id: local_list

        Layout.fillHeight: true
        implicitWidth: 600
        spacing: 20
        snapMode: ListView.SnapToItem
        model: app.vrp.localQueue

        Text {
            anchors.centerIn: local_list
            text: qsTr("Empty")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
            color: app.globalPalette.text
            visible: local_list.count <= 0
        }

        header: Label {
            text: qsTr("Local")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
            color: app.globalPalette.text
        }

        delegate: LocalDelegate {
            width: 640
            height: 160
            releaseName: modelData.release_name
            size: modelData.size
                        thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(modelData.package_name);
                if (path === "")
                    return "Image/matrix.png";
                else
                    return "file://" + path;
            }
            status: app.vrp.getStatus(modelData)

            Connections {
                function onStatusChanged(release_name_, status_) {
                    if (modelData.release_name === release_name_)
                        status = status_;

                }

                function onInstalledQueueChanged() {
                    status = app.vrp.getStatus(modelData);
                }

                target: app.vrp
            }

        }

    }

}
