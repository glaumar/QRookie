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

VrpDownloader::VrpDownloader(QObject* parent)
    : QObject(parent), status_filter_(Status::Unknown) {
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

    loadGamesInfo();
    updateInstalledQueue();

    // Restore download
    if (all_games_.key(Status::Queued) != GameInfo{}) {
        downloadQueuedGames();
    }
}

VrpDownloader::~VrpDownloader() {
    // TODO: cleanup cache
    saveGamesInfo();
}

QVariantList VrpDownloader::gamesInfo() const {
    QVariantList list;

    for (auto it = all_games_.constBegin(); it != all_games_.constEnd(); ++it) {
        QString name = it.key().name;

        if (status_filter_ != Status::Unknown &&
            !(it.value() & status_filter_)) {
            continue;
        }

        if (filter_.isEmpty() ||
            name.remove(" ").contains(filter_, Qt::CaseInsensitive)) {
            list.append(QVariant::fromValue(it.key()));
        }
    }

    return list;
}

QVariantList VrpDownloader::downloadsQueue() const {
    QVariantList list;
    for (const auto& game : downloads_queue_) {
        list.append(QVariant::fromValue(game));
    }
    return list;
}

QVariantList VrpDownloader::localQueue() const {
    QVariantList list;
    for (const auto& game : local_queue_) {
        list.append(QVariant::fromValue(game));
    }
    return list;
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
        QFile::remove(http_downloader_.downloadDirectory() + "/meta.7z");

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

    // Clean up old metadata
    QMutableMapIterator<GameInfo, Status> it(all_games_);
    while (it.hasNext()) {
        it.next();
        if (it.value() == Status::Downloadable) {
            it.remove();
        }
    }

    QTextStream in(&file);

    // Discard the first line
    in.readLine();
    bool is_empty = true;
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

            if (!all_games_.contains(game_info)) {
                all_games_[game_info] = Status::Downloadable;
                is_empty = false;
            }
        }
    }

    if (is_empty) {
        qWarning() << "No games found in VRP-GameList.txt";
        return false;
    } else {
        qDebug() << "Metadata parsed successfully";
        emit gamesInfoChanged();
        // emit localQueueChanged();
        // emit downloadsQueueChanged();
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

QString VrpDownloader::getGameThumbnailPath(const QString& package_name) {
    auto path = data_path_ + "/.meta/thumbnails/" + package_name + ".jpg";
    if (QFile::exists(path)) {
        return path;
    } else {
        return "";
    }
}

bool VrpDownloader::addToDownloadQueue(const GameInfo game) {
    Status s = getStatus(game);
    if (s != Status::Downloadable && s != Status::UpdatableRemotely &&
        s != Status::DownloadError && s != Status::DecompressionError) {
        return false;
    }

    setStatus(game, Status::Queued);
    downloads_queue_.removeAll(game);
    downloads_queue_.append(game);
    emit downloadsQueueChanged();
    qDebug() << "Queued: " << game.release_name;

    // Start download if not already downloading
    if (getDownloadingGame() == GameInfo{}) {
        downloadQueuedGames();
    }

    return true;
}

void VrpDownloader::removeFromDownloadQueue(const GameInfo& game) {
    Status s = getStatus(game);
    if (s == Status::Downloading) {
        setStatus(game, Status::Downloadable);
        http_downloader_.abortDownloadDir(getGameId(game.release_name));
        downloads_queue_.removeAll(game);
        emit downloadsQueueChanged();
    } else {
        setStatus(game, Status::Downloadable);
        downloads_queue_.removeAll(game);
        emit downloadsQueueChanged();
    }
}

bool VrpDownloader::removeFromLocalQueue(const GameInfo& game) {
    setStatus(game, Status::Downloadable);
    local_queue_.removeAll(game);
    emit localQueueChanged();

    auto game_dir = getLocalGamePath(game.release_name);

    QDir dir(game_dir);
    if (dir.exists()) {
        return dir.removeRecursively();
    }
    return false;
}

QCoro::Task<void> VrpDownloader::downloadQueuedGames() {
    GameInfo game = getFirstQueuedGame();
    while (game != GameInfo{}) {
        qDebug() << "Downloading: " << game.release_name;
        setStatus(game, Status::Downloading);

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
            qDebug() << "Download finished: " << game.release_name;
            decompressGame(game);
        } else {
            if (getStatus(game) == Status::Downloading) {
                setStatus(game, Status::DownloadError);
                qWarning() << "Download game failed :" << game.release_name;
            }
        }
        disconnect(conn);
        game = getFirstQueuedGame();
    }
    co_return;
}

QCoro::Task<bool> VrpDownloader::decompressGame(const GameInfo game) {
    if (getStatus(game) == Status::Decompressing) {
        qDebug() << "Already in decompressing queue: " << game.release_name;
        co_return false;
    }

    qDebug() << "Decompressing: " << game.release_name;
    setStatus(game, Status::Decompressing);

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
        setStatus(game, Status::DecompressionError);
        co_return false;
    } else {
        qDebug() << "Decompression finished: " << game.release_name;
        if (hasConnectedDevice()) {
            setStatus(game, Status::Installable);
        } else {
            setStatus(game, Status::Local);
        }
        downloads_queue_.removeAll(game);
        emit downloadsQueueChanged();
        local_queue_.prepend(game);
        emit localQueueChanged();

        if (hasConnectedDevice()) {
            install(game);
        }

        co_return true;
    }
}

QCoro::Task<bool> VrpDownloader::install(const GameInfo game) {
    if (!hasConnectedDevice()) {
        co_return false;
    }
    qDebug() << "Installing: " << game.release_name;
    setStatus(game, Status::Installing);
    bool result = co_await device_manager_.installApk(
        connectedDevice(), getLocalGamePath(game.release_name),
        game.package_name);

    if (result) {
        qDebug() << "Install finished: " << game.release_name;
        setStatus(game, Status::InstalledAndLocally);
        updateInstalledQueue();
    } else {
        qDebug() << "Install failed: " << game.release_name;
        setStatus(game, Status::InstallError);
    }

    co_return result;
}

QCoro::Task<bool> VrpDownloader::uninstall(const QString packege_name) {
    if (!hasConnectedDevice()) {
        co_return false;
    }

    bool result =
        co_await device_manager_.uninstallApk(connectedDevice(), packege_name);
    if (result) {
        updateInstalledQueue();
    }

    co_return result;
}

bool VrpDownloader::saveGamesInfo() {
    QJsonArray jsonArray;
    auto it = all_games_.constBegin();
    auto meta_status = QMetaEnum::fromType<Status>();

    while (it != all_games_.constEnd()) {
        QJsonObject jsonObject;
        const GameInfo& game = it.key();
        Status status = it.value();
        jsonObject["name"] = game.name;
        jsonObject["release_name"] = game.release_name;
        jsonObject["package_name"] = game.package_name;
        jsonObject["version_code"] = game.version_code;
        jsonObject["last_updated"] = game.last_updated;
        jsonObject["size"] = game.size;

        switch (status) {
            case Status::Downloading:
            case Status::DownloadError:
            case Status::Decompressing:
            case Status::DecompressionError:
                status = Status::Queued;
                break;
            case Status::Installable:
            case Status::Installing:
            case Status::InstallError:
            case Status::UpdatableLocally:
            case Status::InstalledAndLocally:
                status = Status::Local;
                break;
            case Status::UpdatableRemotely:
            case Status::InstalledAndRemotely:
                status = Downloadable;
                break;
            case Status::Unknown:
                status = Status::Downloadable;
            default:
                break;
        }

        auto status_key = meta_status.valueToKey(status);
        jsonObject["status"] = QString(status_key);
        jsonArray.append(jsonObject);
        ++it;
    }

    QJsonDocument jsonDoc(jsonArray);
    QFile file(data_path_ + "/games_info.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        return true;
    } else {
        qWarning("save games info: Failed to open file for writing.");
        return false;
    }
}

bool VrpDownloader::loadGamesInfo() {
    QFile file(data_path_ + "/games_info.json");
    if (!file.exists()) {
        qWarning() << "local_queue.json not found";
        return false;
    }

    all_games_.clear();

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
        QJsonArray jsonArray = jsonDoc.array();
        auto meta_status = QMetaEnum::fromType<Status>();
        for (const auto& value : jsonArray) {
            GameInfo game;
            auto obj = value.toObject();
            game.name = obj["name"].toString();
            game.release_name = obj["release_name"].toString();
            game.package_name = obj["package_name"].toString();
            game.version_code = obj["version_code"].toString();
            game.last_updated = obj["last_updated"].toString();
            game.size = value.toObject()["size"].toString();

            int status_int =
                meta_status.keyToValue(obj["status"].toString().toUtf8());

            Status status = status_int >= 0 ? static_cast<Status>(status_int)
                                            : Status::Unknown;
            all_games_[game] = status;
            if (status == Status::Queued || status == Status::Downloading ||
                status == Status::DownloadError ||
                status == Status::Decompressing ||
                status == Status::DecompressionError) {
                downloads_queue_.append(game);
            } else if (status == Status::Local ||
                       status == Status::Installable ||
                       status == Status::InstalledAndLocally ||
                       status == Status::InstallError) {
                local_queue_.append(game);
            }
        }

        emit gamesInfoChanged();
        emit localQueueChanged();
        emit downloadsQueueChanged();
        return true;
    } else {
        qWarning("load games info: Failed to open file for reading.");
        return false;
    }
}

QCoro::Task<void> VrpDownloader::updateInstalledQueue() {
    if (!hasConnectedDevice()) {
        installed_queue_.clear();
        emit installedQueueChanged();

        // Update Status
        QMutableMapIterator<GameInfo, Status> it(all_games_);
        while (it.hasNext()) {
            it.next();

            Status s = it.value();
            if (s == Status::UpdatableRemotely ||
                s == Status::InstalledAndRemotely) {
                it.setValue(Status::Downloadable);
                emit statusChanged(it.key().release_name, Status::Downloadable);
            } else if (s == Status::UpdatableLocally ||
                       s == Status::InstalledAndLocally ||
                       s == Status::Installable) {
                it.setValue(Status::Local);
                emit statusChanged(it.key().release_name, Status::Local);
            }
        }

        co_return;
    }

    auto apps = co_await device_manager_.installedApps(connectedDevice());
    if (apps != installed_queue_) {
        installed_queue_ = apps;
        emit installedQueueChanged();

        QMap<QString, long long> installed_map;
        for (const auto& app : apps) {
            installed_map[app.package_name] = app.version_code;
        }

        // Update game status
        QMutableMapIterator<GameInfo, Status> it(all_games_);
        while (it.hasNext()) {
            it.next();
            Status from_s = it.value();

            if (from_s == Status::UpdatableLocally ||
                from_s == Status::InstalledAndLocally ||
                from_s == Status::Installable) {
                from_s = Status::Local;
            } else if (from_s == Status::UpdatableRemotely ||
                       from_s == Status::InstalledAndRemotely) {
                from_s = Status::Downloadable;
            }

            QString package_name = it.key().package_name;
            QString release_name = it.key().release_name;
            if (installed_map.contains(package_name)) {
                Status to_s;
                if (it.key().version_code.toLongLong() >
                    installed_map[package_name]) {
                    to_s = from_s == Status::Local ? Status::UpdatableLocally
                                                   : Status::UpdatableRemotely;
                } else {
                    to_s = from_s == Status::Local
                               ? Status::InstalledAndLocally
                               : Status::InstalledAndRemotely;
                }
                it.setValue(to_s);
                emit statusChanged(release_name, to_s);

            } else if (from_s == Status::Local) {
                it.setValue(Status::Installable);
                emit statusChanged(release_name, Status::Installable);
            }
        }
    }
    co_return;
}

GameInfo VrpDownloader::getDownloadingGame() const {
    for (const auto& game : downloads_queue_) {
        if (getStatus(game) == Status::Downloading) {
            return game;
        }
    }
    return {};
}

GameInfo VrpDownloader::getFirstQueuedGame() const {
    for (const auto& game : downloads_queue_) {
        if (getStatus(game) == Status::Queued) {
            return game;
        }
    }
    return {};
}