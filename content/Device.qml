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
import org.kde.kirigami as Kirigami

RowLayout {
    Item {
        id: device_info

        Layout.fillHeight: true
        Layout.bottomMargin: 10
        Layout.rightMargin: 5
        width: 310

        Kirigami.Card {
            anchors.fill: parent

            Connections {
                function onSpaceUsageChanged(total_space, free_space) {
                    if (!isNaN(total_space) && !isNaN(free_space) && free_space > 0 && free_space <= total_space) {
                        space_usage_text.text = ((total_space - free_space) / 1024 / 1024).toFixed(2) + " GB / " + (total_space / 1024 / 1024).toFixed(2) + " GB";
                        space_usage_bar.value = (total_space - free_space) / total_space;
                    } else {
                        space_usage_text.text = "0.00 GB / 0.00 GB";
                        space_usage_bar.value = 0;
                    }
                }

                target: app.vrp
            }

            contentItem: Column {
                spacing: 10

                Label {
                    id: space_usage_text

                    width: parent.width
                    text: "0.00 GB / 0.00 GB"
                }

                ProgressBar {
                    id: space_usage_bar

                    width: parent.width
                    value: 0
                }

                ComboBox {
                    id: device_selector

                    property int last_index: -1

                    width: parent.width
                    model: app.vrp.deviceList
                    onModelChanged: {
                        if (device_selector.count > 0 && app.vrp.hasConnectedDevice()) {
                            let index = find(app.vrp.connectedDevice);
                            if (index >= 0) {
                                device_selector.currentIndex = index;
                                device_selector.last_index = index;
                            }
                        }
                    }
                    onCurrentTextChanged: {
                        if (device_selector.count > 0 && app.vrp.connectedDevice !== device_selector.currentText && device_selector.currentIndex !== device_selector.last_index) {
                            device_selector.last_index = device_selector.currentIndex;
                            app.vrp.connectedDevice = device_selector.currentText;
                        }
                    }
                }

            }

            header: Kirigami.Heading {
                id: device_name

                text: app.vrp.deviceModel === "" ? "No device connected" : app.vrp.deviceModel
                level: 1
            }

        }

    }

    GridView {
        id: apps_info

        Layout.fillHeight: true
        Layout.fillWidth: true
        snapMode: GridView.SnapToRow
        clip: true
        cellWidth: 310
        cellHeight: 220
        model: app.vrp.installedQueue

        ScrollBar.vertical: ScrollBar {
            visible: true
        }

        delegate: ApplicationDelegate {
            width: apps_info.cellWidth - 10
            height: apps_info.cellHeight - 10
            name: modelData.package_name
            thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(modelData.package_name);
                if (path === "")
                    return "qrc:/qt/qml/content/Image/matrix.png";
                else
                    return "file://" + path;
            }
            onUninstallButtonClicked: {
                app.vrp.uninstallQml(modelData.package_name);
            }
        }

    }

}
