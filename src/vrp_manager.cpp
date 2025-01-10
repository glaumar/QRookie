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

#include "vrp_manager.h"

#include <QCoroTimer>
#include <QDir>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSharedPointer>
#include <QStandardPaths>

#include "qrookie.h"

#ifdef Q_OS_MAC
const QString OPEN_CMD("open");
#else
const QString OPEN_CMD("xdg-open");
#endif

const QString P7ZA("7za");

VrpManager::VrpManager(QObject *parent)
    : QObject(parent)
    , status_filter_(Status::Unknown)
    , sort_type_(SortType::SortByDate)
    , sort_order_(Qt::AscendingOrder)
    , local_games_(new GameInfoModel(this))
    , download_games_(new GameInfoModel(this))
    , device_manager_(new DeviceManager(this))
    , cache_path_(AppSettings::instance()->cachePath())
    , data_path_(AppSettings::instance()->dataPath())
{
    http_downloader_.setDownloadDirectory(cache_path_);

    connect(device_manager_, &DeviceManager::appListChanged, this, &VrpManager::updateGameStatusWithDevice);
    device_manager_->enableAutoUpdate();

    connect(local_games_, &GameInfoModel::removed, this, &VrpManager::removeLocalGameFile);
    connect(download_games_, &GameInfoModel::removed, this, &VrpManager::removeFromDownloadQueue);
    loadGamesInfo();
    // updateInstalledApps();

    // Restore download
    if (all_games_.key(Status::Queued) != GameInfo{}) {
        downloadQueuedGames();
    }
}

VrpManager::~VrpManager()
{
    saveGamesInfo();
}

QVariantList VrpManager::gamesInfo() const
{
    QList<GameInfo> games;

    for (auto it = all_games_.constBegin(); it != all_games_.constEnd(); ++it) {
        QString name = it.key().name;

        if (status_filter_ != Status::Unknown && !(it.value() & status_filter_)) {
            continue;
        }

        if (filter_.isEmpty() || name.remove(" ").contains(filter_, Qt::CaseInsensitive)) {
            games.append(it.key());
        }
    }

    std::sort(games.begin(), games.end(), [this](const GameInfo &a, const GameInfo &b) {
        switch (sort_type_) {
        case SortType::SortByName: {
            auto name_a = a.name.toLower();
            auto name_b = b.name.toLower();
            return sort_order_ == Qt::AscendingOrder ? name_a < name_b : name_a > name_b;
        }
        case SortType::SortBySize: {
            long long size_a = a.size.toLongLong();
            long long size_b = b.size.toLongLong();
            return sort_order_ == Qt::AscendingOrder ? size_a < size_b : size_a > size_b;
        }
        case SortType::SortByDate:
            // Sort by most recently updated, with the latest updates at the front
            return sort_order_ == Qt::AscendingOrder ? a.last_updated > b.last_updated : a.last_updated < b.last_updated;
        default:
            return false;
        }
    });

    QVariantList list;
    for (auto &game : games) {
        list.append(QVariant::fromValue(game));
    }
    return list;
}

QCoro::Task<bool> VrpManager::updateMetadata()
{
    vrp_torrent_.update();
    if (!co_await vrp_public_.update()) {
        qWarning() << "Update config failed";
        co_return false;
    } else {
        http_downloader_.setBaseUrl(vrp_public_.baseUrl());
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

QCoro::Task<bool> VrpManager::downloadMetadata()
{
    if (co_await http_downloader_.download("meta.7z")) {
        QProcess basic_process;
        auto p7za = qCoro(basic_process);

        // Decompress meta.7z
        p7za.start(P7ZA,
                   QStringList() << "x" << QString("%1/meta.7z").arg(http_downloader_.downloadDirectory())
                                 << "-aoa" // Overwrite All existing files without prompt.
                                 << QString("-o%1").arg(data_path_) << QString("-p%1").arg(vrp_public_.password()));

        co_await p7za.waitForFinished();
        QFile::remove(http_downloader_.downloadDirectory() + "/meta.7z");

        if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
            qWarning("meta.7z decompression failed: %s\n %s", basic_process.readAllStandardOutput().data(), basic_process.readAllStandardError().data());
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

bool VrpManager::parseMetadata()
{
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
        if (parts.size() < 6) {
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
        return true;
    }
}

QString VrpManager::getGameId(const QString &release_name) const
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData((release_name + "\n").toUtf8());
    return hash.result().toHex();
}

QString VrpManager::getLocalGamePath(const QString &release_name) const
{
    // TODO: check path exists
    return data_path_ + "/" + release_name;
}

QString VrpManager::getGameThumbnailPath(const QString &package_name)
{
    auto path = data_path_ + "/.meta/thumbnails/" + package_name + ".jpg";
    if (QFile::exists(path)) {
        return path;
    } else {
        return "";
    }
}

bool VrpManager::addToDownloadQueue(const GameInfo game)
{
    Status s = getStatus(game);

    static constexpr StatusFlags download_flags = {
        Status::Downloadable,
        Status::UpdatableRemotely,
        Status::DownloadError,
        Status::DecompressionError,
    };

    if (!download_flags.testFlag(s)) {
        return false;
    }

    // Decompression failure is most likely due to a corrupted download file, delete the cache and start downloading from scratch
    if (s == Status::DecompressionError) {
        cleanCache(game.release_name);
    }

    download_games_->remove(game);
    setStatus(game, Status::Queued);
    download_games_->append(game);
    qDebug() << "Queued: " << game.release_name;

    // Start download if not already downloading
    if (getDownloadingGame() == GameInfo{}) {
        downloadQueuedGames();
    }

    return true;
}

void VrpManager::cleanCache(const QString &release_name) const
{
    QDir dir(cache_path_ + "/" + getGameId(release_name));
    dir.removeRecursively();
}

void VrpManager::removeFromDownloadQueue(const GameInfo &game)
{
    Status s = getStatus(game);
    if (s == Status::Downloading) {
        http_downloader_.abortDownloadDir(getGameId(game.release_name));
        setStatus(game, Status::Downloadable);
    } else if (s == Status::Queued) {
        setStatus(game, Status::Downloadable);
    }
}

bool VrpManager::removeLocalGameFile(const GameInfo &game)
{
    setStatus(game, Status::Downloadable);
    auto game_dir = getLocalGamePath(game.release_name);

    QDir dir(game_dir);
    if (dir.exists()) {
        return dir.removeRecursively();
    }
    return false;
}

QCoro::Task<void> VrpManager::downloadQueuedGames()
{
    GameInfo game = getFirstQueuedGame();
    while (game != GameInfo{}) {
        qDebug() << "Downloading: " << game.release_name;
        setStatus(game, Status::Downloading);

        QString id = getGameId(game.release_name);

        auto conn = connect(&http_downloader_,
                            &HttpDownloader::downloadProgressDir,
                            this,
                            [this, id, game](QString dir_name, qint64 bytes_received, qint64 bytes_total) {
                                if (dir_name == id) {
                                    emit downloadProgressChanged(game.release_name, double(bytes_received) / double(bytes_total));
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

QCoro::Task<bool> VrpManager::decompressGame(const GameInfo game)
{
    if (getStatus(game) == Status::Decompressing) {
        qDebug() << "Already in decompressing queue: " << game.release_name;
        co_return false;
    }

    qDebug() << "Decompressing: " << game.release_name;
    setStatus(game, Status::Decompressing);

    QProcess basic_process;
    auto p7za = qCoro(basic_process);

    // Decompress
    p7za.start(P7ZA,
               QStringList() << "x" << QString("%1/%2/%2.7z.001").arg(cache_path_, getGameId(game.release_name))
                             << "-aoa" // Overwrite All existing files without prompt.
                             << QString("-o%1").arg(data_path_) << QString("-p%1").arg(vrp_public_.password()));

    co_await p7za.waitForFinished(-1);

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning("Error: %s\n %s", basic_process.readAllStandardOutput().data(), basic_process.readAllStandardError().data());
        qDebug() << "Decompression failed: " << game.release_name;
        setStatus(game, Status::DecompressionError);
        co_return false;
    } else {
        qDebug() << "Decompression finished: " << game.release_name;

        if (device_manager_->hasConnectedDevice()) {
            setStatus(game, Status::Installable);
        } else {
            setStatus(game, Status::Local);
        }

        download_games_->remove(game);
        local_games_->prepend(game);

        if (settings()->autoInstall() && device_manager_->hasConnectedDevice()) {
            install(game);
        }

        if (settings()->autoCleanCache()) {
            cleanCache(game.release_name);
        }
        co_return true;
    }
}

QCoro::Task<bool> VrpManager::install(const GameInfo game)
{
    if (!device_manager_->hasConnectedDevice()) {
        co_return false;
    }
    qDebug() << "Installing: " << game.release_name;
    setStatus(game, Status::Installing);
    bool result = co_await device_manager_->installApk(getLocalGamePath(game.release_name), game.package_name, settings()->renamePackage());

    if (result) {
        qDebug() << "Install finished: " << game.release_name;
        setStatus(game, Status::InstalledAndLocally);
    } else {
        qDebug() << "Install failed: " << game.release_name;
        setStatus(game, Status::InstallError);
    }

    co_return result;
}

bool VrpManager::saveGamesInfo()
{
    QJsonArray jsonArray;
    auto it = all_games_.constBegin();
    auto meta_status = QMetaEnum::fromType<Status>();

    while (it != all_games_.constEnd()) {
        QJsonObject jsonObject;
        const GameInfo &game = it.key();
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

bool VrpManager::loadGamesInfo()
{
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
        for (const auto &value : jsonArray) {
            GameInfo game;
            auto obj = value.toObject();
            game.name = obj["name"].toString();
            game.release_name = obj["release_name"].toString();
            game.package_name = obj["package_name"].toString();
            game.version_code = obj["version_code"].toString();
            game.last_updated = obj["last_updated"].toString();
            game.size = value.toObject()["size"].toString();

            int status_int = meta_status.keyToValue(obj["status"].toString().toUtf8());

            Status status = status_int >= 0 ? static_cast<Status>(status_int) : Status::Unknown;
            all_games_[game] = status;
            if (status == Status::Queued || status == Status::Downloading || status == Status::DownloadError || status == Status::Decompressing
                || status == Status::DecompressionError) {
                download_games_->append(game);
            } else if (status == Status::Local || status == Status::Installable || status == Status::InstalledAndLocally || status == Status::InstallError) {
                local_games_->append(game);
            }
        }

        emit gamesInfoChanged();
        return true;
    } else {
        qWarning("load games info: Failed to open file for reading.");
        return false;
    }
}

void VrpManager::updateGameStatusWithDevice()
{
    auto apps_model = device_manager_->appListModel();
    StatusFlags remote_flags = {Status::UpdatableRemotely, Status::InstalledAndRemotely};

    StatusFlags local_flags = {Status::UpdatableLocally, Status::InstalledAndLocally, Status::Installable, Status::InstallError, Status::Installing};

    if (apps_model->rowCount() == 0) {
        // Update Status
        QMutableMapIterator<GameInfo, Status> it(all_games_);
        while (it.hasNext()) {
            it.next();

            Status s = it.value();
            if (remote_flags.testFlag(s)) {
                it.setValue(Status::Downloadable);
                emit statusChanged(it.key().release_name, Status::Downloadable);
            } else if (local_flags.testFlag(s)) {
                it.setValue(Status::Local);
                emit statusChanged(it.key().release_name, Status::Local);
            }
        }

        return;
    }

    QMap<QString, QString> installed_map;
    for (int i = 0; i < apps_model->rowCount(); i++) {
        auto app = (*apps_model)[i];
        installed_map[app.package_name] = app.version_code;
    }

    // Update game status
    QMutableMapIterator<GameInfo, Status> it(all_games_);
    while (it.hasNext()) {
        it.next();
        Status from_s = it.value();

        if (local_flags.testFlag(from_s)) {
            from_s = Status::Local;
        } else if (remote_flags.testFlag(from_s)) {
            from_s = Status::Downloadable;
        }

        QString package_name = it.key().package_name;
        QString release_name = it.key().release_name;
        if (installed_map.contains(package_name)) {
            Status to_s;
            if (it.key().version_code.toLongLong() > installed_map[package_name].toLongLong()) {
                to_s = from_s == Status::Local ? Status::UpdatableLocally : Status::UpdatableRemotely;
            } else {
                to_s = from_s == Status::Local ? Status::InstalledAndLocally : Status::InstalledAndRemotely;
            }
            it.setValue(to_s);
            emit statusChanged(release_name, to_s);

        } else if (from_s == Status::Local) {
            it.setValue(Status::Installable);
            emit statusChanged(release_name, Status::Installable);
        }
    }
}

GameInfo VrpManager::getDownloadingGame() const
{
    for (int i = 0; i < download_games_->size(); i++) {
        const auto &game = (*download_games_)[i];
        if (getStatus(game) == Status::Downloading) {
            return game;
        }
    }
    return {};
}

GameInfo VrpManager::getFirstQueuedGame() const
{
    for (int i = 0; i < download_games_->size(); i++) {
        const auto &game = (*download_games_)[i];
        if (getStatus(game) == Status::Queued) {
            return game;
        }
    }
    return {};
}

QCoro::Task<bool> VrpManager::openGameFolder(const QString release_name)
{
    QProcess basic_process;
    auto xdg_open = qCoro(basic_process);
    xdg_open.start(OPEN_CMD, {getLocalGamePath(release_name)});
    co_await xdg_open.waitForFinished();

    if (basic_process.exitStatus() != QProcess::NormalExit || basic_process.exitCode() != 0) {
        qWarning("Error: %s\n %s", basic_process.readAllStandardOutput().data(), basic_process.readAllStandardError().data());
        co_return false;
    }
    co_return true;
}

QStringList VrpManager::compatibleThemes() const
{
#ifdef Q_OS_LINUX
    return {"org.kde.breeze", "Universal"};
#elif defined(Q_OS_MAC)
    return {"Material", "Universal"};
#elif defined(Q_OS_WIN)
    return {"WindowsVista", "Universal"};
#else
    return {"Universal"};
#endif
}

void VrpManager::restartMainApp()
{
    QString program = qApp->arguments()[0];
    QStringList arguments = qApp->arguments().mid(1);
    qApp->exit(AppSettings::EXIT_RESTART);
    QProcess::startDetached(program, arguments);
}
