
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpManager
import org.kde.kirigami as Kirigami

Kirigami.Card {
    property string name
    property string thumbnailPath

    signal addButtonClicked()

    banner {
        source: thumbnailPath
        title: name
        titleAlignment: Qt.AlignLeft | Qt.AlignBottom
        titleWrapMode: Text.Wrap
    }

    footer: Button {
        id: action_button

        hoverEnabled: true
        icon.name: "install"
        text: qsTr("Add to User")
        onClicked: {
            addButtonClicked();
        }
    }

}
