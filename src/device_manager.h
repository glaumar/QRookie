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
