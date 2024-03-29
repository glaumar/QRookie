#pragma once
#ifndef QROOKIE_DEVICE_MANAGER
#define QROOKIE_DEVICE_MANAGER
#include <QCoroProcess>
#include <QCoroQmlTask>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "app_info.h"

class DeviceManager : public QObject {
    Q_OBJECT

   public:
    DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();

    QCoro::Task<bool> startServer();
    QCoro::Task<bool> restartServer();
    QVector<QString> serials() const { return serials_; }

    QCoro::Task<QVector<AppInfo>> installedApps(const QString& serial) const;

    QCoro::Task<QString> deviceModel(const QString& serial) const;

    QCoro::Task<QPair<long long, long long>> spaceUsage(
        const QString& serial) const;

    QCoro::Task<bool> installApk(const QString serial, const QString path,
                                 const QString package_name) const;

    QCoro::Task<bool> uninstallApk(const QString serial,
                                   const QString package_name) const;

    Q_INVOKABLE void autoUpdateSerials(const int ms = 3000) {
        update_serials_timer_.start(ms);
    }
    Q_INVOKABLE void stopUpdateSerials() { update_serials_timer_.stop(); }

   signals:
    void serialsChanged();

   private:
    QCoro::Task<void> updateSerials();
    QVector<QString> serials_;
    QTimer update_serials_timer_;
};

#endif /* QROOKIE_DEVICE_MANAGER */
