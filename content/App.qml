// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 6.2
import QtQuick.Controls
import QtQuick.Layouts
import VrpDownloader 1.0

Window {
    visible: true
    width: 1920 
    height: 1080
    title: qsTr("QRookie")

    GridView {
        id: games_view

        anchors.fill: parent
        model: vrp.gamesInfo
        cellWidth: 400
        cellHeight: 400

        delegate: GameDelegate {
            width: games_view.cellWidth
            height: games_view.cellHeight
            name: modelData.name
            size: modelData.size
            last_updated: modelData.last_updated
            thumbnail_path: "file://" + vrp.getGameThumbnailPath(modelData.package_name)
        }

    }

    VrpDownloader {
        id: vrp

        Component.onCompleted: {
            vrp.updateMetadata();
            vrp.onMetadataUpdated.connect(function() {
                console.log("Metadata updated");
            });
            vrp.onMetadataUpdateFailed.connect(function() {
                console.log("Metadata update failed");
            });
        }
    }

}
