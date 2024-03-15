import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridView {
    id: games

    snapMode: GridView.SnapToRow
    model: app.vrp.gamesInfo
    cellWidth: 315
    cellHeight: 255 + Qt.application.font.pixelSize * 7

    ScrollBar.vertical: ScrollBar {
        visible: true
    }

    delegate: GameDelegate {
        id: gameDelegate

        width: games.cellWidth - 20
        height: games.cellHeight - 20
        name: modelData.name
        release_name: modelData.release_name
        size: modelData.size
        last_updated: modelData.last_updated
        version_code: modelData.version_code
        package_name: modelData.package_name
        thumbnail_path: "file://" + app.vrp.getGameThumbnailPath(modelData.package_name)
        progress: 0
        status: app.vrp.getStatus(modelData)

        Connections {
            function onDownloadProgressChanged(release_name, progress_, speed_) {
                if (modelData.release_name === release_name)
                    progress = progress_;

            }

            function onStatusChanged(release_name, status_) {
                if (modelData.release_name === release_name)
                    status = status_;

            }

            function onInstalledQueueChanged() {
                status = app.vrp.getStatus(modelData);
            }

            target: app.vrp
        }

    }

}
