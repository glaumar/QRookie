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
                function onDevicesListChanged() {
                    let model = app.deviceManager.devicesList;
                    if (model.length <= 0) {
                        device_selector.model = ["No devices found"];
                        return ;
                    }
                    device_selector.model = model;
                    if (app.deviceManager.hasConnectedDevice) {
                        let i = device_selector.find(app.deviceManager.connectedDevice);
                        device_selector.currentIndex = i;
                    } else if (device_card.autoConnect) {
                        app.deviceManager.connectToDevice(model[0]);
                    }
                }

                function onConnectedDeviceChanged() {
                    if (app.deviceManager.hasConnectedDevice) {
                        let i = device_selector.find(app.deviceManager.connectedDevice);
                        device_selector.currentIndex = i;
                    }
                }

                function onSpaceUsageChanged(total_space, free_space) {
                    if (!isNaN(total_space) && !isNaN(free_space) && free_space > 0 && free_space <= total_space) {
                        space_usage_text.text = ((total_space - free_space) / 1024 / 1024).toFixed(2) + " GB / " + (total_space / 1024 / 1024).toFixed(2) + " GB";
                        space_usage_bar.value = (total_space - free_space) / total_space;
                    } else {
                        space_usage_text.text = "0.00 GB / 0.00 GB";
                        space_usage_bar.value = 0;
                    }
                }

                function onDeviceNameChanged(name) {
                    device_name.text = name === "" ? qsTr("No device connected") : name;
                }

                function onDeviceIpChanged(ip) {
                    device_ip.text = ip;
                }

                function onBatteryLevelChanged(level) {
                    battery_level.text = level > 0 ? level + "%" : "0%";
                }

                function onOculusOsVersionChanged(version) {
                    if (version.length > 20)
                        oculus_os_version.text = version.substr(0, 17) + "...";
                    else
                        oculus_os_version.text = version;
                    oculus_os_version.visible = !(version === "");
                }

                function onOculusVersionChanged(version) {
                    oculus_version.text = version;
                    oculus_version.visible = !(version === "");
                }

                function onOculusRuntimeVersionChanged(version) {
                    oculus_runtime_version.text = version;
                    oculus_runtime_version.visible = !(version === "");
                }

                function onAndroidVersionChanged(version) {
                    android_version.text = version;
                    android_version.visible = version > 0;
                }

                function onAndroidSdkVersionChanged(version) {
                    android_sdk_version.text = version;
                    android_sdk_version.visible = version > 0;
                }

                target: app.deviceManager
            }

            contentItem: ColumnLayout {
                Layout.fillWidth: true

                Column {
                    spacing: 10
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop

                    Label {
                        id: space_usage_text

                        anchors.right: parent.right
                        text: "0.00 GB / 0.00 GB"
                    }

                    ProgressBar {
                        id: space_usage_bar

                        width: parent.width
                        value: 0
                    }

                    ComboBox {
                        id: device_selector

                        width: parent.width
                        // model: app.deviceManager.devicesList
                        onActivated: (index) => {
                            app.deviceManager.connectToDevice(textAt(index));
                        }
                    }

                    Label {
                        id: wireless_label

                        text: qsTr("Wireless:")
                        font.bold: true
                    }

                    Kirigami.InlineMessage {
                        id: wireless_error_message

                        width: parent.width
                        visible: false
                        text: qsTr("Failed")
                        type: Kirigami.MessageType.Error
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: wireless_adress

                            width: parent.width
                            placeholderText: qsTr("192.168.1.10:5555")
                            Component.onCompleted: {
                                text = app.vrp.settings.lastWirelessAddr;
                                console.log("lastWirelessAddr: " + text);
                            }
                        }

                        Button {
                            text: qsTr("Connect")
                            enabled: wireless_adress.text !== ""
                            onClicked: {
                                app.deviceManager.connectToWirelessDeviceQml(wireless_adress.text).then((connected) => {
                                    if (connected) {
                                        app.vrp.settings.lastWirelessAddr = wireless_adress.text;
                                    } else {
                                        wireless_error_message.visible = true;
                                        wireless_error_message.text = qsTr("Failed to connect to " + wireless_adress.text);
                                    }
                                });
                            }
                        }

                    }

                    Button {
                        id: wireless_mode_button

                        text: qsTr("Enable Wireless Mode")
                        width: parent.width
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("adb tcpip 5555")
                        enabled: app.deviceManager.hasConnectedDevice
                        onClicked: {
                            device_card.autoConnect = false;
                            let address = app.deviceManager.deviceIp + ":5555";
                            app.deviceManager.enableTcpModeQml(5555).then((is_enabled) => {
                                if (is_enabled) {
                                    // device ip is empty
                                    if (address === ":5555") {
                                        device_card.autoConnect = true;
                                        return ;
                                    }
                                    app.deviceManager.connectToWirelessDeviceQml(address).then((connected) => {
                                        if (!connected) {
                                            wireless_error_message.visible = true;
                                            wireless_error_message.text = qsTr("Failed to connect to " + address);
                                        }
                                        device_card.autoConnect = true;
                                    });
                                } else {
                                    wireless_error_message.visible = true;
                                    wireless_error_message.text = qsTr("Failed to enable wireless mode");
                                    device_card.autoConnect = true;
                                }
                            });
                        }
                    }

                }

                Kirigami.FormLayout {
                    id: device_info_layout

                    Layout.alignment: Qt.AlignBottom
                    Layout.fillWidth: true
                    visible: app.deviceManager.hasConnectedDevice

                    Label {
                        id: device_ip

                        Kirigami.FormData.label: qsTr("IP:")
                    }

                    Label {
                        id: battery_level

                        Kirigami.FormData.label: qsTr("Battery Level:")
                    }

                    Label {
                        id: android_version

                        Kirigami.FormData.label: qsTr("Android Version:")
                    }

                    Label {
                        id: android_sdk_version

                        Kirigami.FormData.label: qsTr("Android SDK Version:")
                    }

                    Label {
                        id: oculus_os_version

                        Kirigami.FormData.label: qsTr("OS Version:")
                        text: ""
                    }

                    Label {
                        id: oculus_version

                        Kirigami.FormData.label: qsTr("Oculus Version:")
                    }

                    Label {
                        id: oculus_runtime_version

                        Kirigami.FormData.label: qsTr("Oculus Runtime Version:")
                    }

                }

            }

            header: Label {
                id: device_name

                text: qsTr("No device connected")
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
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
        model: app.deviceManager.appListModel()

        Dialog {
            id: confirm_dialog

            property string packageName: ""
            property int index: -1

            anchors.centerIn: parent
            title: "Uninstall " + packageName
            standardButtons: Dialog.Ok | Dialog.Cancel
            onAccepted: {
                apps_info.model.remove(index);
                app.deviceManager.uninstallApkQml(packageName);
            }
        }

        ScrollBar.vertical: ScrollBar {
            visible: true
        }

        delegate: ApplicationDelegate {
            width: apps_info.cellWidth - 10
            height: apps_info.cellHeight - 10
            name: model.package_name
            thumbnailPath: {
                let path = app.vrp.getGameThumbnailPath(model.package_name);
                if (path === "")
                    return "qrc:/qt/qml/content/Image/matrix.png";
                else
                    return "file://" + path;
            }
            onUninstallButtonClicked: {
                confirm_dialog.packageName = model.package_name;
                confirm_dialog.index = index;
                confirm_dialog.open();
            }
        }

    }

}
