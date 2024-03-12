qt_add_library(content STATIC)
qt6_add_qml_module(content
    URI "content"
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        App.qml
        GameDelegate.qml
        Downloads.qml
        DownloadDelegate.qml
        LocalDelegate.qml
    RESOURCES
        fonts/fonts.txt
)
