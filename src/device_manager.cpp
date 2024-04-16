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

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
    , total_space_(0)
    , free_space_(0)
{
    connect(&auto_update_timer_, &QTimer::timeout, this, &DeviceManager::updateSerials);
    connect(this, &DeviceManager::connectedDeviceChanged, this, &DeviceManager::updateDeviceInfo);
}

DeviceManager::~DeviceManager()
{
}

QCoro::Task<bool> DeviceManager::startServer()
{
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"start-server"});
    co_await adb.waitForFinished();
    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to start adb server";
        co_return false;
    } else {
        co_await updateSerials();
        co_return true;
    }
}

QCoro::Task<bool> DeviceManager::restartServer()
{
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"kill-server"});
    co_await adb.waitForFinished();
    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to kill adb server";
        co_return false;
    }

    bool result = co_await startServer();
    co_return result;
}

void DeviceManager::updateDeviceInfo()
{
    updateDeviceName();
    updateSpaceUsage();
    updatedeviceIp();
    updateBatteryLevel();
    updateOculusOsVersion();
    updateOculusVersion();
    updateOculusRuntimeVersion();
    updateAndroidVersion();
    updateAndroidSdkVersion();

    updateAppList();
}

QCoro::Task<void> DeviceManager::updateSerials()
{
    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"devices"});

    if (!co_await adb.waitForFinished()) {
        qWarning() << "Failed to get devices";
        co_return;
    }
    /* EXAMPLE OUTPUT:
        List of devices attached
        263407eb        device
        2B0YC1GJ7G5YS3  device
    */
    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    lines.removeFirst();
    lines.removeAll("");
    QStringList serials;
    for (const QString &line : lines) {
        if (line.contains("\tdevice")) {
            QStringList parts = line.split("\t");
            if (!parts.isEmpty()) {
                serials.append(parts.first());
            }
        }
    }

    if (serials.isEmpty()) {
        disconnectDevice();
        if (!devices_list_.isEmpty()) {
            devices_list_.clear();
            emit devicesListChanged();
        }
        co_return;
    }

    if (hasConnectedDevice() && serials.contains(connectedDevice())) {
        QStringList new_devices;
        for (auto &s : serials) {
            if (!devices_list_.contains(s)) {
                new_devices.append(s);
            }
        }

        QStringList removed_devices;
        for (auto &s : devices_list_) {
            if (!serials.contains(s)) {
                removed_devices.append(s);
            }
        }

        if (new_devices.isEmpty() && removed_devices.isEmpty()) {
            co_return;
        }

        for (auto &s : removed_devices) {
            devices_list_.removeAll(s);
        }

        devices_list_.removeAll(connectedDevice());
        devices_list_.prepend(connectedDevice());

        devices_list_.append(new_devices);
        emit devicesListChanged();

    } else {
        // The device is disconnected or there is no connected device, connect
        // to the first device
        devices_list_ = serials;
        connectToDevice(devices_list_.first());
        emit devicesListChanged();
    }
}

QCoro::Task<void> DeviceManager::updateDeviceName()
{
    if (!hasConnectedDevice()) {
        setDeviceName("");
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.product.model"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get model for device" << serial;
        co_return;
    } else {
        QString output = basic_process.readAllStandardOutput();
        output.replace("\n", "");

        setDeviceName(output);
        co_return;
    }
}

QCoro::Task<void> DeviceManager::updatedeviceIp()
{
    if (!hasConnectedDevice()) {
        setdeviceIp("");
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "ip", "route"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get IP for device" << serial;
        co_return;
    } else {
        QString output = basic_process.readAllStandardOutput();
        QStringList lines = output.split("\n");

        for (const QString &line : lines) {
            /* EXAMPLE OUTPUT:
                10.15.233.8/30 dev rmnet_data2 proto kernel scope link
               src 10.15.233.9 192.168.50.0/24 dev wlan1 proto kernel scope link
               src 192.168.50.16
            */

            if (line.contains("wlan") && line.contains("src")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                for (int i = 0; i < parts.size(); i++) {
                    if (parts[i] == "src") {
                        setdeviceIp(parts[i + 1]);
                        co_return;
                    }
                }
            }
        }

        co_return;
    }
}

QCoro::Task<void> DeviceManager::updateSpaceUsage()
{
    if (!hasConnectedDevice()) {
        setSpaceUsage(0, 0);
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "df", "/sdcard"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get space usage for device" << serial;
        co_return;

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
            co_return;
        }

        QString line = lines.first();
        QStringList parts = line.split(QRegularExpression("\\s+"));
        if (parts.size() < 4) {
            qWarning() << "Failed to get space usage for device" << serial;
            co_return;
        }
        long long total_space = parts[1].toLongLong();
        long long free_space = parts[3].toLongLong();
        setSpaceUsage(total_space, free_space);
        co_return;
    }
}

QCoro::Task<void> DeviceManager::updateBatteryLevel()
{
    if (!hasConnectedDevice()) {
        setBatteryLevel(0);
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "dumpsys", "battery"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get battery level for device" << serial;
        co_return;

    } else {
        /* EXAMPLE OUTPUT:
            Current Battery Service state:
                AC powered: false
                USB powered: true
                Wireless powered: false
                Max charging current: 500000
                Max charging voltage: 5000000
                Charge counter: 1880000
                status: 2
                health: 2
                present: true
                level: 47
                scale: 100
                voltage: 3874
                temperature: 323
                technology: Li-ion

        */
        QString output = basic_process.readAllStandardOutput();
        QStringList lines = output.split("\n");
        lines.removeAll("");

        if (lines.isEmpty()) {
            qWarning() << "Failed to get battery level for device" << serial;
            co_return;
        }

        for (const QString &line : lines) {
            if (line.contains("level:")) {
                QStringList parts = line.split(QRegularExpression(":"));
                if (parts.size() < 2) {
                    qWarning() << "Failed to get battery level for device" << serial;
                    co_return;
                }
                int battery_level = parts[1].toInt();
                setBatteryLevel(battery_level);
                co_return;
            }
        }
        qWarning() << "Failed to get battery level for device" << serial;
        co_return;
    }
}

QCoro::Task<void> DeviceManager::updateOculusOsVersion()
{
    if (!hasConnectedDevice()) {
        setOculusOsVersion("");
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.build.display.id"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get Oculus OS version for device" << serial;
        co_return;
    }

    QString output = basic_process.readAllStandardOutput();
    output.replace("\n", "");
    setOculusOsVersion(output);
}

QCoro::Task<void> DeviceManager::updateOculusVersion()
{
    if (!hasConnectedDevice()) {
        setOculusVersion("");
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "dumpsys", "package", "com.oculus.systemux"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get Oculus version for device" << serial;
        co_return;
    }

    /* EXAMPLE OUTPUT:
        ...
        versionName=64.0.0.484.370
        ...
        versionCode=585752815 minSdk=32 targetSdk=32
        ...
    */

    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");

    QString version_name;
    for (const QString &line : lines) {
        if (line.contains("versionName=")) {
            QRegularExpression re("versionName=(\\S+)");
            auto match = re.match(line);
            if (match.hasMatch()) {
                version_name = match.captured(1);
                setOculusVersion(version_name);
                co_return;
            }
        }
    }

    setOculusVersion("");
}

QCoro::Task<void> DeviceManager::updateOculusRuntimeVersion()
{
    if (!hasConnectedDevice()) {
        setOculusRuntimeVersion("");
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "dumpsys", "package", "com.oculus.vrshell"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get Oculus runtime version for device" << serial;
        co_return;
    }

    /* EXAMPLE OUTPUT:
        ...
        versionName=64.0.0.480.368
        ...
        versionCode=585752889 minSdk=29 targetSdk=29
        ...
    */

    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");

    QString version_name;
    for (const QString &line : lines) {
        if (line.contains("versionName=")) {
            QRegularExpression re("versionName=(\\S+)");
            auto match = re.match(line);
            if (match.hasMatch()) {
                version_name = match.captured(1);
                setOculusRuntimeVersion(version_name);
                co_return;
            }
        }
    }

    setOculusRuntimeVersion("");
}

QCoro::Task<void> DeviceManager::updateAndroidVersion()
{
    if (!hasConnectedDevice()) {
        setAndroidVersion(-1);
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.build.version.release"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get Android version for device" << serial;
        co_return;
    }

    QString output = basic_process.readAllStandardOutput();
    output.replace("\n", "");
    setAndroidVersion(output.toInt());
}

QCoro::Task<void> DeviceManager::updateAndroidSdkVersion()
{
    if (!hasConnectedDevice()) {
        setAndroidSdkVersion(-1);
        co_return;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.build.version.sdk"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get Android SDK version for device" << serial;
        co_return;
    }

    QString output = basic_process.readAllStandardOutput();
    output.replace("\n", "");
    setAndroidSdkVersion(output.toInt());
}

QCoro::Task<void> DeviceManager::updateAppList()
{
    if (!hasConnectedDevice()) {
        app_list_model_.clear();
        emit appListChanged();
        co_return;
    }
    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"-s", serial, "shell", "pm", "list", "packages", "--show-versioncode", "-3"});

    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to get apps for device" << serial;
        co_return;
    }

    /* EXAMPLE OUTPUT:
        package:com.facebook.arvr.quillplayer versionCode:135
        package:com.oculus.mobile_mrc_setup versionCode:1637173263
    */
    QString output = basic_process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    lines.removeAll("");
    QRegularExpression re("package:(\\S+) versionCode:(\\d+)");

    app_list_model_.clear();
    for (const QString &line : lines) {
        auto match = re.match(line);
        if (match.hasMatch()) {
            QString package_name = match.captured(1);
            QString version_code = match.captured(2);
            auto app = GameInfo{.package_name = package_name, .version_code = version_code};

            app_list_model_.append(app);
        }
    }
    emit appListChanged();
    co_return;
}

QCoro::Task<bool> DeviceManager::installApk(const QString path, const QString package_name)
{
    if (!hasConnectedDevice()) {
        co_return false;
    }
    auto serial = connectedDevice();

    QDir apk_dir(path);

    if (!apk_dir.exists()) {
        qWarning() << path << " does not exist";
        co_return false;
    }

    QStringList apk_files = apk_dir.entryList(QStringList() << "*.apk", QDir::Files);

    if (apk_files.isEmpty()) {
        qWarning() << "No apk file found in" << path;
        co_return false;
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    for (const QString &apk_file : apk_files) {
        QString apk_path = path + "/" + apk_file;
        qDebug() << "Installing" << apk_path << "on device" << serial;
        adb.start("adb", {"-s", serial, "install", "-r", apk_path});
        co_await adb.waitForFinished(-1);

        if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
            qWarning() << "Failed to install" << apk_path << "on device" << serial;
            qWarning() << basic_process.readAllStandardError();
            co_return false;
        }
    }

    QString obb_path = path + "/" + package_name;
    QDir obb_dir(obb_path);
    if (!package_name.isEmpty() && obb_dir.exists()) {
        qDebug() << "Pushing obb file for" << package_name << "to device" << serial;
        adb.start("adb", {"-s", serial, "shell", "rm", "-rf", "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished(-1);
        adb.start("adb", {"-s", serial, "shell", "mkdir", "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished(-1);

        QStringList obb_files = obb_dir.entryList(QStringList() << "*", QDir::Files);
        for (const QString &obb_file : obb_files) {
            qDebug() << "Pushing" << obb_file << "to device" << serial;
            adb.start("adb", {"-s", serial, "push", obb_path + "/" + obb_file, "/sdcard/Android/obb/" + package_name + "/"});
            co_await adb.waitForFinished(-1);
            if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
                qWarning() << "Failed to push obb file for" << package_name << "on device" << serial;
                qWarning() << basic_process.readAllStandardError();
                co_return false;
            }
        }
    }

    updateDeviceInfo();

    // TODO: support for install.txt
    //  https://vrpirates.wiki/en/Howto/Manual-Sideloading

    co_return true;
}

QCoro::Task<bool> DeviceManager::uninstallApk(const QString package_name)
{
    if (!hasConnectedDevice()) {
        co_return false;
    }

    auto serial = connectedDevice();

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"-s", serial, "uninstall", package_name});
    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to uninstall" << package_name << "on device" << serial;
        qWarning() << basic_process.readAllStandardError();
        co_return false;
    }

    updateDeviceInfo();
    co_return true;
}

QCoro::Task<bool> DeviceManager::connectToWirelessDevice(const QString address /*host[:port]*/)
{
    if (address.isEmpty()) {
        qWarning() << "Empty address";
        co_return false;
    }

    QProcess basic_process;
    auto adb = qCoro(basic_process);
    adb.start("adb", {"connect", address});
    co_await adb.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning() << "Failed to connect to" << address;
        qWarning() << basic_process.readAllStandardError();
        co_return false;
    }

    auto output = basic_process.readAllStandardOutput();
    QByteArray success_str = "connected to " + address.toLocal8Bit();
    if (!output.startsWith(success_str)) {
        qWarning() << "Failed to connect to" << address;
        qWarning() << output;
        co_return false;
    }

    connectToDevice(address);
    co_await updateSerials();
    co_return true;
}
