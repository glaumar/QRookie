#include "device_manager.h"

#include <QCoroTask>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSharedPointer>

DeviceManager::DeviceManager(QObject* parent) : QObject(parent) {
    connect(&update_serials_timer_, &QTimer::timeout, this,
            &DeviceManager::updateSerials);
    autoUpdateSerials();  // TODO: delete this line
    startServer();
}

DeviceManager::~DeviceManager() {}

QCoro::Task<bool> DeviceManager::startServer() {
    QProcess basicProcess;
    auto adb = qCoro(basicProcess);
    adb.start("adb", {"start-server"});
    co_await adb.waitForFinished();
    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to start adb server";
        co_return false;
    } else {
        co_await updateSerials();
        co_return true;
    }
}

QCoro::Task<bool> DeviceManager::restartServer() {
    QProcess basicProcess;
    auto adb = qCoro(basicProcess);
    adb.start("adb", {"kill-server"});
    co_await adb.waitForFinished();
    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to kill adb server";
        co_return false;
    }

    bool result = co_await startServer();
    co_return result;
}

QVariantList DeviceManager::serials_property() const {
    QVariantList list;
    for (const auto& serial : serials_) {
        list.append(QVariant::fromValue(serial));
    }
    return list;
}

QCoro::Task<void> DeviceManager::updateSerials() {
    QProcess basicProcess;
    auto adb = qCoro(basicProcess);
    adb.start("adb", {"devices"});
    co_await adb.waitForFinished();
    /* EXAMPLE OUTPUT:
        List of devices attached
        263407eb        device
        2B0YC1GJ7G5YS3  device
    */
    QString output = basicProcess.readAllStandardOutput();
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
        bool r =
            co_await installApk("2G0YC1ZF7G0GS3",
                                "/home/glaumar/.local/share/QRookie/Death "
                                "Horizon- Reloaded (MR-Fix) v175+0.9.5.1 -VRP/",
                                "com.mrf.dreamdev.deathhorizon.quest");
        qDebug() << "install result:" << r;
        emit serialsChanged();
    }
}

QCoro::Task<QVector<QPair<QString, long>>> DeviceManager::deviceApps(
    QString serial) {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QVector<QPair<QString, long>>();
    }

    QProcess basicProcess;
    auto adb = qCoro(basicProcess);
    adb.start("adb", {"-s", serial, "shell", "pm", "list", "packages",
                      "--show-versioncode", "-3"});

    co_await adb.waitForFinished();

    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to get apps for device" << serial;
        co_return QVector<QPair<QString, long>>();
    }

    /* EXAMPLE OUTPUT:
        package:com.facebook.arvr.quillplayer versionCode:135
        package:com.oculus.mobile_mrc_setup versionCode:1637173263
    */
    QString output = basicProcess.readAllStandardOutput();
    QStringList lines = output.split("\n");
    lines.removeAll("");
    QRegularExpression re("package:(\\S+) versionCode:(\\d+)");
    QVector<QPair<QString, long>> apps;
    for (const QString& line : lines) {
        auto match = re.match(line);
        if (match.hasMatch()) {
            QString package_name = match.captured(1);
            QString version_code = match.captured(2);
            apps.append({package_name, version_code.toLong()});
        }
    }
    co_return apps;
}

QCoro::Task<QString> DeviceManager::deviceModel(QString serial) {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QString("");
    }

    QProcess basicProcess;
    auto adb = qCoro(basicProcess);

    adb.start("adb", {"-s", serial, "shell", "getprop", "ro.product.model"});

    co_await adb.waitForFinished();

    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to get model for device" << serial;
        co_return QString("");
    } else {
        QString output = basicProcess.readAllStandardOutput();
        output.replace("\n", "");
        co_return output;
    }
}

QCoro::Task<QPair<long, long>> DeviceManager::spaceUsage(QString serial) {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return QPair<long, long>(0, 0);
    }

    QProcess basicProcess;
    auto adb = qCoro(basicProcess);

    adb.start("adb", {"-s", serial, "shell", "df", "/sdcard"});

    co_await adb.waitForFinished();

    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to get space usage for device" << serial;
        co_return QPair<long, long>(0, 0);
    } else {
        /* EXAMPLE OUTPUT:
            Filesystem     1K-blocks     Used Available Use% Mounted on
            /dev/fuse      107584204 83476996  23959752  78% /storage/emulated
        */
        QString output = basicProcess.readAllStandardOutput();
        QStringList lines = output.split("\n");
        lines.removeFirst();
        lines.removeAll("");

        if (lines.isEmpty()) {
            qWarning() << "Failed to get space usage for device" << serial;
            co_return QPair<long, long>(0, 0);
        }

        QString line = lines.first();
        QStringList parts = line.split(QRegularExpression("\\s+"));
        if (parts.size() < 4) {
            qWarning() << "Failed to get space usage for device" << serial;
            co_return QPair<long, long>(0, 0);
        }
        long total_space = parts[1].toLong();
        long free_space = parts[3].toLong();
        co_return QPair<long, long>(total_space, free_space);
    }
}

QCoro::Task<bool> DeviceManager::installApk(QString serial, QString path,
                                            QString package_name) {
    if (serials_.isEmpty() || !serials_.contains(serial)) {
        co_return false;
    }

    QProcess basicProcess;

    QString apk_path = path + "/" + package_name + ".apk";
    if (!QFile::exists(apk_path)) {
        qWarning() << "APK file" << apk_path << "does not exist";
        co_return false;
    }
    auto adb = qCoro(basicProcess);
    adb.start("adb", {"-s", serial, "install", "-r", apk_path});
    co_await adb.waitForFinished();

    if (basicProcess.exitStatus() != QProcess::NormalExit ||
        basicProcess.exitCode() != 0) {
        qWarning() << "Failed to install" << apk_path << "on device" << serial;
        qWarning() << basicProcess.readAllStandardError();
        co_return false;
    }

    if (QFile::exists(path + "/" + package_name)) {
        adb.start("adb", {"-s", serial, "shell", "rm", "-rf",
                          "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished();
        adb.start("adb", {"-s", serial, "shell", "mkdir",
                          "/sdcard/Android/obb/" + package_name});
        co_await adb.waitForFinished();
        adb.start("adb", {"-s", serial, "push", path + "/" + package_name + "/",
                          "/sdcard/Android/obb/"});
        co_await adb.waitForFinished();
        if (basicProcess.exitStatus() != QProcess::NormalExit ||
            basicProcess.exitCode() != 0) {
            qWarning() << "Failed to push obb file for" << package_name
                       << "on device" << serial;
            qWarning() << basicProcess.readAllStandardError();
            co_return false;
        }
    }

    co_return true;
}