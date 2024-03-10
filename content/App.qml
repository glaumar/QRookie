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

        Downloads {
            id: downloads_tab
            vrp: vrp
        }

        Item {
            id: deviceTab
        }

        // Item {
        //     id: settingsTab
        // }

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

    }

}
