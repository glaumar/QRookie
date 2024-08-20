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

ColumnLayout {
    Kirigami.Card {
        Layout.rightMargin: 10
        Layout.bottomMargin: 10
        height: 50

        Kirigami.ActionToolBar {
            id: toolbar

            anchors.left: parent.left
            anchors.right: parent.right
            Layout.rightMargin: 10
            Layout.bottomMargin: 10
            height: 50

            actions: [
                Kirigami.Action {
                    id: sort_button

                    property int sortOrder: Qt.AscendingOrder

                    icon.name: "view-sort-ascending"
                    onTriggered: {
                        if (sortOrder === Qt.AscendingOrder) {
                            icon.name = "view-sort-descending";
                            sortOrder = Qt.DescendingOrder;
                        } else {
                            icon.name = "view-sort-ascending";
                            sortOrder = Qt.AscendingOrder;
                        }
                        app.vrp.sortGames(sort_field.currentIndex, sortOrder);
                    }
                },

                Kirigami.Action {
                    id: sort_field

                    displayComponent: ComboBox {
                        onActivated: app.vrp.sortGames(currentIndex, sort_button.sortOrder)
                        model: ["Update Date", "Name", "Size"]
                    }
                },

                Kirigami.Action {
                    displayComponent: Kirigami.SearchField {
                        id: filter_field

                        placeholderText: qsTr("Filter by name...")
                        implicitWidth: Math.max(200, toolbar.width / 3)
                        focusSequence: StandardKey.Find
                        onAccepted: app.vrp.filterGamesByName(text)
                        Component.onCompleted: {
                            forceActiveFocus();
                        }
                    }
                },

                Kirigami.Action {
                    displayComponent: RadioButton {
                        checked: true
                        text: qsTr("All")
                        onClicked: app.vrp.filterGamesByStatus(VrpManager.Unknown)
                    }
                },

                Kirigami.Action {
                    displayComponent: RadioButton {
                        text: qsTr("Downloading")
                        onClicked: app.vrp.filterGamesByStatus(VrpManager.Downloading | VrpManager.Queued | VrpManager.DownloadError | VrpManager.Decompressing | VrpManager.DecompressionError)
                    }
                },

                Kirigami.Action {
                    displayComponent: RadioButton {
                        text: qsTr("Local")
                        onClicked: app.vrp.filterGamesByStatus(VrpManager.Local | VrpManager.Installable | VrpManager.UpdatableLocally | VrpManager.Installing | VrpManager.InstallError | VrpManager.InstalledAndLocally)
                    }
                },

                Kirigami.Action {
                    displayComponent: RadioButton {
                        text: qsTr("Updatable")
                        onClicked: app.vrp.filterGamesByStatus(VrpManager.UpdatableLocally | VrpManager.UpdatableRemotely)
                        enabled: app.deviceManager.connectedDevice.length > 0
                    }
                },

                Kirigami.Action {
                    displayComponent: RadioButton {
                        text: qsTr("Installed")

                        onClicked: app.vrp.filterGamesByStatus(VrpManager.InstalledAndLocally | VrpManager.InstalledAndRemotely)
                        enabled: app.deviceManager.connectedDevice.length > 0
                    }
                },

                Kirigami.Action {
                    displayComponent: Button {
                        id: settings_button

                        flat: true
                        anchors.right: parent.right
                        icon.name: "settings-configure"
                        onClicked: settings_sheet.open()
                    }
                }
            ]

        }
    }

    Kirigami.OverlaySheet {
        id: settings_sheet

        property var headerHeight: header.height
        property var headerPadding: header.parent.height

        title: qsTr("Settings")
        implicitHeight: headerHeight +
                        headerPadding +
                        auto_install_setting.height +
                        auto_clean_cache_setting.height
        implicitWidth: 300

        Kirigami.FormLayout {
            id: settings_form

            anchors.fill: parent

            CheckBox {
                id: auto_install_setting

                Kirigami.FormData.label: qsTr("Auto Install:")
                Component.onCompleted: {
                    checked = app.vrp.settings.autoInstall;
                }
                onClicked: {
                    app.vrp.settings.autoInstall = checked;
                }
                text: qsTr("Enable")
                ToolTip.text: qsTr("Automatically install after the download is complete when a device is connected.")
                ToolTip.visible: hovered
            }

            CheckBox {
                id: auto_clean_cache_setting

                Kirigami.FormData.label: qsTr("Auto Clean Cache:")
                Component.onCompleted: {
                    checked = app.vrp.settings.autoCleanCache;
                }
                onClicked: {
                    app.vrp.settings.autoCleanCache = checked;
                }
                text: qsTr("Enable")
                ToolTip.text: qsTr("Automatically clean the cache when the game is decompressed.")
                ToolTip.visible: hovered
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
        cellWidth: 310
        cellHeight: 310

        ScrollBar.vertical: ScrollBar {
            visible: true
        }

        delegate: GameDelegate {
            id: gameDelegate

            width: games.cellWidth - 10
            height: games.cellHeight - 10
            name: modelData.name
            releaseName: modelData.release_name
            size: modelData.size
            lastUpdated: modelData.last_updated
            versionCode: modelData.version_code
            thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(modelData.package_name);
                if (path === "")
                    return "qrc:/qt/qml/content/Image/matrix.png";
                else
                    return "file://" + path;
            }
            progress: 0
            status: app.vrp.getStatus(modelData)
            onInstallButtonClicked: {
                app.vrp.installQml(modelData);
            }
            onDownloadButtonClicked: {
                app.vrp.addToDownloadQueue(modelData);
            }

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
