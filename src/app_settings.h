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

#pragma once
#ifndef QROOKIE_APP_SETTINGS
#define QROOKIE_APP_SETTINGS

#include <QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool autoInstall READ autoInstall WRITE setAutoInstall NOTIFY autoInstallChanged)
    Q_PROPERTY(bool autoCleanCache READ autoCleanCache WRITE setAutoCleanCache NOTIFY autoCleanCacheChanged)
    Q_PROPERTY(bool renamePackage READ renamePackage WRITE setRenamePackage NOTIFY renamePackageChanged)
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
    Q_PROPERTY(QString lastWirelessAddr READ lastWirelessAddr WRITE setLastWirelessAddr NOTIFY lastWirelessAddrChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();

    static AppSettings *instance();
    static int const EXIT_RESTART = 1000;

    bool autoInstall() const
    {
        return auto_install_;
    }
    void setAutoInstall(bool auto_install);

    bool autoCleanCache() const
    {
        return auto_clean_cache_;
    }
    void setAutoCleanCache(bool auto_clean_cache);

    bool renamePackage() const
    {
        return rename_package_;
    }
    void setRenamePackage(bool rename_package);

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

    QString keyStorePath() const
    {
        return keystore_path_;
    }
    void setKeyStorePath(const QString &keystore_path);

    QString lastWirelessAddr() const
    {
        return last_wireless_addr_;
    }
    void setLastWirelessAddr(const QString &addr);

    QString theme() const
    {
        return theme_;
    }
    void setTheme(const QString &theme);

signals:
    void autoInstallChanged(bool auto_install);
    void autoCleanCacheChanged(bool auto_clean_cache);
    void renamePackageChanged(bool rename_package);
    void cachePathChanged(QString cache_path);
    void dataPathChanged(QString data_path);
    void keyStorePathChanged(QString keystore_path);
    void lastWirelessAddrChanged(QString addr);
    void themeChanged(QString theme);

private:
    void loadAppSettings();

    QSettings *settings_;
    bool auto_install_;
    bool auto_clean_cache_;
    bool rename_package_;
    QString cache_path_;
    QString data_path_;
    QString keystore_path_;
    QString last_wireless_addr_;
    QString theme_;
};

#endif /* QROOKIE_APP_SETTINGS */
