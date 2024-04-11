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
#include <QStringListModel>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "game_info_model.h"

class DeviceManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(
        QVariantList devicesList READ devicesList NOTIFY devicesListChanged);
    Q_PROPERTY(QString connectedDevice READ connectedDevice WRITE
                   connectToDevice NOTIFY connectedDeviceChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString deviceIP READ deviceIP NOTIFY deviceIPChanged)
    Q_PROPERTY(long long totalSpace READ totalSpace NOTIFY spaceUsageChanged)
    Q_PROPERTY(long long freeSpace READ freeSpace NOTIFY spaceUsageChanged)
    Q_PROPERTY(double batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)

   public:
    DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();

    Q_INVOKABLE QCoro::Task<bool> startServer();
    Q_INVOKABLE QCoro::Task<bool> restartServer();
    QCoro::Task<bool> installApk(const QString path,
                                 const QString package_name);

    Q_INVOKABLE QCoro::QmlTask installApkQml(const QString path,
                                             const QString package_name) {
        return installApk(path, package_name);
    }

    QCoro::Task<bool> uninstallApk(const QString package_name);
    Q_INVOKABLE QCoro::QmlTask uninstallApkQml(const QString package_name) {
        return uninstallApk(package_name);
    };

    Q_INVOKABLE QVariantList devicesList() const {
        QVariantList list;
        for (auto& device : devices_list_) {
            list.append(device);
        }
        return list;
    }

    Q_INVOKABLE GameInfoModel* appListModel() { return &app_list_model_; }

    Q_INVOKABLE QCoro::Task<void> updateSerials();
    Q_INVOKABLE void updateDeviceInfo();
    Q_INVOKABLE QCoro::Task<void> updateDeviceName();
    Q_INVOKABLE QCoro::Task<void> updateDeviceIP();
    Q_INVOKABLE QCoro::Task<void> updateSpaceUsage();
    Q_INVOKABLE QCoro::Task<void> updateBatteryLevel();
    Q_INVOKABLE QCoro::Task<void> updateAppList();

    Q_INVOKABLE void enableAutoUpdate(const int ms = 3000) {
        auto_update_timer_.start(ms);
    }
    Q_INVOKABLE void disableAutoUpdate() { auto_update_timer_.stop(); }

    Q_INVOKABLE bool hasConnectedDevice() const {
        return !connected_device_.isEmpty();
    }
    Q_INVOKABLE void disconnectDevice() {
        if (!hasConnectedDevice()) return;

        connected_device_.clear();
        emit connectedDeviceChanged();
    }

    Q_INVOKABLE QString connectedDevice() const { return connected_device_; }
    Q_INVOKABLE void connectToDevice(const QString& serial) {
        if (connectedDevice() != serial && devices_list_.contains(serial)) {
            connected_device_ = serial;
            emit connectedDeviceChanged();
        }
    }

    Q_INVOKABLE QString deviceName() const { return device_name_; }
    Q_INVOKABLE void setDeviceName(const QString& device_name) {
        device_name_ = device_name;
        emit deviceNameChanged(device_name_);
    }

    Q_INVOKABLE QString deviceIP() const { return device_ip_; }
    Q_INVOKABLE void setDeviceIP(const QString& device_ip) {
        device_ip_ = device_ip;
        emit deviceIPChanged(device_ip_);
    }

    Q_INVOKABLE long long totalSpace() const { return total_space_; }
    Q_INVOKABLE long long freeSpace() const { return free_space_; }
    Q_INVOKABLE void setSpaceUsage(long long total_space,
                                   long long free_space) {
        total_space_ = total_space;
        free_space_ = free_space;
        emit spaceUsageChanged(total_space, free_space);
    }

    Q_INVOKABLE double batteryLevel() const { return battery_level_; }
    Q_INVOKABLE void setBatteryLevel(double battery_level) {
        battery_level_ = battery_level;
        emit batteryLevelChanged(battery_level_);
    }

   signals:
    void devicesListChanged();
    void appListChanged();
    void connectedDeviceChanged();
    void deviceNameChanged(QString device_name);
    void deviceIPChanged(QString device_ip);
    void spaceUsageChanged(long long total_space, long long free_space);
    void batteryLevelChanged(double battery_level);

   private:
    QStringList devices_list_;
    GameInfoModel app_list_model_;
    QTimer auto_update_timer_;
    QString connected_device_;
    QString device_name_;
    QString device_ip_;
    double battery_level_;
    long long total_space_;
    long long free_space_;
};

#endif /* QROOKIE_DEVICE_MANAGER */
