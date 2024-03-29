#pragma once
#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include <QCoroTask>
#include <QCryptographicHash>
#include <QMap>
#include <QMultiMap>
#include <QProcess>
#include <QTimer>
#include <QVariant>

#include "app_info.h"
#include "device_manager.h"
#include "game_info.h"
#include "http_downloader.h"
#include "vrp_public.h"

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
        UpdatableRemotely,  // Not yet downloaded
        Downloadable,
        Queued,
        Downloading,
        DownloadError,
        Decompressing,
        DecompressionError,
        Local,
        UpdatableLocally,  // Downloaded
        Installable,
        Installing,
        InstallError,
        InstalledAndRemotely,
        InstalledAndLocally,
        Unknown
    };

    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    QCoro::Task<bool> updateMetadata();
    Q_INVOKABLE QCoro::QmlTask updateMetadataQml() { return updateMetadata(); }
    Q_INVOKABLE void filterGamesByName(const QString& filter) {
        filter_ = filter;
        filter_.remove(" ");
        emit gamesInfoChanged();
    }
    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);
    Q_INVOKABLE QString getGameId(const QString& release_name) const;
    Q_INVOKABLE QString getLocalGamePath(const QString& release_name) const;
    Q_INVOKABLE bool addToDownloadQueue(const GameInfo game);
    Q_INVOKABLE void removeFromDownloadQueue(const GameInfo& game);

    QCoro::Task<bool> install(const GameInfo game);
    Q_INVOKABLE QCoro::QmlTask installQml(const GameInfo game) {
        return install(game);
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
        // TODO: emit queue change signal
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
    Q_INVOKABLE bool removeFromLocalQueue(const GameInfo& game);

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
    void spaceUsageChanged();
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
    QString cache_path_;
    QString data_path_;
    QString filter_;
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
