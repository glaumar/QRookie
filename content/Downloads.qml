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
import org.kde.kirigami as Kirigami

RowLayout {
    anchors.fill: parent

    ColumnLayout {
        Layout.preferredWidth: parent.width / 2
        Layout.fillWidth: true
        Layout.fillHeight: true

        Label {
            text: qsTr("Downloading")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
        }

        ListView {
            id: downloading_list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 10
            snapMode: ListView.SnapToItem
            model: app.vrp.downloadsQueue

            Label {
                anchors.centerIn: downloading_list
                text: qsTr("Empty")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
                visible: downloading_list.count <= 0
            }

            delegate: DownloadDelegate {
                height: 130
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

    }

    ColumnLayout {
        Layout.preferredWidth: parent.width / 2
        Layout.fillWidth: true
        Layout.fillHeight: true

        Label {
            text: qsTr("Local")
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
        }

        Kirigami.CardsListView {
            id: local_list

            property int oldIndex: 0

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            clip: true
            snapMode: ListView.SnapToItem
            model: app.vrp.localQueue
            onModelChanged: {
                if (count - 1 >= oldIndex)
                    positionViewAtIndex(oldIndex, ListView.Center);
                else
                    positionViewAtIndex(count - 1, ListView.Center);
            }

            Label {
                anchors.centerIn: local_list
                text: qsTr("Empty")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
                visible: local_list.count <= 0
            }

            delegate: LocalDelegate {
                height: 130
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

}
