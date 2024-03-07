// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "app_environment.h"
#include "import_qml_components_plugins.h"
#include "import_qml_plugins.h"
#include "qrookie.h"

//TODO:delete
#include "vrp_downloader.h"
#include "game_info.h"


int main(int argc, char *argv[])
{
    set_qt_environment();

    QGuiApplication app(argc, argv);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    app.setApplicationName(APPLICATION_NAME);
    app.setApplicationVersion(APPLICATION_VERSION);
    app.setDesktopFileName(DESKTOP_FILE_NAME);

    qmlRegisterType<VrpDownloader>("VrpDownloader", 1, 0, "VrpDownloader");
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/Main/main.qml"_qs);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    //TODO:delete
    // VrpDownloader vrp_downloader;
    // vrp_downloader.updateMetadata();

    return app.exec();
}
