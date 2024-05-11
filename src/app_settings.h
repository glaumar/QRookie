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
    Q_PROPERTY(QString cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
    Q_PROPERTY(QString dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
    Q_PROPERTY(QString lastWirelessAddr READ lastWirelessAddr WRITE setLastWirelessAddr NOTIFY lastWirelessAddrChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();

    static AppSettings *instance();

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

    QString lastWirelessAddr() const
    {
        return last_wireless_addr;
    }
    void setLastWirelessAddr(const QString &addr);

signals:
    void autoInstallChanged(bool auto_install);
    void autoCleanCacheChanged(bool auto_clean_cache);
    void cachePathChanged(QString cache_path);
    void dataPathChanged(QString data_path);
    void lastWirelessAddrChanged(QString addr);

private:
    void loadAppSettings();

    QSettings *settings_;
    bool auto_install_;
    bool auto_clean_cache_;
    QString cache_path_;
    QString data_path_;
    QString last_wireless_addr;
};

#endif /* QROOKIE_APP_SETTINGS */
