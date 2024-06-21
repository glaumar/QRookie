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

import DeviceManager
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpManager
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: app

    property VrpManager vrp
    property DeviceManager deviceManager

    visible: true
    width: 1280
    height: 800
    title: qsTr("QRookie")
    Component.onCompleted: {
        deviceManager.enableAutoUpdate();
        vrp.updateMetadataQml();
    }

    StackLayout {
        currentIndex: bar.currentIndex
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.topMargin: 10

        Games {
            id: games_tab
        }

        Downloads {
            id: downloads_tab
        }

        Device {
            id: device_tab
        }

        Users {
            id: users_tab
        }

    }

    vrp: VrpManager {
    }

    deviceManager: vrp.deviceManager()

    header: TabBar {
        id: bar

        TabButton {
            text: qsTr("Games")
        }

        TabButton {
            text: qsTr("Downloads")
        }

        TabButton {
            text: qsTr("Devices")
        }

        TabButton {
            text: qsTr("Users")
        }

    }

}
