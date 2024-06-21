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
                    user_running.text = app.deviceManager.selectedUserIsLogged
                        ? qsTr("Yes")
                        : qsTr("No");
                    onUserAppsListChanged();
                }

                function onUserAppsListChanged() {
                    installed_apps.text = app.deviceManager.selectedUsersInstalledApps > -1
                        ? app.deviceManager.selectedUsersInstalledApps
                        : qsTr("loading...");
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

    GridView {
        id: apps_info

        Layout.fillHeight: true
        Layout.fillWidth: true
        snapMode: GridView.SnapToRow
        clip: true
        cellWidth: 220
        cellHeight: 180
        model: app.deviceManager.userAppsListModel()

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

        delegate: UsersInstalledDelegate {
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
            // onUninstallButtonClicked: {
            //     confirm_dialog.packageName = model.package_name;
            //     confirm_dialog.index = index;
            //     confirm_dialog.open();
            // }
        }

    }
}
