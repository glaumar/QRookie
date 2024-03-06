#include "vrp_downloader.h"

#include <QDir>
#include <QProcess>
#include <QStandardPaths>

#include "qrookie.h"
#include "rclone_result.h"

VrpDownloader::VrpDownloader(QObject* parent) : QObject(parent) {
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

    // decompression meta.7z finished
    connect(
        &p7za_,
        &QProcess::finished,
        this,
        [this](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus != QProcess::NormalExit || p7za_.exitCode() != 0) {
                qWarning("meta.7z decompression failed: %s\n %s",
                         p7za_.readAllStandardOutput().data(),
                         p7za_.readAllStandardError().data());
                emit metadataUpdateFailed();
            } else {
                qDebug() << "meta.7z decompression successful";
                parseMetadata();
            }
        });
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
        // Decompress meta.7z
        p7za_.start(
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

QString VrpDownloader::getGameThumbnailPath(const QString& package_name) {
    auto path = data_path_ + "/.meta/thumbnails/" + package_name + ".jpg";
    if (QFile::exists(path)) {
        return path;
    } else {
        // TODO: return a default image
        return data_path_ + "/.meta/thumbnails/jp.co.avex.anicastmaker.jpg";
    }
}