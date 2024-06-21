import DeviceManager
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    Item {
        id: users_tab

        Layout.fillHeight: true
        Layout.bottomMargin: 10
        Layout.rightMargin: 5
        width: 310

        Kirigami.Card {
            id: user_card
            property bool autoConnect: true

            anchors.fill: parent

            Connections {
                function onUsersListChanged() {
                    if (app.deviceManager.usersList.length > 0) {
                        app.deviceManager.selectUser(0);
                        user_selector.model = app.deviceManager.usersList;
                    } else {
                        user_selector.model = ["No device connected"];
                    }
                }

                function onUserInfoChanged() {
                    user_name.text = app.deviceManager.selectedUserName;
                    user_id.text = app.deviceManager.selectedUserId;
                    user_running.text = app.deviceManager.selectedUserRunning
                        ? qsTr("Yes")
                        : qsTr("No");
                    installed_apps.text = app.deviceManager.selectedUsersInstalledApps;
                }

                target: app.deviceManager
            }

            header: Label {
                id: users_title

                text: "Users"
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 2
            }

            contentItem: ColumnLayout {
                Layout.fillWidth: true

                Column {
                    spacing: 10
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    ComboBox {
                        id: user_selector

                        model: ["No device connected"]
                        width: parent.width
                        onActivated: index => {
                            app.deviceManager.selectUser(index);
                        }
                    }

                    Label {
                        id: users_info_label

                        text: "User Info"
                        font.bold: true
                        font.pointSize: Qt.application.font.pointSize * 1.5
                    }

                    Kirigami.FormLayout {
                        id: user_info_layout

                        Layout.alignment: Qt.AlignBottom
                        Layout.fillWidth: true
                        visible: app.deviceManager.hasConnectedDevice

                        Label {
                            id: user_id
                            Kirigami.FormData.label: qsTr("User ID:")
                        }

                        Label {
                            id: user_name
                            Kirigami.FormData.label: qsTr("Name:")
                        }

                        Label {
                            id: user_running
                            Kirigami.FormData.label: qsTr("Is Logged:")
                        }

                        Label {
                            id: installed_apps
                            Kirigami.FormData.label: qsTr("Installed Apps:")
                        }

                    }
                }
            }
        }
    }
}
