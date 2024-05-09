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

#include "app_settings.h"
#include "qrookie.h"
#include <QDir>
#include <QStandardPaths>

Q_GLOBAL_STATIC(AppSettings, kGlobalAppSettings)

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , settings_(new QSettings(APPLICATION_NAME, APPLICATION_NAME, this))
    , auto_install_(true)
    , cache_path_(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
    , data_path_(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
    , last_wireless_addr(QString())
{
    loadAppSettings();
}

AppSettings::~AppSettings()
{
    settings_->sync();
}

AppSettings *AppSettings::instance()
{
    return kGlobalAppSettings();
}

void AppSettings::loadAppSettings()
{
    auto_install_ = settings_->value("auto_install", auto_install_).toBool();

    QDir dir;
    cache_path_ = settings_->value("cache_path", cache_path_).toString();
    if (!dir.exists(cache_path_)) {
        dir.mkpath(cache_path_);
    }

    data_path_ = settings_->value("data_path", data_path_).toString();
    if (!dir.exists(data_path_)) {
        dir.mkpath(data_path_);
    }

    last_wireless_addr = settings_->value("last_wireless_addr", last_wireless_addr).toString();
}

void AppSettings::setAutoInstall(bool auto_install)
{
    auto_install_ = auto_install;
    settings_->setValue("auto_install", auto_install_);
    emit autoInstallChanged(auto_install);
}

void AppSettings::setCachePath(const QString &cache_path)
{
    cache_path_ = cache_path;
    settings_->setValue("cache_path", cache_path_);
    QDir dir;
    if (!dir.exists(cache_path_)) {
        dir.mkpath(cache_path_);
    }
    emit cachePathChanged(cache_path);
}

void AppSettings::setDataPath(const QString &data_path)
{
    data_path_ = data_path;
    settings_->setValue("data_path", data_path_);
    QDir dir;
    if (!dir.exists(data_path_)) {
        dir.mkpath(data_path_);
    }
    emit dataPathChanged(data_path);
}

void AppSettings::setLastWirelessAddr(const QString &addr)
{
    last_wireless_addr = addr;
    settings_->setValue("last_wireless_addr", last_wireless_addr);
    emit lastWirelessAddrChanged(addr);
}