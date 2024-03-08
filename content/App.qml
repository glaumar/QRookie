import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("QRookie")
    Component.onCompleted: {
        vrp.updateMetadata();
    }

    GridView {

        id: games_view

        anchors.fill: parent
        model: vrp.gamesInfo
        cellWidth: 360
        cellHeight: 280 + Qt.application.font.pixelSize * 5

        delegate: GameDelegate {
            width: games_view.cellWidth - 20
            height: games_view.cellHeight - 20
            name: modelData.name
            size: modelData.size
            last_updated: modelData.last_updated
            thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
            onDownloadClicked: {
                vrp.download(modelData.release_name);
            }

            // Connections {
            //     target: vrp
            //     onDownloadProgressChanged: function(release_name, progress, speed) {
            //         if (modelData.release_name === release_name && !isNaN(progress)) {
            //             console.log("Download progress changed: " + progress.toFixed(2) + "%");
            //         }
            //     }
            //     onDownloadSucceeded: {
            //         if (modelData.release_name === release_name) {
            //             console.log("Download succeeded");
            //         }
            //     }
            //     onDecompressSucceeded : {
            //         if (modelData.release_name === release_name) {
            //             console.log("Decompress succeeded");
            //         }
            //     }
            // }
        }

    }

    VrpDownloader {
        id: vrp

        onMetadataUpdated: {
            console.log("Metadata updated");
        }
        onMetadataUpdateFailed: {
            console.log("Metadata update failed");
        }
    }

}
