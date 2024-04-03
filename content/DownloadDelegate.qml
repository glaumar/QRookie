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

import Qt5Compat.GraphicalEffects
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

Rectangle {
    id: game_info

    property var name
    property var size
    property var thumbnailPath
    property var progress
    property var status

    signal deleteButtonClicked()

    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Image {
        id: thumbnail

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 10
        height: parent.height - 20
        asynchronous: true
        source: thumbnailPath
        fillMode: Image.PreserveAspectFit
        layer.enabled: true

        layer.effect: OpacityMask {

            maskSource: Rectangle {
                width: thumbnail.width
                height: thumbnail.height
                radius: 5
            }

        }

    }

    Text {
        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.top: parent.top
        text: name
        width: parent.width - thumbnail.width - 30
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2
        font.pointSize: Qt.application.font.pointSize * 1.3
        color: app.globalPalette.text
    }

    ProgressBar {
        id: progress_bar

        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.bottom: parent.bottom
        value: progress
        layer.enabled: true

        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 6
            verticalOffset: 6
            color: app.globalPalette.shadow
        }

    }

    Text {
        id: status_label

        anchors.margins: 10
        anchors.right: progress_bar.right
        anchors.bottom: progress_bar.top
        text: {
            if (status === VrpDownloader.Queued) {
                progress_bar.indeterminate = false;
                delete_button.enabled = true;
                status_label.color = app.globalPalette.text;
                return qsTr("Queued");
            } else if (status === VrpDownloader.Decompressing) {
                progress_bar.indeterminate = true;
                delete_button.enabled = false;
                status_label.color = app.globalPalette.text;
                return qsTr("Decompressing");
            } else if (status === VrpDownloader.Downloading) {
                delete_button.enabled = true;
                status_label.color = app.globalPalette.text;
                if (isNaN(progress) || progress <= 1e-36) {
                    progress_bar.indeterminate = true;
                    return qsTr("Starting Downloading");
                } else {
                    progress_bar.indeterminate = false;
                    let downloaded = progress * size;
                    let downloaded_unit = downloaded < 1024 ? "MB" : "GB";
                    downloaded = downloaded < 1024 ? downloaded : downloaded / 1024;
                    let total_size = size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB";
                    return downloaded.toFixed(2) + " " + downloaded_unit + " / " + total_size;
                }
            } else if (status === VrpDownloader.DownloadError) {
                delete_button.enabled = true;
                progress_bar.indeterminate = false;
                status_label.color = "red";
                return qsTr("DownloadError");
            } else if (status === VrpDownloader.DecompressionError) {
                delete_button.enabled = true;
                progress_bar.indeterminate = false;
                status_label.color = "red";
                return qsTr("DecompressionError");
            } else {
                delete_button.enabled = false;
                progress_bar.indeterminate = false;
                return qsTr("Unknown Status");
            }
        }
        color: app.globalPalette.text
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

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
