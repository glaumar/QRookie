#include "app_settings.h"
#include "qrookie.h"

Q_GLOBAL_STATIC(AppSettings, kGlobalAppSettings)

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , settings_(new QSettings(APPLICATION_NAME, APPLICATION_NAME, this))
    , auto_install_(true)
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
}

void AppSettings::setAutoInstall(bool auto_install)
{
    auto_install_ = auto_install;
    settings_->setValue("auto_install", auto_install_);
    emit autoInstallChanged(auto_install);
}
