#pragma once
#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include <QCryptographicHash>
#include <QProcess>
#include <QTimer>
#include <QVariant>
#include "device_manager.h"

#include "game_info.h"
#include "vrp_public.h"

class VrpDownloader : public QObject {
    Q_OBJECT
    Q_ENUMS(Status)

    Q_PROPERTY(QVariantList gamesInfo READ gamesInfo NOTIFY gamesInfoChanged)
    Q_PROPERTY(QVariantList downloadsQueue READ downloadsQueue NOTIFY
                   downloadsQueueChanged)
    Q_PROPERTY(QVariantList localQueue READ localQueue NOTIFY localQueueChanged)

   public:
    enum Status {
        // Updatable,
        // Downloadable,
        Queued,
        Downloading,
        Decompressing,
        Local,
        // Installable,
        // Installing,
        // Installed,
        Error,
        Unknown
    };

    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    Q_INVOKABLE void updateMetadata();
    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);
    Q_INVOKABLE QString getGameId(const QString& release_name) const;
    Q_INVOKABLE void download(const GameInfo& game);
    Q_INVOKABLE Status getStatus(const GameInfo& game);
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
        return local_queue_.contains(game);
    }
    Q_INVOKABLE bool isFailed(const GameInfo& game) const {
        return failed_queue_.contains(game);
    }

    QVariantList gamesInfo() const {
        QVariantList list;
        for (const auto& game_info : games_info_) {
            list.append(QVariant::fromValue(game_info));
        }
        return list;
    }

    QVariantList downloadsQueue() const {
        QVariantList list;
        for (const auto& game_info : decompressing_queue_) {
            list.append(QVariant::fromValue(game_info));
        }

        for (const auto& game_info : downloading_queue_) {
            list.append(QVariant::fromValue(game_info));
        }

        for (const auto& game_info : failed_queue_) {
            list.append(QVariant::fromValue(game_info));
        }

        return list;
    }

    QVariantList localQueue() const {
        QVariantList list;
        for (const auto& game_info : local_queue_) {
            list.append(QVariant::fromValue(game_info));
        }

        return list;
    }

   signals:
    void metadataUpdated();
    void metadataUpdateFailed();
    void gamesInfoChanged();
    void downloadsQueueChanged();
    void localQueueChanged();
    void statusChanged(QString release_name, Status status);
    void downloadFailed(GameInfo game);
    void downloadProgressChanged(QString release_name,
                                 double progress,
                                 double speed);
    void downloadSucceeded(GameInfo game);
    void decompressFailed(GameInfo game);
    void decompressSucceeded(GameInfo game);

   public slots:
    void checkDownloadStatus();

   private:
    void downloadMetadata();
    void parseMetadata();
    void decompressGame(const GameInfo& game);
    void downloadNext();
    bool saveLocalQueue();
    bool loadLocalQueue();

    VrpPublic vrp_public_;
    QString cache_path_;
    QString data_path_;
    QVector<GameInfo> games_info_;
    QVector<GameInfo> downloading_queue_;
    QVector<GameInfo> decompressing_queue_;
    QVector<GameInfo> local_queue_;
    QVector<GameInfo> failed_queue_;
    QTimer download_status_timer_;
    int current_job_id_;
    DeviceManager device_manager_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
