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

import QCoro
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpManager
import org.kde.kirigami as Kirigami

Kirigami.Card {
    property string name
    property string releaseName
    property int size
    property string lastUpdated
    property string versionCode
    property string thumbnailPath
    property double progress
    property var status

    signal installButtonClicked()
    signal downloadButtonClicked()

    onProgressChanged: {
        if (status === VrpManager.Downloading && !isNaN(progress)) {
            progress_bar.value = progress;
            progress_bar.indeterminate = false;
            let downloaded = progress * size;
            let downloaded_unit = downloaded < 1024 ? "MB" : "GB";
            downloaded = downloaded < 1024 ? downloaded : downloaded / 1024;
            let total_size = size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB";
            action_button.text = downloaded.toFixed(2) + " " + downloaded_unit + " / " + total_size;
        }
    }
    onStatusChanged: function() {
        progress_bar.value = 0;
        progress_bar.indeterminate = false;
        progress_bar.visible = false;
        action_button.enabled = false;
        action_button.icon.name = "";
        switch (status) {
        case VrpManager.UpdatableRemotely:
            action_button.text = qsTr("Download and Update");
            action_button.enabled = true;
            action_button.icon.name = "download";
            break;
        case VrpManager.Downloadable:
            action_button.text = qsTr("Download");
            action_button.enabled = true;
            action_button.icon.name = "download";
            break;
        case VrpManager.Queued:
            action_button.text = qsTr("Queued");
            break;
        case VrpManager.Downloading:
            if (isNaN(progress) || progress <= 0) {
                action_button.text = qsTr("Starting Downloading");
                progress_bar.indeterminate = true;
                progress_bar.visible = true;
                action_button.icon.name = "download";
            }
            break;
        case VrpManager.Decompressing:
            action_button.text = qsTr("Decompressing");
            progress_bar.indeterminate = true;
            progress_bar.visible = true;
            break;
        case VrpManager.Local:
            action_button.text = qsTr("No Connected Device");
            progress_bar.value = 1;
            progress_bar.visible = true;
            action_button.icon.name = "install";
            break;
        case VrpManager.UpdatableLocally:
            action_button.text = qsTr("Update");
            action_button.enabled = true;
            action_button.icon.name = "install";
            break;
        case VrpManager.Installable:
            action_button.text = qsTr("Install");
            action_button.enabled = true;
            progress_bar.visible = true;
            progress_bar.value = 1;
            action_button.icon.name = "install";
            break;
        case VrpManager.Installing:
            action_button.text = qsTr("Installing");
            progress_bar.indeterminate = true;
            progress_bar.visible = true;
            break;
        case VrpManager.InstalledAndRemotely:
        case VrpManager.InstalledAndLocally:
            action_button.text = qsTr("Installed");
            progress_bar.value = 1;
            progress_bar.visible = true;
            action_button.icon.name = "install";
            break;
        case VrpManager.DownloadError:
            action_button.text = qsTr("Re-download (DownloadError)");
            action_button.enabled = true;
            action_button.icon.name = "error";
            break;
        case VrpManager.DecompressionError:
            action_button.text = qsTr("Re-download (DecompressionError)");
            action_button.enabled = true;
            action_button.icon.name = "error";
            break;
        case VrpManager.InstallError:
            action_button.text = qsTr("Reinstall (InstallError)");
            action_button.enabled = true;
            action_button.icon.name = "error";
            break;
        }
    }

    banner {
        source: thumbnailPath
        title: name
        titleAlignment: Qt.AlignLeft | Qt.AlignBottom
    }

    contentItem: ColumnLayout {
        Layout.margins: 10

        Label {
            text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: "Version: " + versionCode
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: lastUpdated
        }

    }

    footer: ColumnLayout {
        Layout.margins: 5

        RowLayout {
            Button {
                id: magnet_button

                text: "Get Magnet"
                icon.name: "kt-magnet"
                onClicked: {
                    textEdit.text = app.vrp.getMagnetURI(releaseName);
                    textEdit.selectAll();
                    textEdit.copy();
                }
                ToolTip.text: qsTr("Copy Magnet")
                ToolTip.visible: hovered

                TextEdit {
                    id: textEdit

                    //just to copy the text
                    visible: false
                }

            }

            Button {
                id: action_button

                Layout.fillWidth: true
                text: qsTr("Downlad")
                onClicked: {
                    if (status === VrpManager.Installable || status === VrpManager.UpdatableLocally || status === VrpManager.InstallError)
                        installButtonClicked();
                    else
                        downloadButtonClicked();
                }
            }

        }

        ProgressBar {
            id: progress_bar

            Layout.fillWidth: true
            value: (isNaN(progress) || progress <= 0) ? 0 : progress
        }

    }

}
