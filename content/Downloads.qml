import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    property var vrp: vrp
    Layout.margins: 10

    ListView {
        id: downloading_list

        Layout.fillHeight: true
        implicitWidth: 600
        spacing: 20
        snapMode: ListView.SnapToItem 
        model: vrp.downloadsQueue

        header: Text {
            text: "Downloading"
        }

        delegate: DownloadDelegate {
            width: 590
            height: 160
            name: modelData.name
            size: modelData.size
            thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
            progress: 0

            Connections {
                target: vrp
                onDownloadProgressChanged: function(release_name, progress_, speed_) {
                    if (modelData.release_name === release_name)
                        progress = progress_;

                }
            }

        }
    }

    ListView {
        Layout.fillHeight: true
        implicitWidth: 600

        spacing: 20
        snapMode: ListView.SnapToItem 
        model: vrp.downloadsQueue

        delegate: DownloadDelegate {
            width: 640
            height: 160
            name: modelData.name
            size: modelData.size
            thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
            progress: 0

            Connections {
                target: vrp
                onDownloadProgressChanged: function(release_name, progress_, speed_) {
                    if (modelData.release_name === release_name)
                        progress = progress_;

                }
            }

        }
    }

}
