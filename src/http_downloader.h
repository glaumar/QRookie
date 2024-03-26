#ifndef QROOKIE_HTTP_DOWNLOADER
#define QROOKIE_HTTP_DOWNLOADER
#include <QCoroTask>
#include <QNetworkAccessManager>
#include <QObject>
#include <QStack>
#include <QString>

class HttpDownloader : public QObject {
    Q_OBJECT
   public:
    HttpDownloader(QObject* parent = nullptr);
    QString baseUrl() const { return base_url_; }
    void setBaseUrl(const QString& url) { base_url_ = url; }
    QString downloadDirectory() const { return download_directory_; }
    void setDownloadDirectory(const QString& directory) {
        download_directory_ = directory;
    }

    // Download a file from the server
    QCoro::Task<bool> download(const QString file_path);
    void abortDownload(const QString file_path) {
        abort_files_.push(file_path);
    }
    // Download a directory from the server
    QCoro::Task<bool> downloadDir(const QString dir_path);
    void abortDownloadDir(const QString dir_path) {
        abort_dirs_.push(dir_path);
    }

   signals:
    void downloadProgress(QString filename, qint64 bytes_received,
                          qint64 bytes_total);
    void downloadProgressDir(QString dir_name, qint64 bytes_received,
                             qint64 bytes_total);

   private:
    QNetworkAccessManager manager_;
    QString download_directory_;
    QString base_url_;
    QStack<QString> abort_files_;
    QStack<QString> abort_dirs_;
};

#endif /* QROOKIE_HTTP_DOWNLOADER */
