#ifndef QROOKIE_VRP_DOWNLOADER
#define QROOKIE_VRP_DOWNLOADER

#include <QProcess>
#include <QVariant>

#include "game_info.h"
#include "vrp_public.h"

class VrpDownloader : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList gamesInfo READ gamesInfo NOTIFY gamesInfoChanged)

   public:
    VrpDownloader(QObject* parent = nullptr);
    ~VrpDownloader();
    Q_INVOKABLE void updateMetadata();
    Q_INVOKABLE QString getGameThumbnailPath(const QString& package_name);

    QVariantList gamesInfo() const {
        return games_info_;
    }

   signals:
    void metadataUpdated();
    void metadataUpdateFailed();
    void gamesInfoChanged();

   private:
    void downloadMetadata();
    void parseMetadata();
    // void downloadGame();

    VrpPublic vrp_public_;
    QString cache_path_;
    QString data_path_;
    QProcess p7za_;
    // QVector<GameInfo> games_info_;
    QVariantList games_info_;
};

#endif /* QROOKIE_VRP_DOWNLOADER */
