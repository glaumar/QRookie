import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    Rectangle {
        id: toolbar

        color: app.globalPalette.base
        radius: 5
        anchors.top: parent.top
        Layout.fillWidth: true
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        height: 40
        z: 1

        TextField {
            anchors.fill: parent
            id: filter_field
            placeholderText: qsTr("Filter by name...")
            onTextChanged: app.vrp.filterGamesByName(text)
        }

    }

    GridView {
        id: games

        clip: true
        anchors.top: toolbar.bottom
        anchors.margins: 10
        Layout.fillWidth: true
        Layout.fillHeight: true
        snapMode: GridView.SnapToRow
        model: app.vrp.gamesInfo
        cellWidth: 315
        cellHeight: 255 + Qt.application.font.pixelSize * 7

        populate: Transition {
            NumberAnimation {
                properties: "x"
                duration: 100
            }

        }

        ScrollBar.vertical: ScrollBar {
            visible: true
        }

        delegate: GameDelegate {
            id: gameDelegate

            width: games.cellWidth - 20
            height: games.cellHeight - 20
            name: modelData.name
            size: modelData.size
            lastUpdated: modelData.last_updated
            thumbnailPath: "file://" + app.vrp.getGameThumbnailPath(modelData.package_name)
            progress: 0
            status: app.vrp.getStatus(modelData)

            Connections {
                function onDownloadProgressChanged(release_name_, progress_, speed_) {
                    if (modelData.release_name === release_name_)
                        progress = progress_;

                }

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
