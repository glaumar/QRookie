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
                    user_running.text = app.deviceManager.selectedUserIsLogged ? qsTr("Yes") : qsTr("No");
                    bar.currentIndex = 0;
                    onUserAppsListChanged();
                }

                function onUserAppsListChanged() {
                    installed_apps_number.text = app.deviceManager.selectedUsersInstalledApps > -1
                        ? app.deviceManager.selectedUsersInstalledApps
                        : qsTr("loading...");
                    installed_tab.text = qsTr("Installed (") + app.deviceManager.selectedUsersInstalledApps + qsTr(")");
                    available_tab.text = qsTr("Available (") + app.deviceManager.availableAppsCount + qsTr(")");
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
                            id: installed_apps_number
                            Kirigami.FormData.label: qsTr("Installed Apps:")
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        StackLayout {
            currentIndex: bar.currentIndex

            //anchors.fill: parent

            GridView {
                id: installed_apps

                Layout.fillHeight: true
                Layout.fillWidth: true
                snapMode: GridView.SnapToRow
                clip: true
                cellWidth: 220
                cellHeight: 180
                model: app.deviceManager.userAppsListModel()

                Dialog {
                    id: remove_confirm_dialog

                    property string packageName: ""
                    property int index: -1

                    anchors.centerIn: parent
                    title: "Remove " + packageName + " from user" + app.deviceManager.selectedUserName
                    standardButtons: Dialog.Ok | Dialog.Cancel
                    onAccepted: {
                        installed_apps.model.remove(index);
                        app.deviceManager.uninstallFromUser(packageName)
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    visible: true
                }

                delegate: UsersInstalledDelegate {
                    width: installed_apps.cellWidth - 10
                    height: installed_apps.cellHeight - 10
                    name: model.package_name
                    thumbnailPath: {
                        let path = app.vrp.getGameThumbnailPath(model.package_name);
                        if (path === "")
                            return "qrc:/qt/qml/content/Image/matrix.png";
                        else
                            return "file://" + path;
                    }
                    onRemoveButtonClicked: {
                        remove_confirm_dialog.packageName = model.package_name;
                        remove_confirm_dialog.index = index;
                        remove_confirm_dialog.open();
                    }
                }
            }
            GridView {
                id: available_apps

                Layout.fillHeight: true
                Layout.fillWidth: true
                snapMode: GridView.SnapToRow
                clip: true
                cellWidth: 220
                cellHeight: 180
                model: app.deviceManager.userAppsAvailableListModel()

                Dialog {
                    id: confirm_dialog

                    property string packageName: ""
                    property int index: -1

                    anchors.centerIn: parent
                    title: "Add " + packageName + " to user" + app.deviceManager.selectedUserName
                    standardButtons: Dialog.Ok | Dialog.Cancel
                    onAccepted: {
                        available_apps.model.remove(index);
                        app.deviceManager.installToUser(packageName);
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    visible: true
                }

                delegate: UsersToAddDelegate {
                    width: available_apps.cellWidth - 10
                    height: available_apps.cellHeight - 10
                    name: model.package_name
                    thumbnailPath: {
                        let path = app.vrp.getGameThumbnailPath(model.package_name);
                        if (path === "")
                            return "qrc:/qt/qml/content/Image/matrix.png";
                        else
                            return "file://" + path;
                    }
                    onAddButtonClicked: {
                        confirm_dialog.packageName = model.package_name;
                        confirm_dialog.index = index;
                        confirm_dialog.open();
                    }
                }
            }
        }
        TabBar {
            id: bar
            Layout.fillWidth: true

            TabButton {
                id: installed_tab
                text: qsTr("Installed")
            }

            TabButton {
                id: available_tab
                text: qsTr("Available")
            }
        }
    }
}
