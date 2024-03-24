#include "vrp_downloader.h"

#include <QCoroTimer>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSharedPointer>
#include <QStandardPaths>

#include "qrookie.h"

VrpDownloader::VrpDownloader(QObject* parent) : QObject(parent) {
    // Create cache and data directories
    QDir dir;
    cache_path_ =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    data_path_ =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dir.exists(cache_path_)) {
        dir.mkpath(cache_path_);
    }
    if (!dir.exists(data_path_)) {
        dir.mkpath(data_path_);
    }

    http_downloader_.setDownloadDirectory(cache_path_);

    connect(this, &VrpDownloader::localQueueChanged,
            [this]() { saveLocalQueue(); });

    // DeviceManager
    connect(&device_manager_, &DeviceManager::serialsChanged, [this]() {
        auto devices = device_manager_.serials();

        if (devices.isEmpty()) {
            disconnectDevice();
        } else if (!devices.contains(connectedDevice())) {
            connectToDevice(devices.first());
        }
        emit deviceListChanged();
    });

    connect(this, &VrpDownloader::connectedDeviceChanged,
            [this]() -> QCoro::Task<void> {
                if (!hasConnectedDevice()) {
                    device_model_.clear();
                    total_space_ = 0;
                    free_space_ = 0;
                } else {
                    device_model_ =
                        co_await device_manager_.deviceModel(connectedDevice());
                    auto usage =
                        co_await device_manager_.spaceUsage(connectedDevice());
                    total_space_ = usage.first;
                    free_space_ = usage.second;
                }
                emit deviceModelChanged();
                emit spaceUsageChanged();
                updateInstalledQueue();
            });
    device_manager_.autoUpdateSerials();

    loadLocalQueue();

    // parese local metadata
    parseMetadata();
}

VrpDownloader::~VrpDownloader() {
    saveLocalQueue();
    // TODO: cleanup cache
}

VrpDownloader::Status VrpDownloader::getStatus(const GameInfo& game) {
    if (isFailed(game)) {
        return Status::Error;
    } else if (isInstalled(game)) {
        return Status::Installed;
    } else if (isInstalling(game)) {
        return Status::Installing;
    } else if (isUpdatableLocally(game)) {
        return Status::UpdatableLocally;
    } else if (isInstallable(game)) {
        return Status::Installable;
    } else if (isLocal(game)) {
        return Status::Local;
    } else if (isDecompressing(game)) {
        return Status::Decompressing;
    } else if (isDownloading(game)) {
        return Status::Downloading;
    } else if (isQueued(game)) {
        return Status::Queued;
    } else if (isUpdatableRemotely(game)) {
        return Status::UpdatableRemotely;
    } else if (isDownloadable(game)) {
        return Status::Downloadable;
    } else {
        return Status::Unknown;
    }
}

QCoro::Task<bool> VrpDownloader::updateMetadata() {
    if (!co_await vrp_public_.update()) {
        qWarning() << "Update metadata failed";
        co_return false;
    }

    if (!co_await downloadMetadata()) {
        qWarning() << "Update metadata failed";
        co_return false;
    }

    if (parseMetadata()) {
        qDebug() << "Update metadata successful";
        http_downloader_.setBaseUrl(vrp_public_.baseUrl());
        co_return true;
    } else {
        qWarning() << "Update metadata failed";
        co_return false;
    }
}

QCoro::Task<bool> VrpDownloader::downloadMetadata() {
    if (co_await http_downloader_.download("meta.7z")) {
        QProcess basic_process;
        auto p7za = qCoro(basic_process);

        // Decompress meta.7z
        p7za.start(
            "7za",
            QStringList()
                << "x"
                << QString("%1/meta.7z")
                       .arg(http_downloader_.downloadDirectory())
                << "-aoa"  // Overwrite All existing files without prompt.
                << QString("-o%1").arg(data_path_)
                << QString("-p%1").arg(vrp_public_.password()));

        co_await p7za.waitForFinished();

        if (basic_process.exitStatus() != QProcess::NormalExit ||
            basic_process.exitCode() != 0) {
            qWarning("meta.7z decompression failed: %s\n %s",
                     basic_process.readAllStandardOutput().data(),
                     basic_process.readAllStandardError().data());
            co_return false;
        } else {
            qDebug() << "meta.7z decompression successful";
            co_return true;
        }
    } else {
        qWarning() << "Download Metadata failed";
        co_return false;
    }
}

bool VrpDownloader::parseMetadata() {
    QFile file(data_path_ + "/VRP-GameList.txt");
    if (!file.exists()) {
        qWarning() << "VRP-GameList.txt not found";
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "VRP-GameList.txt open failed";
        return false;
    }

    QTextStream in(&file);

    // Discard the first line
    in.readLine();

    games_info_.clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(';');
        if (parts.size() != 6) {
            qDebug() << "Invalid line in VRP-GameList.txt: " << line;
            continue;
        } else {
            GameInfo game_info;
            game_info.name = parts[0];
            game_info.release_name = parts[1];
            game_info.package_name = parts[2];
            game_info.version_code = parts[3];
            game_info.last_updated = parts[4];
            game_info.size = parts[5];

            games_info_.append(game_info);
        }
    }

    if (games_info_.isEmpty()) {
        qWarning() << "No games found in VRP-GameList.txt";
        return false;
    } else {
        qDebug() << "Metadata parsed successfully";
        emit gamesInfoChanged();
        return true;
    }
}

QString VrpDownloader::getGameId(const QString& release_name) const {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData((release_name + "\n").toUtf8());
    return hash.result().toHex();
}

QString VrpDownloader::getLocalGamePath(const QString& release_name) const {
    // TODO: check path exists
    return data_path_ + "/" + release_name;
}

bool VrpDownloader::addToDownloadQueue(const GameInfo game, bool auto_install) {
    // if (downloading_queue_.contains(game) ||
    //     decompressing_queue_.contains(game)) {
    //     qDebug() << "Already in queue: " << game.release_name;
    //     return false;
    // }

    // if (local_queue_.contains(game)) {
    //     qDebug() << "Already downloaded: " << game.release_name;
    //     emit statusChanged(game.release_name, getStatus(game));
    //     return false;
    // }

    if (isFailed(game)) {
        qDebug() << "Try again: " << game.release_name;
        failed_queue_.removeAll(game);
    }

    if (auto_install) {
        auto_install_queue_.append(game);
    }

    downloading_queue_.append(game);
    emit downloadsQueueChanged();
    qDebug() << "Queued: " << game.release_name;
    emit statusChanged(game.release_name, Status::Queued);

    if (downloading_queue_.size() == 1) downloadQueuedGames();
    return true;
}

QCoro::Task<bool> VrpDownloader::install(const GameInfo game) {
    if (!hasConnectedDevice()) {
        co_return false;
    }
    qDebug() << "Installing: " << game.release_name;
    installing_queue_.append(game);
    emit statusChanged(game.release_name, Status::Installing);
    bool result = co_await device_manager_.installApk(
        connectedDevice(), getLocalGamePath(game.release_name),
        game.package_name);

    if (result) {
        qDebug() << "Install finished: " << game.release_name;
        emit statusChanged(game.release_name, Status::Installed);
        updateInstalledQueue();
    } else {
        qDebug() << "Install failed: " << game.release_name;
        failed_queue_.removeAll(game);
        failed_queue_.append(game);
        emit statusChanged(game.release_name, Status::Error);
    }

    installing_queue_.removeAll(game);
    co_return result;
}

QCoro::Task<void> VrpDownloader::downloadQueuedGames() {
    if (downloading_queue_.isEmpty()) {
        qDebug() << "Queue is empty";
        co_return;
    }

    while (!downloading_queue_.isEmpty()) {
        auto game = downloading_queue_.first();

        qDebug() << "Downloading: " << game.release_name;
        emit statusChanged(game.release_name, Status::Downloading);

        QString id = getGameId(game.release_name);

        auto conn = connect(
            &http_downloader_, &HttpDownloader::downloadProgressDir, this,
            [this, id, game](QString dir_name, qint64 bytes_received,
                             qint64 bytes_total) {
                if (dir_name == id) {
                    emit downloadProgressChanged(
                        game.release_name,
                        double(bytes_received) / double(bytes_total));
                }
            });

        if (co_await http_downloader_.downloadDir(id)) {
            disconnect(conn);
            qDebug() << "Download finished: " << game.release_name;
            downloading_queue_.removeAll(game);
            decompressGame(game);
        } else {
            qDebug() << "Download failed" << game.release_name;
            downloading_queue_.removeAll(game);
            // emit downloadsQueueChanged();
            failed_queue_.removeAll(game);
            failed_queue_.append(game);
            emit statusChanged(game.release_name, Status::Error);
            qWarning() << "Download game failed :" << game.release_name;
        }
    }
    co_return;
}

QCoro::Task<bool> VrpDownloader::decompressGame(const GameInfo game) {
    if (decompressing_queue_.contains(game)) {
        qDebug() << "Already in decompressing queue: " << game.release_name;
        co_return false;
    }

    decompressing_queue_.append(game);

    qDebug() << "Decompressing: " << game.release_name;
    emit statusChanged(game.release_name, Status::Decompressing);

    QProcess basic_process;
    auto p7za = qCoro(basic_process);
    // Decompress
    p7za.start("7za",
               QStringList()
                   << "x"
                   << QString("%1/%2/%2.7z.001")
                          .arg(cache_path_, getGameId(game.release_name))
                   << "-aoa"  // Overwrite All existing files without prompt.
                   << QString("-o%1").arg(data_path_)
                   << QString("-p%1").arg(vrp_public_.password()));

    co_await p7za.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit ||
        basic_process.exitCode() != 0) {
        qWarning("Error: %s\n %s", basic_process.readAllStandardOutput().data(),
                 basic_process.readAllStandardError().data());
        qDebug() << "Decompression failed: " << game.release_name;
        decompressing_queue_.removeAll(game);
        failed_queue_.removeAll(game);
        failed_queue_.append(game);
        // emit downloadsQueueChanged();
        emit statusChanged(game.release_name, Status::Error);
        co_return false;
    } else {
        qDebug() << "Decompression finished: " << game.release_name;
        decompressing_queue_.removeAll(game);

        if (local_queue_.contains(game)) {
            local_queue_.removeAll(game);
        }
        local_queue_.push_front(game);

        emit downloadsQueueChanged();
        emit localQueueChanged();
        emit statusChanged(game.release_name, getStatus(game));

        if (auto_install_queue_.contains(game)) {
            auto_install_queue_.removeAll(game);

            if (hasConnectedDevice()) {
                install(game);
            }
        }

        co_return true;
    }
}

QVariantList VrpDownloader::find(const QString& package_name) {
    /*
    Usually the GameInfo objects with the same package_name
    may have different other information(name,release_name,version_code), but
    they are adjacent in the VRP-GameList.txt file.
    */
    QVariantList result;
    auto it = games_info_.begin();
    // Find the first GameInfo with the package_name
    while (it != games_info_.end() && it->package_name != package_name) {
        it++;
    }

    // find all adjacent GameInfo with package_name
    while (it != games_info_.end() && it->package_name == package_name) {
        result.append(QVariant::fromValue(*it));
        it++;
    }

    return result;
}

QString VrpDownloader::getGameThumbnailPath(const QString& package_name) {
    auto path = data_path_ + "/.meta/thumbnails/" + package_name + ".jpg";
    if (QFile::exists(path)) {
        return path;
    } else {
        return "";
    }
}

bool VrpDownloader::saveLocalQueue() {
    QJsonArray jsonArray;
    for (const auto& game : local_queue_) {
        QJsonObject jsonObject;
        jsonObject["name"] = game.name;
        jsonObject["release_name"] = game.release_name;
        jsonObject["package_name"] = game.package_name;
        jsonObject["version_code"] = game.version_code;
        jsonObject["last_updated"] = game.last_updated;
        jsonObject["size"] = game.size;
        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDoc(jsonArray);
    QFile file(data_path_ + "/local_queue.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        return true;
    } else {
        qWarning("saveLocalQueue: Failed to open file for writing.");
        return false;
    }
}

bool VrpDownloader::loadLocalQueue() {
    QFile file(data_path_ + "/local_queue.json");
    if (!file.exists()) {
        qWarning() << "local_queue.json not found";
        return false;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonArray jsonArray = jsonDoc.array();
        for (const auto& value : jsonArray) {
            GameInfo game;
            game.name = value.toObject()["name"].toString();
            game.release_name = value.toObject()["release_name"].toString();
            game.package_name = value.toObject()["package_name"].toString();
            game.version_code = value.toObject()["version_code"].toString();
            game.last_updated = value.toObject()["last_updated"].toString();
            game.size = value.toObject()["size"].toString();
            local_queue_.append(game);
        }
        return true;
    } else {
        qWarning("loadLocalQueue: Failed to open file for reading.");
        return false;
    }
}

QCoro::Task<void> VrpDownloader::updateInstalledQueue() {
    if (!hasConnectedDevice()) {
        installed_queue_.clear();
        emit installedQueueChanged();
        co_return;
    }
    auto apps = co_await device_manager_.installedApps(connectedDevice());
    if (apps != installed_queue_) {
        installed_queue_ = apps;
        emit installedQueueChanged();
    }
    co_return;
}

QVariantList VrpDownloader::gamesInfo() const {
    QVariantList list;
    for (const auto& game_info : games_info_) {
        auto name = game_info.name;
        if (filter_.isEmpty() ||
            name.remove(" ").contains(filter_, Qt::CaseInsensitive)) {
            list.append(QVariant::fromValue(game_info));
        }
    }
    return list;
}

QVariantList VrpDownloader::downloadsQueue() const {
    QVariantList list;
    for (const auto& game_info : decompressing_queue_) {
        list.append(QVariant::fromValue(game_info));
    }

    for (const auto& game_info : downloading_queue_) {
        list.append(QVariant::fromValue(game_info));
    }

    // for (const auto& game_info : failed_queue_) {
    //     list.append(QVariant::fromValue(game_info));
    // }

    return list;
}

QVariantList VrpDownloader::localQueue() const {
    QVariantList list;
    for (const auto& game_info : local_queue_) {
        list.append(QVariant::fromValue(game_info));
    }

    return list;
}

bool VrpDownloader::removeFromLocalQueue(const GameInfo& game) {
    local_queue_.removeAll(game);
    saveLocalQueue();
    emit localQueueChanged();

    auto game_dir = getLocalGamePath(game.release_name);

    QDir dir(game_dir);
    if (dir.exists()) {
        return dir.removeRecursively();
    }
    return false;
}

QVariantList VrpDownloader::installedQueue() const {
    QVariantList list;
    for (const auto& app_info : installed_queue_) {
        list.append(QVariant::fromValue(app_info));
    }

    return list;
}

QVariantList VrpDownloader::deviceList() const {
    auto devices = device_manager_.serials();
    QVariantList list;
    for (const auto& device : devices) {
        list.append(QVariant::fromValue(device));
    }
    return list;
}