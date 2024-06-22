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

#pragma once
#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include "app_settings.h"
#include "device_manager.h"
#include "models/game_info.h"
#include "models/game_info_model.h"
#include "http_downloader.h"
#include "vrp_public.h"
#include "vrp_torrent.h"
#include <QCoroTask>
#include <QCryptographicHash>
#include <QMap>
#include <QMetaEnum>
#include <QMultiMap>
#include <QProcess>
#include <QVariant>

class VrpManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_ENUMS(SortType)

    Q_PROPERTY(QVariantList gamesInfo READ gamesInfo NOTIFY gamesInfoChanged)
    Q_PROPERTY(AppSettings *settings READ settings)

public:
    enum Status {
        Unknown = 0x0000,
        UpdatableRemotely = 0x0001, // Not yet downloaded
        Downloadable = 0x0002,
        Queued = 0x0004,
        Downloading = 0x0008,
        DownloadError = 0x0010,
        Decompressing = 0x0020,
        DecompressionError = 0x0040,
        Local = 0x0080,
        UpdatableLocally = 0x0100, // Downloaded
        Installable = 0x0200,
        Installing = 0x0400,
        InstallError = 0x0800,
        InstalledAndRemotely = 0x1000,
        InstalledAndLocally = 0x2000
    };
    Q_DECLARE_FLAGS(StatusFlags, Status)
    Q_FLAG(Status)

    enum SortType { SortByDate, SortByName, SortBySize };

    explicit VrpManager(QObject *parent = nullptr);
    ~VrpManager();
    QCoro::Task<bool> updateMetadata();
    Q_INVOKABLE QCoro::QmlTask updateMetadataQml()
    {
        return updateMetadata();
    }
    Q_INVOKABLE void filterGamesByName(const QString &filter)
    {
        filter_ = filter;
        filter_.remove(" ");
        emit gamesInfoChanged();
    }

    Q_INVOKABLE void filterGamesByStatus(const StatusFlags status_filter)
    {
        status_filter_ = status_filter;
        emit gamesInfoChanged();
    }

    Q_INVOKABLE void sortGames(SortType sort_type, Qt::SortOrder order = Qt::AscendingOrder)
    {
        sort_type_ = sort_type;
        sort_order_ = order;
        emit gamesInfoChanged();
    }

    // TODO: as property
    Q_INVOKABLE GameInfoModel *localGamesModel()
    {
        return local_games_;
    }
    // TODO: as property
    Q_INVOKABLE GameInfoModel *downloadGamesModel()
    {
        return download_games_;
    }
    // TODO: as property
    Q_INVOKABLE DeviceManager *deviceManager()
    {
        return device_manager_;
    }

    QCoro::Task<bool> install(const GameInfo game);
    Q_INVOKABLE QCoro::QmlTask installQml(const GameInfo game)
    {
        return install(game);
    }
    Q_INVOKABLE QString getGameThumbnailPath(const QString &package_name);
    Q_INVOKABLE QString getGameId(const QString &release_name) const;
    Q_INVOKABLE QString getLocalGamePath(const QString &release_name) const;
    Q_INVOKABLE QString getMagnetURI(const QString &release_name) const
    {
        return vrp_torrent_.findMagnetURI(release_name);
    }
    Q_INVOKABLE bool addToDownloadQueue(const GameInfo game);
    Q_INVOKABLE void cleanCache(const QString &release_name) const;
    void removeFromDownloadQueue(const GameInfo &game);
    bool removeLocalGameFile(const GameInfo &game);

    Q_INVOKABLE Status getStatus(const GameInfo &game) const
    {
        return all_games_[game];
    }
    void setStatus(const GameInfo &game, Status status)
    {
        if (!all_games_.contains(game)) {
            return;
        }

        all_games_[game] = status;
        emit statusChanged(game.release_name, status);
    }

    QVariantList gamesInfo() const;
    Q_INVOKABLE AppSettings *settings() const
    {
        return AppSettings::instance();
    }

    QCoro::Task<bool> openGameFolder(const QString release_name);
    Q_INVOKABLE QCoro::QmlTask openGameFolderQml(const QString &release_name)
    {
        return openGameFolder(release_name);
    }

signals:
    void gamesInfoChanged();
    void statusChanged(QString release_name, Status status);
    void downloadProgressChanged(QString release_name, double progress);

private:
    QCoro::Task<bool> downloadMetadata();
    bool parseMetadata();
    QCoro::Task<bool> decompressGame(const GameInfo game);
    QCoro::Task<void> downloadQueuedGames();
    bool saveGamesInfo();
    bool loadGamesInfo();
    GameInfo getDownloadingGame() const;
    GameInfo getFirstQueuedGame() const;
    void updateGameStatusWithDevice();

    VrpPublic vrp_public_;
    VrpTorrent vrp_torrent_;
    QString cache_path_;
    QString data_path_;
    QString filter_;
    StatusFlags status_filter_;
    SortType sort_type_;
    Qt::SortOrder sort_order_;
    DeviceManager *device_manager_;
    GameInfoModel *download_games_;
    GameInfoModel *local_games_;
    QMap<GameInfo, Status> all_games_;
    HttpDownloader http_downloader_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
