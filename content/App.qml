import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader 1.0

Window {
    visible: true
    width: 1280
    height: 720
    title: qsTr("QRookie")

    GridView {
        // ScrollBar.vertical: ScrollBar {
        //     parent: lv.parent
        //     anchors.top: lv.top
        //     anchors.left: lv.right
        //     anchors.bottom: lv.bottom
        // }

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
        }

    }

    VrpDownloader {
        id: vrp

        Component.onCompleted: {
            vrp.updateMetadata();
            console.log(Qt.application.font.pointSize)
            vrp.onMetadataUpdated.connect(function() {
                console.log("Metadata updated");
            });
            vrp.onMetadataUpdateFailed.connect(function() {
                console.log("Metadata update failed");
            });
        }
    }

}
