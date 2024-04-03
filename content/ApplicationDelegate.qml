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
    property var name
    property var thumbnailPath

    signal uninstallButtonClicked()

    radius: 5
    layer.enabled: true
    color: app.globalPalette.base

    Column {
        id: app_info

        padding: 10
        spacing: 5
        width: parent.width - 20

        Image {
            id: thumbnail

            asynchronous: true
            width: parent.width
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
            text: name
            font.bold: true
            wrapMode: Text.WrapAnywhere
            width: thumbnail.width
            height: font.pixelSize * 3
            font.pointSize: Qt.application.font.pointSize * 1.3
            color: app.globalPalette.text
        }

    }

    Button {
        id: action_button

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        width: app_info.width
        text: qsTr("Uninstall")
        onClicked: {
            uninstallButtonClicked();
        }
    }

    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 6
        verticalOffset: 6
        color: app.globalPalette.shadow
    }

}
