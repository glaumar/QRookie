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
    , auto_clean_cache_(true)
    , rename_package_(false)
    , cache_path_(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
    , data_path_(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
    , keystore_path_(QString())
    , last_wireless_addr_(QString())
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
    auto_clean_cache_ = settings_->value("auto_clean_cache", auto_clean_cache_).toBool();

    QDir dir;
    cache_path_ = settings_->value("cache_path", cache_path_).toString();
    if (!dir.exists(cache_path_)) {
        dir.mkpath(cache_path_);
    }

    data_path_ = settings_->value("data_path", data_path_).toString();
    if (!dir.exists(data_path_)) {
        dir.mkpath(data_path_);
    }

    keystore_path_ = settings_->value("keystore_path", keystore_path_).toString();
    if (keystore_path_.isEmpty()) {
        keystore_path_ = QStandardPaths::locate(QStandardPaths::AppDataLocation, "qrookie.keystore");
    }

    last_wireless_addr_ = settings_->value("last_wireless_addr", last_wireless_addr_).toString();
}

void AppSettings::setAutoInstall(bool auto_install)
{
    auto_install_ = auto_install;
    settings_->setValue("auto_install", auto_install_);
    emit autoInstallChanged(auto_install);
}

void AppSettings::setAutoCleanCache(bool auto_clean_cache)
{
    auto_clean_cache_ = auto_clean_cache;
    settings_->setValue("auto_clean_cache", auto_clean_cache_);
    emit autoCleanCacheChanged(auto_clean_cache);
}

void AppSettings::setRenamePackage(bool rename_package)
{
    rename_package_ = rename_package;
    settings_->setValue("rename_package", rename_package_);
    emit renamePackageChanged(rename_package);
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

void AppSettings::setKeyStorePath(const QString &keystore_path)
{
    keystore_path_ = keystore_path;
    settings_->setValue("keystore_path", keystore_path_);
    emit keyStorePathChanged(keystore_path);
}

void AppSettings::setLastWirelessAddr(const QString &addr)
{
    last_wireless_addr_ = addr;
    settings_->setValue("last_wireless_addr", last_wireless_addr_);
    emit lastWirelessAddrChanged(addr);
}