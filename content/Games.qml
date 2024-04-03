/*
 Copyright (c) 2024 glaumar <glaumar@geekgo.tech>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

ColumnLayout {
    Rectangle {
        id: toolbar

        color: app.globalPalette.base
        radius: 5
        Layout.fillWidth: true
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        height: 40

        RowLayout {
            anchors.fill: parent

            TextField {
                id: filter_field

                Layout.fillWidth: true
                Layout.fillHeight: true
                placeholderText: qsTr("Filter by name...")
                onTextChanged: app.vrp.filterGamesByName(text)
            }

            RadioButton {
                checked: true
                text: qsTr("All")
                onClicked: app.vrp.filterGamesByStatus(VrpDownloader.Unknown)
            }

            RadioButton {
                text: qsTr("Downloading")
                onClicked: app.vrp.filterGamesByStatus(VrpDownloader.Downloading | VrpDownloader.Queued | VrpDownloader.DownloadError | VrpDownloader.Decompressing | VrpDownloader.DecompressionError)
            }

            RadioButton {
                text: qsTr("Local")
                onClicked: app.vrp.filterGamesByStatus(VrpDownloader.Local | VrpDownloader.UpdatableLocally | VrpDownloader.Installing | VrpDownloader.InstallError | VrpDownloader.InstalledAndLocally)
            }

            RadioButton {
                text: qsTr("Updatable")
                onClicked: app.vrp.filterGamesByStatus(VrpDownloader.UpdatableLocally | VrpDownloader.UpdatableRemotely)
                enabled: app.vrp.connectedDevice.length > 0
            }

            RadioButton {
                text: qsTr("Installed")
                onClicked: app.vrp.filterGamesByStatus(VrpDownloader.InstalledAndLocally | VrpDownloader.InstalledAndRemotely)
                enabled: app.vrp.connectedDevice.length > 0
                Layout.rightMargin: 10
            }

        }

    }

    GridView {
        id: games

        clip: true
        Layout.fillWidth: true
        Layout.fillHeight: true
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
            size: modelData.size
            lastUpdated: modelData.last_updated
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
                function onDownloadProgressChanged(release_name_, progress_) {
                    if (modelData.release_name === release_name_)
                        progress = progress_;

                }

                function onStatusChanged(release_name_, status_) {
                    if (modelData.release_name === release_name_)
                        status = status_;

                }

                target: app.vrp
            }

        }

    }

}
