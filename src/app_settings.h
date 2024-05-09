#pragma once
#ifndef QROOKIE_APP_SETTINGS
#define QROOKIE_APP_SETTINGS

#include <QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool autoInstall READ autoInstall WRITE setAutoInstall NOTIFY autoInstallChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();

    static AppSettings *instance();

    bool autoInstall() const
    {
        return auto_install_;
    }

    void setAutoInstall(bool auto_install);

signals:
    void autoInstallChanged(bool auto_install);

private:
    void loadAppSettings();

    QSettings *settings_;
    bool auto_install_;
};

#endif /* QROOKIE_APP_SETTINGS */
