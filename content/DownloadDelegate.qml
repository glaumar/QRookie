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

Kirigami.Card {
    id: game_info

    property string name
    property int size
    property string thumbnailPath
    property double progress
    property var status

    signal deleteButtonClicked()

    onProgressChanged: function() {
        progress_bar.indeterminate = false;
        let downloaded = progress * size;
        let downloaded_unit = downloaded < 1024 ? "MB" : "GB";
        downloaded = downloaded < 1024 ? downloaded : downloaded / 1024;
        let total_size = size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB";
        status_label.text = downloaded.toFixed(2) + " " + downloaded_unit + " / " + total_size;
    }
    onStatusChanged: function() {
        if (status === VrpManager.Queued) {
            progress_bar.indeterminate = false;
            delete_button.enabled = true;
            status_label.color = Kirigami.Theme.textColor;
            status_label.text = qsTr("Queued");
        } else if (status === VrpManager.Decompressing) {
            progress_bar.indeterminate = true;
            delete_button.enabled = false;
            status_label.color = Kirigami.Theme.textColor;
            status_label.text = qsTr("Decompressing");
        } else if (status === VrpManager.Downloading) {
            delete_button.enabled = true;
            status_label.color = Kirigami.Theme.textColor;
            if (isNaN(progress) || progress <= 0) {
                progress_bar.indeterminate = true;
                status_label.text = qsTr("Starting Downloading");
            }
        } else if (status === VrpManager.DownloadError) {
            delete_button.enabled = true;
            progress_bar.indeterminate = false;
            status_label.color = "red";
            status_label.text = qsTr("DownloadError");
        } else if (status === VrpManager.DecompressionError) {
            delete_button.enabled = true;
            progress_bar.indeterminate = false;
            status_label.color = "red";
            status_label.text = qsTr("DecompressionError");
        } else {
            delete_button.enabled = false;
            progress_bar.indeterminate = false;
            console.log("Unknown status: " + status);
            status_label.text = qsTr("Unknown Status");
        }
    }

    Image {
        id: thumbnail

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
        height: parent.height - 20
        asynchronous: true
        source: thumbnailPath
        fillMode: Image.PreserveAspectFit
    }

    Label {
        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.top: parent.top
        text: name
        width: parent.width - thumbnail.width - 30
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2
        font.pointSize: Qt.application.font.pointSize * 1.3
    }

    ProgressBar {
        id: progress_bar

        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.bottom: parent.bottom
        value: progress
        layer.enabled: true
    }

    Label {
        id: status_label

        anchors.margins: 10
        anchors.right: progress_bar.right
        anchors.bottom: progress_bar.top
        text: "Unknown Status"
    }

    Button {
        id: delete_button

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        icon.source: "delete"
        onClicked: {
            deleteButtonClicked();
        }
    }

}
