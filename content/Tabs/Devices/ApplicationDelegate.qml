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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import VrpManager
import org.kde.kirigami as Kirigami

Kirigami.Card {
    property string name
    property string thumbnailPath

    signal uninstallButtonClicked()

    banner {
        source: thumbnailPath
        title: name
        titleAlignment: Qt.AlignLeft | Qt.AlignBottom
        titleWrapMode: Text.Wrap
    }

    footer: Button {
        id: action_button

        hoverEnabled: true
        icon.name: "uninstall"
        text: qsTr("Uninstall For All Users")
        onClicked: {
            uninstallButtonClicked();
        }
    }

}
