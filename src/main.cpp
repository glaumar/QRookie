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
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QRegularExpression>

#include "device_manager.h"
#include "qrookie.h"
#include "vrp_manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setApplicationVersion(APPLICATION_VERSION);
    app.setDesktopFileName(APPLICATION_ID);

#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
    }
#elif defined(Q_OS_MAC)
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("Material"));
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
#ifdef MACOS_BUNDLE
    QString res_dir = QCoreApplication::applicationDirPath() + "/../Resources";

    // add ../Resources to the PATH (for adb,7za,zipalign,apktool,apksigner)
    QStringList path = env.value("PATH").split(QDir::listSeparator());
    path.prepend(res_dir);
    qputenv("PATH", path.join(QDir::listSeparator()).toUtf8());

    // add ../Resources/icons to the icon theme search path
    QStringList icon_dirs = QIcon::themeSearchPaths();
    icon_dirs += res_dir + "/icons";
    QIcon::setThemeSearchPaths(icon_dirs);
#endif // MACOS_BUNDLE

    // add XDG_DATA_DIRS/icons to the icon theme search path (for nix-darwin)
    if (env.contains("XDG_DATA_DIRS")) {
        QString xdg_data_dirs = env.value("XDG_DATA_DIRS");
        QStringList data_dirs = xdg_data_dirs.split(QDir::listSeparator());
        QStringList icon_dirs;
        for (auto dir : data_dirs) {
            auto icon_dir = dir + "/icons";
            if (QDir(icon_dir).exists()) {
                icon_dirs.append(icon_dir);
            }
        }
        icon_dirs += ":/icons";
        QIcon::setThemeSearchPaths(icon_dirs);
    }
    QIcon::setThemeName("breeze");
#endif

    qmlRegisterType<VrpManager>("VrpManager", 1, 0, "VrpManager");
    qmlRegisterType<DeviceManager>("DeviceManager", 1, 0, "DeviceManager");
    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;

#ifdef MACOS_BUNDLE
    engine.addImportPath(res_dir + "/kirigami");
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
