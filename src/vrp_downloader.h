#pragma once
#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include <QCoroTask>
#include <QCryptographicHash>
#include <QProcess>
#include <QTimer>
#include <QVariant>

#include "app_info.h"
#include "device_manager.h"
#include "game_info.h"
#include "vrp_public.h"
#include "http_downloader.h"

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
        Decompressing,
        Local,
        UpdatableLocally,  // Downloaded
        Installable,
        Installing,
        Installed,
        Error,
        Unknown
    };

    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    QCoro::Task<bool> updateMetadata();
    Q_INVOKABLE QCoro::QmlTask updateMetadataQml() { return updateMetadata(); }
    Q_INVOKABLE void filterGamesByName(const QString& filter){
        filter_ = filter;
        emit gamesInfoChanged();
    }
    Q_INVOKABLE QVariantList find(const QString& package_name);
    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);
    Q_INVOKABLE QString getGameId(const QString& release_name) const;
    Q_INVOKABLE QString getLocalGamePath(const QString& release_name) const;
    Q_INVOKABLE bool addToDownloadQueue(const GameInfo game, bool auto_install);

    QCoro::Task<bool> install(const GameInfo game);
    Q_INVOKABLE QCoro::QmlTask installQml(const GameInfo game) {
        return install(game);
    }
    Q_INVOKABLE Status getStatus(const GameInfo& game);

    Q_INVOKABLE bool isUpdatableRemotely(const GameInfo& game) const {
        if (!hasConnectedDevice()) return false;
        for (const auto& app : installed_queue_) {
            if (app.package_name == game.package_name &&
                app.version_code < game.version_code.toLongLong()) {
                return true;
            }
        }
        return false;
    }
    Q_INVOKABLE bool isDownloadable(const GameInfo& game) const {
        return games_info_.contains(game);
    }
    Q_INVOKABLE bool isQueued(const GameInfo& game) const {
        return !downloading_queue_.empty() && downloading_queue_.contains(game);
    }
    Q_INVOKABLE bool isDownloading(const GameInfo& game) const {
        return !downloading_queue_.empty() &&
               downloading_queue_.first().release_name == game.release_name;
    }
    Q_INVOKABLE bool isDecompressing(const GameInfo& game) const {
        return !decompressing_queue_.empty() &&
               decompressing_queue_.contains(game);
    }
    Q_INVOKABLE bool isLocal(const GameInfo& game) const {
        return !local_queue_.empty() && local_queue_.contains(game);
    }

    Q_INVOKABLE bool isUpdatableLocally(const GameInfo& game) const {
        if (!hasConnectedDevice() || !isLocal(game)) return false;
        for (const auto& app : installed_queue_) {
            if (app.package_name == game.package_name &&
                app.version_code < game.version_code.toLongLong()) {
                return true;
            }
        }
        return false;
    }

    Q_INVOKABLE bool isInstallable(const GameInfo& game) const {
        return hasConnectedDevice() && isLocal(game);
    }

    Q_INVOKABLE bool isInstalling(const GameInfo& game) const {
        return !installing_queue_.empty() && installing_queue_.contains(game);
    }

    Q_INVOKABLE bool isInstalled(const GameInfo& game) const {
        if (!hasConnectedDevice()) return false;

        for (const auto& app : installed_queue_) {
            if (app.package_name == game.package_name &&
                app.version_code >= game.version_code.toLongLong()) {
                return true;
            }
        }
        return false;
    }
    Q_INVOKABLE bool isFailed(const GameInfo& game) const {
        return failed_queue_.contains(game);
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
    bool saveLocalQueue();
    bool loadLocalQueue();
    QCoro::Task<void> updateInstalledQueue();

    VrpPublic vrp_public_;
    QString cache_path_;
    QString data_path_;
    QVector<GameInfo> games_info_;
    QString filter_;
    QVector<GameInfo> downloading_queue_;
    QVector<GameInfo> decompressing_queue_;
    QVector<GameInfo> local_queue_;
    QVector<GameInfo> failed_queue_;
    QVector<GameInfo> installing_queue_;
    QVector<AppInfo> installed_queue_;
    QVector<GameInfo> auto_install_queue_;
    DeviceManager device_manager_;
    QString connected_device_;
    QString device_model_;
    long long total_space_;
    long long free_space_;
    HttpDownloader http_downloader_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
