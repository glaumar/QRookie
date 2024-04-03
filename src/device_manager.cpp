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

#include "device_manager.h"

#include <QCoroTask>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSharedPointer>

DeviceManager::DeviceManager(QObject* parent) : QObject(parent) {
    connect(&update_serials_timer_, &QTimer::timeout, this,
            &DeviceManager::updateSerials);
    restartServer();
}

DeviceManager::~DeviceManager() {}

QCoro::Task<bool> DeviceManager::startServer() {
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"start-server"});
    co_await adb.waitForFinished();
    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to start adb server";
        co_return false;
    } else {
        co_await updateSerials();
        co_return true;
    }
}

QCoro::Task<bool> DeviceManager::restartServer() {
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"kill-server"});
    co_await adb.waitForFinished();
    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to kill adb server";
        co_return false;
    }

    bool result = co_await startServer();
    co_return result;
}

QCoro::Task<void> DeviceManager::updateSerials() {
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"devices"});
    co_await adb.waitForFinished();
    /* EXAMPLE OUTPUT:
        List of devices attached
        263407eb        device
        2B0YC1GJ7G5YS3  device
    */
    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    lines.removeFirst();
    lines.removeAll("");
    QVector<QString> serials;
    for (const QString& line : lines) {
        if (line.contains("\tdevice")) {
            QStringList parts = line.split("\t");
            if (!parts.isEmpty()) {
                serials.append(parts.first());
            }
        }
    }

    if (serials_ != serials) {
        serials_ = serials;
        emit serialsChanged();
    }
}

QCoro::Task<QVector<AppInfo>> DeviceManager::installedApps(
    const QString& serial) const {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QVector<AppInfo>();
    }
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"-s", serial, "shell", "pm", "list", "packages",
                      "--show-versioncode", "-3"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to get apps for device" << serial;
        co_return QVector<AppInfo>();
    }

    /* EXAMPLE OUTPUT:
        package:com.facebook.arvr.quillplayer versionCode:135
        package:com.oculus.mobile_mrc_setup versionCode:1637173263
    */
    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    lines.removeAll("");
    QRegularExpression re("package:(\\S+) versionCode:(\\d+)");
    QVector<AppInfo> apps;
    for (const QString& line : lines) {
        auto match = re.match(line);
        if (match.hasMatch()) {
            QString package_name = match.captured(1);
            QString version_code = match.captured(2);
            apps.append({package_name, version_code.toLongLong()});
        }
    }
    co_return apps;
}

QCoro::Task<QString> DeviceManager::deviceModel(const QString& serial) const {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QString("");
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.product.model"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to get model for device" << serial;
        co_return QString("");
    } else {
        QString output = basic_process.readAllStandardOutput();
        output.replace("\n", "");
        co_return output;
    }
}

QCoro::Task<QPair<long long, long long>> DeviceManager::spaceUsage(
    const QString& serial) const {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QPair<long long, long long>(0, 0);
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "df", "/sdcard"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to get space usage for device" << serial;
        co_return QPair<long long, long long>(0, 0);

    } else {
        /* EXAMPLE OUTPUT:
            Filesystem     1K-blocks     Used Available Use% Mounted on
            /dev/fuse      107584204 83476996  23959752  78% /storage/emulated
        */
        QString output = basic_process.readAllStandardOutput();
        QStringList lines = output.split("\n");
        lines.removeFirst();
        lines.removeAll("");

        if (lines.isEmpty()) {
            qWarning() << "Failed to get space usage for device" << serial;
            co_return QPair<long long, long long>(0, 0);
        }

        QString line = lines.first();
        QStringList parts = line.split(QRegularExpression("\\s+"));
        if (parts.size() < 4) {
            qWarning() << "Failed to get space usage for device" << serial;
            co_return QPair<long long, long long>(0, 0);
        }
        long long total_space = parts[1].toLongLong();
        long long free_space = parts[3].toLongLong();
        co_return QPair<long long, long long>(total_space, free_space);
        ;
    }
}

QCoro::Task<bool> DeviceManager::installApk(const QString serial,
                                            const QString path,
                                            const QString package_name) const {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return false;
    }

    QDir apk_dir(path);

    if (!apk_dir.exists()) {
        qWarning() << path << " does not exist";
        co_return false;
    }

    QStringList apk_files =
        apk_dir.entryList(QStringList() << "*.apk", QDir::Files);

    if (apk_files.isEmpty()) {
        qWarning() << "No apk file found in" << path;
        co_return false;
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    for (const QString& apk_file : apk_files) {
        QString apk_path = path + "/" + apk_file;
        qDebug() << "Installing" << apk_path << "on device" << serial;
        adb.start("adb", {"-s", serial, "install", "-r", apk_path});
        co_await adb.waitForFinished();

        if (basic_process.exitStatus() != QProcess::NormalExit ||
            basic_process.exitCode() != 0) {
            qWarning() << "Failed to install" << apk_path << "on device"
                       << serial;
            qWarning() << basic_process.readAllStandardError();
            co_return false;
        }
    }

    QString obb_path = path + "/" + package_name;
    QDir obb_dir(obb_path);
    if (!package_name.isEmpty() && obb_dir.exists()) {
        qDebug() << "Pushing obb file for" << package_name << "on device"
                 << serial;
        adb.start("adb", {"-s", serial, "shell", "rm", "-rf",
                          "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished();
        adb.start("adb", {"-s", serial, "shell", "mkdir",
                          "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished();

        QStringList obb_files =
            obb_dir.entryList(QStringList() << "*", QDir::Files);
        for (const QString& obb_file : obb_files) {
            qDebug() << "Pushing" << obb_file << "to device" << serial;
            adb.start("adb", {"-s", serial, "push", obb_path + "/" + obb_file,
                              "/sdcard/Android/obb/" + package_name + "/"});
            co_await adb.waitForFinished();
        }

        co_await adb.waitForFinished();
        if (basic_process.exitStatus() != QProcess::NormalExit ||
            basic_process.exitCode() != 0) {
            qWarning() << "Failed to push obb file for" << package_name
                       << "on device" << serial;
            qWarning() << basic_process.readAllStandardError();
            co_return false;
        }
    }

    // TODO: support for install.txt
    //  https://vrpirates.wiki/en/Howto/Manual-Sideloading

    co_return true;
}

QCoro::Task<bool> DeviceManager::uninstallApk(
    const QString serial, const QString package_name) const {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return false;
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"-s", serial, "uninstall", package_name});
    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning() << "Failed to uninstall" << package_name << "on device"
                   << serial;
        qWarning() << basic_process.readAllStandardError();
        co_return false;
    }

    co_return true;
}