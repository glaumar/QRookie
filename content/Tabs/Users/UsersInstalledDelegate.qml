import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpManager
import org.kde.kirigami as Kirigami

Kirigami.Card {
    property string name
    property string thumbnailPath

    signal removeButtonClicked()

    banner {
        source: thumbnailPath
        title: name
        titleAlignment: Qt.AlignLeft | Qt.AlignBottom
        titleWrapMode: Text.Wrap
    }

    footer: Button {
        id: action_button

        hoverEnabled: true
        icon.name: "delete"
        text: qsTr("Remove from user")
        onClicked: {
            removeButtonClicked();
        }
    }

}
