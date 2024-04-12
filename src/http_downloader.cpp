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

#include "http_downloader.h"

#include <QCoroIODevice>
#include <QCoroNetworkReply>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSharedPointer>

HttpDownloader::HttpDownloader(QObject *parent)
    : QObject(parent)
    , manager_(nullptr)
    , download_directory_("./")
    , base_url_("https://theapp.vrrookie.xyz/")
{
}

QCoro::Task<bool> HttpDownloader::download(const QString file_path)
{
    QString filename = download_directory_ + "/" + file_path;
    QString tmp_filename = filename + ".tmp";

    if (QFile::exists(filename)) {
        qDebug() << "File already exists: " << filename;
        co_return true;
    }

    QFile file(tmp_filename);
    QIODeviceBase::OpenMode open_mode;
    qint64 downloaded_bytes_ = 0;

    if (file.exists()) {
        qDebug() << "Temp file exists: " << tmp_filename;
        open_mode = QIODevice::WriteOnly | QIODevice::Append;
        downloaded_bytes_ = file.size();
    } else {
        open_mode = QIODevice::WriteOnly | QIODevice::Truncate;
    }

    if (!file.open(open_mode)) {
        qWarning() << "Failed to open file: " << file.fileName();
        co_return false;
    }

    QUrl url = base_url_ + file_path;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "rclone/v1.65.2");
    if (downloaded_bytes_ > 0) {
        request.setRawHeader("Range", QString("bytes=%1-").arg(downloaded_bytes_).toUtf8());
    }
    auto *reply = manager_.get(request);
    auto device = qCoro(dynamic_cast<QIODevice *>(reply));
    connect(reply, &QNetworkReply::downloadProgress, this, [this, file_path, downloaded_bytes_](qint64 bytes_received, qint64 bytes_total) {
        emit downloadProgress(file_path, bytes_received + downloaded_bytes_, bytes_total + downloaded_bytes_);
    });

    qDebug() << "Downloading: " << url;
    bool result = false;
    while (true) {
        if (!abort_files_.isEmpty() && abort_files_.top() == file_path) {
            abort_files_.pop();
            break;
        }

        if (!abort_dirs_.isEmpty() && file_path.startsWith(abort_dirs_.top())) {
            abort_dirs_.pop();
            break;
        }

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Downloading Error: " << reply->errorString();
            break;
        }

        if (reply->isFinished()) {
            file.write(reply->readAll());
            result = true;
            qDebug() << "Downloaded: " << url;

            file.rename(filename);
            break;
        }

        co_await device.waitForReadyRead(1000);
        file.write(reply->readAll());
    }

    reply->deleteLater();
    co_return result;
}

QCoro::Task<bool> HttpDownloader::downloadDir(const QString dir_path)
{
    QUrl url = base_url_ + dir_path + "/";
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "rclone/v1.65.2");
    auto *reply = co_await manager_.get(request);
    auto html = co_await qCoro(reply).readAll();

    QDomDocument doc;
    doc.setContent(html);
    auto pre_tag = doc.elementsByTagName("pre");
    if (pre_tag.isEmpty()) {
        qWarning() << "Downloading Error: No pre tag found: " << url;
        co_return false;
    }

    QString pre_text = pre_tag.at(0).toElement().text();

    QStringList lines = pre_text.split("\n");
    /*
    Line Example:
        "c383d4bed11d5d96411920f0451f80da.7z.001           18-Dec-2023 01:46
    524288000"
    */
    QRegularExpression re(R"_(^(../)?([0-9a-z]+\.7z\.\d+)+.*\s+(\d+)$)_");

    long long total_size = 0;
    QVector<QPair<QString, long long>> files;
    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            QString name = match.captured(2);
            QString size = match.captured(3);
            total_size += size.toLongLong();
            files.push_back({name, size.toLongLong()});
        }
    }

    if (files.isEmpty()) {
        qWarning() << "Downloading Error: No files found: " << url;
        co_return false;
    }

    emit downloadProgressDir(dir_path, 0, total_size);

    auto total_received = QSharedPointer<long long>::create(0);

    auto conn = connect(this,
                        &HttpDownloader::downloadProgress,
                        [this, dir_path, total_size, total_received](QString filename, qint64 bytes_received, qint64 bytes_total) {
                            if (filename.startsWith(dir_path)) {
                                emit downloadProgressDir(dir_path, *total_received + bytes_received, total_size);
                            }
                        });

    QDir dir(downloadDirectory() + "/" + dir_path);
    if (!dir.exists()) {
        dir.mkpath(downloadDirectory() + "/" + dir_path);
    }

    bool result = true;
    for (const auto &[name, size] : files) {
        if (!co_await download(dir_path + "/" + name)) {
            qDebug() << "Download failed: " << dir_path;
            result = false;
            break;
        }
        *total_received += size;
    }

    if (result) {
        emit downloadProgressDir(dir_path, total_size, total_size);
    }
    disconnect(conn);
    co_return result;
}
