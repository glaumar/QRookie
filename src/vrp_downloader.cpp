#include "vrp_downloader.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSharedPointer>
#include <QStandardPaths>

#include "qrookie.h"
#include "rclone_result.h"

VrpDownloader::VrpDownloader(QObject* parent)
    : QObject(parent), ready_to_download_(true) {
    RcloneInitialize();

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

    // vrp_public_ updated
    connect(&vrp_public_,
            &VrpPublic::updated,
            this,
            &VrpDownloader::downloadMetadata);

    connect(&vrp_public_,
            &VrpPublic::failed,
            this,
            &VrpDownloader::metadataUpdateFailed);

    connect(&download_status_timer_,
            &QTimer::timeout,
            this,
            &VrpDownloader::checkDownloadStatus);
}

VrpDownloader::~VrpDownloader() {
    RcloneFinalize();
    // TODO: cleanup cache
}

void VrpDownloader::updateMetadata() {
    // TODO: uncomment this line
    // vrp_public_.update();

    // For testing now
    parseMetadata();
}

void VrpDownloader::downloadMetadata() {
    // https://rclone.org/rc/#operations-copyfile
    QString rc_method("operations/copyfile");
    QString rc_input = QString(R"({
        "srcFs": ":http,url='%1':",
        "srcRemote": "/meta.7z",
        "dstFs": "%2",
        "dstRemote": "meta.7z",
        "_config": {
                "Transfers": 1,
                "TPSLimit": 1.0,
                "TPSLimitBurst": 3,
                "MultiThreadStreams": 0,
                "UserAgent": "rclone/v1.65.2"
        }
    })")
                           .arg(vrp_public_.baseUrl(), cache_path_);

    RcloneResult result = RcloneRPC(rc_method, rc_input);
    if (result.isSuccessful()) {
        auto p7za = QSharedPointer<QProcess>::create(this);
        connect(p7za.data(),
                &QProcess::finished,
                this,
                [this, p7za](int exitCode, QProcess::ExitStatus exitStatus) {
                    if (exitStatus != QProcess::NormalExit ||
                        p7za->exitCode() != 0) {
                        qWarning("meta.7z decompression failed: %s\n %s",
                                 p7za->readAllStandardOutput().data(),
                                 p7za->readAllStandardError().data());
                        emit metadataUpdateFailed();
                    } else {
                        qDebug() << "meta.7z decompression successful";
                        parseMetadata();
                    }
                });

        // Decompress meta.7z
        p7za->start(
            "7za",
            QStringList()
                << "x" << QString("%1/meta.7z").arg(cache_path_)
                << "-aoa"  // Overwrite All existing files without prompt.
                << QString("-o%1").arg(data_path_)
                << QString("-p%1").arg(vrp_public_.password()));
    } else {
        qWarning() << "Download Metadata failed :\n\t status :"
                   << result.status() << "\n\t output: " << result.output();
    }
}

void VrpDownloader::parseMetadata() {
    QFile file(data_path_ + "/VRP-GameList.txt");
    if (!file.exists()) {
        qWarning() << "VRP-GameList.txt not found";
        emit metadataUpdateFailed();
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "VRP-GameList.txt open failed";
        emit metadataUpdateFailed();
        return;
    }

    QTextStream in(&file);

    // Discard the first line
    in.readLine();

    games_info_.clear();
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

            games_info_.append(QVariant::fromValue(game_info));
        }
    }

    if (games_info_.isEmpty()) {
        qWarning() << "No games found in VRP-GameList.txt";
        emit metadataUpdateFailed();
    } else {
        qDebug() << "Metadata updated";
        emit metadataUpdated();
        emit gamesInfoChanged();
    }
}

QString VrpDownloader::getGameId(const QString& release_name) const {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData((release_name + "\n").toUtf8());
    return hash.result().toHex();
}

void VrpDownloader::download(const QString& release_name) {
    if (!ready_to_download_) {
        // TODO: add to queue
        qWarning() << "Not ready to download";
        return;
    }

    qDebug() << "Downloading: " << release_name;
    current_release_name_ = release_name;
    QString id = getGameId(release_name);

    // https://rclone.org/rc/#sync-copy
    QString rc_method("sync/copy");
    QString rc_input = QString(R"({
        "srcFs": ":http,url='%1':/%2",
        "dstFs": "%3/%4",
        "_async": true,
        "_config": {
                "Transfers": 1,
                "TPSLimit": 1.0,
                "TPSLimitBurst": 3,
                "MultiThreadStreams": 0,
                "Progress": true,
                "ProgressTerminalTitle":true,
                "UserAgent": "rclone/v1.65.2"
        }
    })")
                           .arg(QString("https://theapp.vrrookie.xyz/"),
                                id,
                                cache_path_,
                                id);  // TODO:use vrp_public

    RcloneResult result = RcloneRPC(rc_method, rc_input);

    QJsonDocument doc = QJsonDocument::fromJson(result.output().toLocal8Bit());
    current_job_id_ = doc.object()["jobid"].toInt();

    if (result.isSuccessful()) {
        download_status_timer_.start(1000);
    } else {
        emit downloadFailed(release_name);
        qWarning() << "Download game failed :"
                   << "\n\tgame: " << id << "\n\tstatus: " << result.status()
                   << "\n\toutput: " << result.output();
    }
}

void VrpDownloader::checkDownloadStatus() {
    // https://rclone.org/rc/#job-status
    RcloneResult job_status =
        RcloneRPC(QString("job/status"),
                  QString(R"({"jobid":%1})").arg(current_job_id_));
    QJsonDocument doc_job =
        QJsonDocument::fromJson(job_status.output().toLocal8Bit());
    bool is_finished = doc_job.object()["finished"].toBool();

    if (!is_finished) {
        ready_to_download_ = false;
        QString group = doc_job.object()["group"].toString();

        // https://rclone.org/rc/#core-stats
        RcloneResult group_stats =
            RcloneRPC(QString("core/stats"),
                      QString(R"({"group":"%1"})").arg(group));
        QJsonDocument group_stats_doc =
            QJsonDocument::fromJson(group_stats.output().toLocal8Bit());
        double transferred = group_stats_doc.object()["bytes"].toDouble();
        double total = group_stats_doc.object()["totalBytes"].toDouble();
        double speed = group_stats_doc.object()["speed"].toDouble();
        emit downloadProgressChanged(current_release_name_,
                                   transferred / total * 100,
                                   speed);
    } else {
        ready_to_download_ = true;
        download_status_timer_.stop();
        qDebug() << "Download finished: " << current_release_name_;
        emit downloadSucceeded(current_release_name_);
        decompressGame(current_release_name_);
    }
}

void VrpDownloader::decompressGame(const QString& release_name) {
    qDebug() << "Decompressing: " << release_name;

    auto p7za = QSharedPointer<QProcess>::create(this);
    connect(
        p7za.data(),
        &QProcess::finished,
        this,
        [this, p7za, release_name](int exitCode,
                                   QProcess::ExitStatus exitStatus) {
            if (exitStatus != QProcess::NormalExit || p7za->exitCode() != 0) {
                qWarning("Decompression failed: %s\n %s\n %s",
                         release_name.data(),
                         p7za->readAllStandardOutput().data(),
                         p7za->readAllStandardError().data());
                emit decompressFailed(release_name);
            } else {
                qDebug() << "Decompression successful: " << release_name;
                emit decompressSucceeded(release_name);
            }
        });

    // Decompress
    p7za->start("7za",
                QStringList()
                    << "x"
                    << QString("%1/%2/%2.7z.001")
                           .arg(cache_path_, getGameId(release_name))
                    << "-aoa"  // Overwrite All existing files without prompt.
                    << QString("-o%1").arg(data_path_)
                    << QString("-p%1").arg(
                           QString("gL59VfgPxoHR")));  // TODO: use vrp_public
}

QString VrpDownloader::getGameThumbnailPath(const QString& package_name) {
    auto path = data_path_ + "/.meta/thumbnails/" + package_name + ".jpg";
    if (QFile::exists(path)) {
        return path;
    } else {
        // TODO: return a default image
        return data_path_ + "/.meta/thumbnails/jp.co.avex.anicastmaker.jpg";
    }
}