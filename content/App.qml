import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader

ApplicationWindow {
    id: app

    property SystemPalette globalPalette
    property VrpDownloader vrp

    visible: true
    width: 1280
    height: 800
    title: qsTr("QRookie")
    Component.onCompleted: {
        vrp.updateMetadataQml().then(function() {
            //TODO: Add a loading screen
            console.log("Metadata updated");
        })
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

    }

    globalPalette: SystemPalette {
        colorGroup: SystemPalette.Active
    }

    vrp: VrpDownloader {
    }

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

    }

}
