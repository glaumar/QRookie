import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

ApplicationWindow {
    id: app

    property SystemPalette globalPalette

    visible: true
    width: 1280
    height: 800
    title: qsTr("QRookie")
    Component.onCompleted: {
        vrp.updateMetadata();
    }

    VrpDownloader {
        id: vrp

        // onMetadataUpdated: {
        //     console.log("Metadata updated");
        // }
        // onMetadataUpdateFailed: {
        //     console.log("Metadata update failed");
        // }
    }

    StackLayout {
        currentIndex: bar.currentIndex
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.topMargin: 10

        GridView {
            id: games_tab

            snapMode: GridView.SnapToRow
            model: vrp.gamesInfo
            cellWidth: 315
            cellHeight: 255 + Qt.application.font.pixelSize * 7

            ScrollBar.vertical: ScrollBar {
                visible: true
            }

            delegate: GameDelegate {
                id: gameDelegate

                width: games_tab.cellWidth - 20
                height: games_tab.cellHeight - 20
                name: modelData.name
                size: modelData.size
                last_updated: modelData.last_updated
                thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
                onDownloadClicked: {
                    vrp.download(modelData); 
                }
            }

        }

        ListView {
            id: downloads_tab

            spacing: 15
            snapMode: GridView.SnapToRow
            model: vrp.downloadsQueue

            delegate: DownloadDelegate {
                width: 640
                height: 160
                name: modelData.name
                size: modelData.size
                last_updated: modelData.last_updated
                thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
            }

            ScrollBar.vertical: ScrollBar {
                visible: true
            }

        }

        Item {
            id: deviceTab
        }

        Item {
            id: settingsTab
        }

    }

    globalPalette: SystemPalette {
        colorGroup: SystemPalette.Active
    }

    header: TabBar {
        id: bar

        TabButton {
            text: qsTr("Games")
        }

        TabButton {
            text: qsTr("Downloads")
        }

        TabButton {
            text: qsTr("Devices")
        }

        TabButton {
            text: qsTr("Settings")
        }

    }

}
