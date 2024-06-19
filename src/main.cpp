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

#include <QCoroQml>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "device_manager.h"
#include "qrookie.h"
#include "vrp_manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setApplicationVersion(APPLICATION_VERSION);
    app.setDesktopFileName(APPLICATION_ID);

    qmlRegisterType<VrpManager>("VrpManager", 1, 0, "VrpManager");
    qmlRegisterType<DeviceManager>("DeviceManager", 1, 0, "DeviceManager");
    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;

#if defined(Q_OS_MAC)
    QString projectDir = QCoreApplication::applicationDirPath() + "/../Resources";
    engine.addImportPath(projectDir + "/kirigami");
#endif

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

    return app.exec();
}
