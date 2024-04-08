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

#include <QCoroTask>
#include <QCryptographicHash>
#include <QMap>
#include <QMetaEnum>
#include <QMultiMap>
#include <QProcess>
#include <QTimer>
#include <QVariant>

#include "app_info.h"
#include "device_manager.h"
#include "game_info.h"
#include "http_downloader.h"
#include "vrp_public.h"
#include "vrp_torrent.h"

class VrpDownloader : public QObject {
    Q_OBJECT
    Q_ENUMS(Status)

    Q_PROPERTY(QVariantList gamesInfo READ gamesInfo NOTIFY gamesInfoChanged)
    Q_PROPERTY(QVariantList downloadsQueue READ downloadsQueue NOTIFY
                   downloadsQueueChanged)
    Q_PROPERTY(QVariantList localQueue READ localQueue NOTIFY localQueueChanged)
    Q_PROPERTY(QVariantList installedQueue READ installedQueue NOTIFY
                   installedQueueChanged)
    Q_PROPERTY(QVariantList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QString connectedDevice READ connectedDevice WRITE
                   connectToDevice NOTIFY connectedDeviceChanged)
    Q_PROPERTY(QString deviceModel READ deviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(long long totalSpace READ totalSpace NOTIFY spaceUsageChanged)
    Q_PROPERTY(long long freeSpace READ freeSpace NOTIFY spaceUsageChanged)

   public:
    enum Status {
        Unknown = 0x0000,
        UpdatableRemotely = 0x0001,  // Not yet downloaded
        Downloadable = 0x0002,
        Queued = 0x0004,
        Downloading = 0x0008,
        DownloadError = 0x0010,
        Decompressing = 0x0020,
        DecompressionError = 0x0040,
        Local = 0x0080,
        UpdatableLocally = 0x0100,  // Downloaded
        Installable = 0x0200,
        Installing = 0x0400,
        InstallError = 0x0800,
        InstalledAndRemotely = 0x1000,
        InstalledAndLocally = 0x2000
    };
    Q_DECLARE_FLAGS(StatusFlags, Status)
    Q_FLAG(Status)

    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    QCoro::Task<bool> updateMetadata();
    Q_INVOKABLE QCoro::QmlTask updateMetadataQml() { return updateMetadata(); }
    Q_INVOKABLE void filterGamesByName(const QString& filter) {
        filter_ = filter;
        filter_.remove(" ");
        emit gamesInfoChanged();
    }

    Q_INVOKABLE void filterGamesByStatus(const StatusFlags status_filter) {
        status_filter_ = status_filter;
        emit gamesInfoChanged();
    }

    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);
    Q_INVOKABLE QString getGameId(const QString& release_name) const;
    Q_INVOKABLE QString getLocalGamePath(const QString& release_name) const;
    Q_INVOKABLE QString getMagnetURI(const QString& release_name) const {
        return vrp_torrent_.findMagnetURI(release_name);
    }
    Q_INVOKABLE bool addToDownloadQueue(const GameInfo game);
    Q_INVOKABLE void removeFromDownloadQueue(const GameInfo& game);
    Q_INVOKABLE bool removeFromLocalQueue(const GameInfo& game);

    QCoro::Task<bool> install(const GameInfo game);
    Q_INVOKABLE QCoro::QmlTask installQml(const GameInfo game) {
        return install(game);
    }

    QCoro::Task<bool> uninstall(const QString packege_name);
    Q_INVOKABLE QCoro::QmlTask uninstallQml(const QString packege_name) {
        return uninstall(packege_name);
    }

    Q_INVOKABLE Status getStatus(const GameInfo& game) const {
        return all_games_[game];
    }
    void setStatus(const GameInfo& game, Status status) {
        if (!all_games_.contains(game)) {
            return;
        }

        all_games_[game] = status;
        emit statusChanged(game.release_name, status);
    }

    Q_INVOKABLE bool hasConnectedDevice() const {
        return !connected_device_.isEmpty();
    }

    void connectToDevice(const QString& serial) {
        if (connected_device_ != serial &&
            device_manager_.serials().contains(serial)) {
            connected_device_ = serial;
            emit connectedDeviceChanged();
        }
    }
    Q_INVOKABLE void disconnectDevice() {
        connected_device_.clear();
        emit connectedDeviceChanged();
    }

    QString connectedDevice() const { return connected_device_; }
    QString deviceModel() const { return device_model_; }
    long long totalSpace() const { return total_space_; }
    long long freeSpace() const { return free_space_; }

    QVariantList gamesInfo() const;
    QVariantList downloadsQueue() const;
    QVariantList localQueue() const;
    QVariantList installedQueue() const;
    QVariantList deviceList() const;

   signals:
    void gamesInfoChanged();
    void downloadsQueueChanged();
    void localQueueChanged();
    void installedQueueChanged();
    void deviceListChanged();
    void connectedDeviceChanged();
    void deviceModelChanged();
    void spaceUsageChanged(long long total_space, long long free_space);
    void statusChanged(QString release_name, Status status);
    void downloadProgressChanged(QString release_name, double progress);

   private:
    QCoro::Task<bool> downloadMetadata();
    bool parseMetadata();
    QCoro::Task<bool> decompressGame(const GameInfo game);
    QCoro::Task<void> downloadQueuedGames();
    bool saveGamesInfo();
    bool loadGamesInfo();
    QCoro::Task<void> updateInstalledQueue();
    GameInfo getDownloadingGame() const;
    GameInfo getFirstQueuedGame() const;

    VrpPublic vrp_public_;
    VrpTorrent vrp_torrent_;
    QString cache_path_;
    QString data_path_;
    QString filter_;
    StatusFlags status_filter_;
    QVector<AppInfo> installed_queue_;
    QVector<GameInfo> downloads_queue_;
    QVector<GameInfo> local_queue_;
    QMap<GameInfo, Status> all_games_;
    DeviceManager device_manager_;
    QString connected_device_;
    QString device_model_;
    long long total_space_;
    long long free_space_;
    HttpDownloader http_downloader_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
