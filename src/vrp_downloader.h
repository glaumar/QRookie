#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include <QCryptographicHash>
#include <QProcess>
#include <QTimer>
#include <QVariant>

#include "game_info.h"
#include "vrp_public.h"

class VrpDownloader : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList gamesInfo READ gamesInfo NOTIFY gamesInfoChanged)
    Q_PROPERTY(QVariantList downloadsQueue READ downloadsQueue NOTIFY
                   downloadsQueueChanged)

   public:
    enum Status {
        Updatable,
        Downloadable,
        Queued,
        Downloading,
        Decompressing,
        Installable,
        Installing,
        Installed,
        Error
    };
    Q_ENUM(Status)

    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    Q_INVOKABLE void updateMetadata();
    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);
    Q_INVOKABLE QString getGameId(const QString& release_name) const;
    Q_INVOKABLE void download(const GameInfo& game);
    Q_INVOKABLE bool isDownloading(const QString& release_name) const {
        return !downloads_queue_.empty() &&
               downloads_queue_.first().release_name == release_name;
    }
    Q_INVOKABLE bool isQueued(const QString& release_name) const {
        return !downloads_queue_.empty() && downloads_queue_.contains(GameInfo{
                                                .release_name = release_name});
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
        for (const auto& game_info : downloads_queue_) {
            list.append(QVariant::fromValue(game_info));
        }

        return list;
    }

   signals:
    void metadataUpdated();
    void metadataUpdateFailed();
    void gamesInfoChanged();
    void downloadsQueueChanged();
    void downloadFailed(QString release_name);
    void downloadProgressChanged(QString release_name,
                                 double progress,
                                 double speed);
    void downloadSucceeded(QString release_name);
    void decompressFailed(QString release_name);
    void decompressSucceeded(QString release_name);

   public slots:
    void checkDownloadStatus();

   private:
    void downloadMetadata();
    void parseMetadata();
    void decompressGame(const QString& release_name);
    void downloadNext();

    VrpPublic vrp_public_;
    QString cache_path_;
    QString data_path_;
    QVector<GameInfo> games_info_;
    QVector<GameInfo> downloads_queue_;
    QTimer download_status_timer_;
    int current_job_id_;
    // QString current_release_name_;
    // bool ready_to_download_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
