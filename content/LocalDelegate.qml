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

    property int size
    property string releaseName
    property string thumbnailPath
    property var status

    signal installButtonClicked()
    signal deleteButtonClicked()

    onStatusChanged: function() {
        install_button.enabled = false;
        install_button.icon.source = "install";
        switch (status) {
        case VrpManager.Local:
            install_button.text = qsTr("No Connected Device");
            break;
        case VrpManager.UpdatableLocally:
            install_button.text = qsTr("Update");
            install_button.enabled = true;
            break;
        case VrpManager.Installable:
            install_button.text = qsTr("Install");
            install_button.enabled = true;
            break;
        case VrpManager.Installing:
            install_button.text = qsTr("Installing...");
            break;
        case VrpManager.InstalledAndLocally:
            install_button.text = qsTr("Reinstall");
            install_button.enabled = true;
            break;
        case VrpManager.InstallError:
            install_button.text = qsTr("Reinstall (InstallError)");
            install_button.enabled = true;
            install_button.icon.source = "error";
            break;
        default:
            install_button.text = qsTr("Unknown Status");
            break;
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
        id: name_text

        anchors.margins: 10
        anchors.left: thumbnail.right
        anchors.top: parent.top
        text: releaseName
        width: parent.width - thumbnail.width - 30
        font.bold: true
        wrapMode: Text.WordWrap
        height: font.pixelSize * 2.5
        font.pointSize: Qt.application.font.pointSize * 1.3
    }

    Label {
        id: size_text

        anchors.margins: 10
        anchors.top: name_text.bottom
        anchors.left: thumbnail.right
        text: size > 1024 ? (size / 1024).toFixed(2) + " GB" : size + " MB"
    }

    Button {
        id: install_button

        anchors.left: thumbnail.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        onClicked: {
            installButtonClicked();
        }
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
