qt_add_library(content STATIC)
qt6_add_qml_module(content
    URI "content"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        App.qml
        Tabs/Games/Games.qml
        Tabs/Games/GameDelegate.qml
        Tabs/Downloads/Downloads.qml
        Tabs/Downloads/DownloadDelegate.qml
        Tabs/Downloads/LocalDelegate.qml
        Tabs/Devices/Device.qml
        Tabs/Devices/ApplicationDelegate.qml
        Tabs/Users/Users.qml
        Tabs/Users/UsersInstalledDelegate.qml
        Tabs/Users/UsersToAddDelegate.qml
    RESOURCES
        fonts/fonts.txt
        Image/matrix.png
)
