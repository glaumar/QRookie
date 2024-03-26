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
            onDeleteButtonClicked: {
                app.vrp.removeFromDownloadQueue(modelData);
            }

            Connections {
                function onDownloadProgressChanged(release_name, progress_) {
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

        property int oldIndex: 0

        Layout.fillHeight: true
        implicitWidth: 600
        spacing: 20
        snapMode: ListView.SnapToItem
        model: app.vrp.localQueue
        onModelChanged: {
            if (count - 1 >= oldIndex)
                positionViewAtIndex(oldIndex, ListView.Center);
            else
                positionViewAtIndex(count - 1, ListView.Center);
        }

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
            onInstallButtonClicked: {
                app.vrp.installQml(modelData);
            }
            onDeleteButtonClicked: {
                local_list.oldIndex = local_list.indexAt(x, y);
                app.vrp.removeFromLocalQueue(modelData);
            }

            Connections {
                function onStatusChanged(release_name_, status_) {
                    if (modelData.release_name === release_name_)
                        status = status_;

                }

                target: app.vrp
            }

        }

    }

}
