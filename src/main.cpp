#include <QCoroQml>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "device_manager.h"
#include "qrookie.h"
#include "vrp_downloader.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setApplicationVersion(APPLICATION_VERSION);
    app.setDesktopFileName(APPLICATION_ID);

    qmlRegisterType<VrpDownloader>("VrpDownloader", 1, 0, "VrpDownloader");
    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/Main/main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
