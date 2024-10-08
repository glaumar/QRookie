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
import VrpManager
import org.kde.kirigami as Kirigami

RowLayout {
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Label {
            id: download_title

            text: qsTr("Downloading (%1)").arg(downloading_list.count)
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 2
        }

        Kirigami.CardsListView {
            id: downloading_list

            width: parent.width
            height: parent.height - download_title.height
            anchors.top: download_title.bottom
            anchors.margins: 5
            spacing: 10
            clip: true
            snapMode: ListView.SnapToItem
            model: app.vrp.downloadGamesModel()

            Label {
                anchors.centerIn: downloading_list
                text: qsTr("Empty")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
                visible: downloading_list.count <= 0
            }

            delegate: DownloadDelegate {
                height: 130
                name: model.name
                size: model.size
                thumbnailPath: {
                    let path = app.vrp.getGameThumbnailPath(model.package_name);
                    if (path === "")
                        return "qrc:/qt/qml/content/Image/matrix.png";
                    else
                        return "file://" + path;
                }
                progress: 0
                status: app.vrp.getStatus(model.game_info)
                onDeleteButtonClicked: {
                    downloading_list.model.remove(model.index);
                }

                Connections {
                    function onDownloadProgressChanged(release_name, progress_) {
                        if (model.release_name === release_name)
                            progress = progress_;

                        if (status !== VrpManager.Downloading)
                            status = app.vrp.getStatus(model.game_info);

                    }

                    function onStatusChanged(release_name, status_) {
                        if (model.release_name === release_name)
                            status = status_;

                    }

                    target: app.vrp
                }

            }

        }

    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        RowLayout {
            id: local_title_row

            width: parent.width

            Label {
                id: local_title

                Layout.alignment: Qt.AlignLeft
                text: qsTr("Local (%1)").arg(local_list.count)
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
            }

            Row {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 10

                CheckBox {
                    id: rename_checkbox

                    Component.onCompleted: {
                        checked = app.vrp.settings.renamePackage;
                    }
                    onClicked: {
                        app.vrp.settings.renamePackage = checked;
                    }
                    text: qsTr("Rename pkg")
                    ToolTip.text: qsTr("Rename the package before installing the game. This is useful when a Quest prevents you from launching a game whose free trial has expired.")
                    ToolTip.visible: hovered
                }

            }

        }

        Kirigami.CardsListView {
            id: local_list

            width: parent.width
            height: parent.height - local_title.height
            anchors.top: local_title_row.bottom
            anchors.margins: 5
            spacing: 10
            clip: true
            snapMode: ListView.SnapToItem
            model: app.vrp.localGamesModel()

            Label {
                anchors.centerIn: local_list
                text: qsTr("Empty")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
                visible: local_list.count <= 0
            }

            delegate: LocalDelegate {
                height: 130
                releaseName: model.release_name
                size: model.size
                thumbnailPath: {
                    let path = app.vrp.getGameThumbnailPath(model.package_name);
                    if (path === "")
                        return "qrc:/qt/qml/content/Image/matrix.png";
                    else
                        return "file://" + path;
                }
                status: app.vrp.getStatus(model.game_info)
                onInstallButtonClicked: {
                    app.vrp.installQml(model.game_info);
                }
                onDeleteButtonClicked: {
                    local_list.model.remove(model.index);
                }
                onNameTextClicked: {
                    app.vrp.openGameFolderQml(model.release_name);
                }

                Connections {
                    function onStatusChanged(release_name_, status_) {
                        if (release_name === release_name_)
                            status = status_;

                    }

                    target: app.vrp
                }

            }

        }

    }

}
