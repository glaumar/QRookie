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