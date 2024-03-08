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

   public:
    enum Status {
        Updatable,
        Downloadable,
        Queued,
        Downloading,
        Decompressing,
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
    Q_INVOKABLE void download(const QString& release_name);
    // Q_INVOKABLE Status getStatus(const QString& release_name){
    //     TODO:
    // }
    QVariantList gamesInfo() const { return games_info_; }

   signals:
    void metadataUpdated();
    void metadataUpdateFailed();
    void gamesInfoChanged();
    void downloadFailed(const QString& release_name);
    void downloadProgressChanged(QString release_name,
                                 double progress,
                                 double speed);
    void downloadSucceeded(const QString& release_name);
    void decompressFailed(const QString& release_name);
    void decompressSucceeded(const QString& release_name);

   public slots:
    void checkDownloadStatus();

   private:
    void downloadMetadata();
    void parseMetadata();
    void decompressGame(const QString& release_name);

    VrpPublic vrp_public_;
    QString cache_path_;
    QString data_path_;
    QVariantList games_info_;
    QTimer download_status_timer_;
    int current_job_id_;
    QString current_release_name_;
    bool ready_to_download_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
