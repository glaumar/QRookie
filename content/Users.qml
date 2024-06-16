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

RowLayout {
    Item {
        id: device_info

        Layout.fillHeight: true
        Layout.bottomMargin: 10
        Layout.rightMargin: 5
        width: 310

        Kirigami.Card {
            id: device_card

            property bool autoConnect: true

            anchors.fill: parent

            Connections {
                target: app.deviceManager
            }

            contentItem: ColumnLayout {
                Layout.fillWidth: true

                Column {
                    spacing: 10
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop

                    ComboBox {
                        id: user_selector

                        width: parent.width
                        // model: app.deviceManager.devicesList
                        onActivated: (index) => {
                            // ChangeUser
                        }
                    }

                }

            }

            header: Label {
                id: device_name

                text: qsTr("Users")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
            }

        }

    }


}
