#pragma once
#ifndef QROOKIE_APP_SETTINGS
#define QROOKIE_APP_SETTINGS

#include <QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool autoInstall READ autoInstall WRITE setAutoInstall NOTIFY autoInstallChanged)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();

    static AppSettings *instance();

    bool autoInstall() const
    {
        return auto_install_;
    }
    void setAutoInstall(bool auto_install);

    QString cachePath() const
    {
        return cache_path_;
    }
    void setCachePath(const QString &cache_path);

    QString dataPath() const
    {
        return data_path_;
    }
    void setDataPath(const QString &data_path);

signals:
    void autoInstallChanged(bool auto_install);
    void cachePathChanged(const QString &cache_path);
    void dataPathChanged(const QString &data_path);

private:
    void loadAppSettings();

    QSettings *settings_;
    bool auto_install_;
    QString cache_path_;
    QString data_path_;
};

#endif /* QROOKIE_APP_SETTINGS */
