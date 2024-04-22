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

#include "game_info_model.h"
#include <QCoroProcess>
#include <QCoroQmlTask>
#include <QVariantList>

class DeviceManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList devicesList READ devicesList NOTIFY devicesListChanged);
    Q_PROPERTY(QString connectedDevice READ connectedDevice WRITE connectToDevice NOTIFY connectedDeviceChanged)
    Q_PROPERTY(bool hasConnectedDevice READ hasConnectedDevice NOTIFY connectedDeviceChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString deviceIp READ deviceIp NOTIFY deviceIpChanged)
    Q_PROPERTY(long long totalSpace READ totalSpace NOTIFY spaceUsageChanged)
    Q_PROPERTY(long long freeSpace READ freeSpace NOTIFY spaceUsageChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(QString oculusOsVersion READ oculusOsVersion NOTIFY oculusOsVersionChanged)
    Q_PROPERTY(QString oculusVersion READ oculusVersion NOTIFY oculusVersionChanged)
    Q_PROPERTY(QString oculusRuntimeVersion READ oculusRuntimeVersion NOTIFY oculusRuntimeVersionChanged)
    Q_PROPERTY(int androidVersion READ androidVersion NOTIFY androidVersionChanged)
    Q_PROPERTY(int androidSdkVersion READ androidSdkVersion NOTIFY androidSdkVersionChanged)

public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();

    Q_INVOKABLE QCoro::Task<bool> startServer();
    Q_INVOKABLE QCoro::Task<bool> restartServer();
    QCoro::Task<bool> installApk(const QString path, const QString package_name);

    Q_INVOKABLE QCoro::QmlTask installApkQml(const QString path, const QString package_name)
    {
        return installApk(path, package_name);
    }

    QCoro::Task<bool> uninstallApk(const QString package_name);
    Q_INVOKABLE QCoro::QmlTask uninstallApkQml(const QString package_name)
    {
        return uninstallApk(package_name);
    };

    Q_INVOKABLE QVariantList devicesList() const
    {
        QVariantList list;
        for (auto &device : devices_list_) {
            list.append(device);
        }
        return list;
    }

    Q_INVOKABLE GameInfoModel *appListModel()
    {
        return &app_list_model_;
    }

    Q_INVOKABLE QCoro::Task<void> updateSerials();
    Q_INVOKABLE void updateDeviceInfo();
    Q_INVOKABLE QCoro::Task<void> updateDeviceName();
    Q_INVOKABLE QCoro::Task<void> updatedeviceIp();
    Q_INVOKABLE QCoro::Task<void> updateSpaceUsage();
    Q_INVOKABLE QCoro::Task<void> updateBatteryLevel();
    Q_INVOKABLE QCoro::Task<void> updateOculusOsVersion();
    Q_INVOKABLE QCoro::Task<void> updateOculusVersion();
    Q_INVOKABLE QCoro::Task<void> updateOculusRuntimeVersion();
    Q_INVOKABLE QCoro::Task<void> updateAndroidVersion();
    Q_INVOKABLE QCoro::Task<void> updateAndroidSdkVersion();
    Q_INVOKABLE QCoro::Task<void> updateAppList();

    Q_INVOKABLE void enableAutoUpdate(const int ms = 3000)
    {
        auto_update_timer_.start(ms);
    }
    Q_INVOKABLE void disableAutoUpdate()
    {
        auto_update_timer_.stop();
    }

    Q_INVOKABLE bool hasConnectedDevice() const
    {
        return !connected_device_.isEmpty();
    }
    Q_INVOKABLE void disconnectDevice()
    {
        if (!hasConnectedDevice())
            return;

        connected_device_.clear();
        emit connectedDeviceChanged();
    }

    Q_INVOKABLE QString connectedDevice() const
    {
        return connected_device_;
    }
    Q_INVOKABLE void connectToDevice(const QString &serial)
    {
        if (serial.isEmpty())
            return;

        if (connectedDevice() == serial) {
            qDebug() << "Already connected to device" << serial;
            return;
        }

        if (!devices_list_.contains(serial)) {
            qDebug() << "Device not found" << serial;
            return;
        }

        connected_device_ = serial;
        qDebug() << "Connected to device" << serial;
        emit connectedDeviceChanged();
    }

    Q_INVOKABLE QCoro::Task<bool> connectToWirelessDevice(const QString address /*host[:port]*/);
    Q_INVOKABLE QCoro::QmlTask connectToWirelessDeviceQml(const QString &address)
    {
        return connectToWirelessDevice(address);
    }

    Q_INVOKABLE QCoro::Task<bool> enableTcpMode(int port = 5555);
    Q_INVOKABLE QCoro::QmlTask enableTcpModeQml(int port = 5555)
    {
        return enableTcpMode(port);
    }

    Q_INVOKABLE QString deviceName() const
    {
        return device_name_;
    }
    Q_INVOKABLE void setDeviceName(const QString &device_name)
    {
        device_name_ = device_name;
        emit deviceNameChanged(device_name_);
    }

    Q_INVOKABLE QString deviceIp() const
    {
        return device_ip_;
    }
    Q_INVOKABLE void setdeviceIp(const QString &device_ip)
    {
        device_ip_ = device_ip;
        emit deviceIpChanged(device_ip_);
    }

    Q_INVOKABLE long long totalSpace() const
    {
        return total_space_;
    }
    Q_INVOKABLE long long freeSpace() const
    {
        return free_space_;
    }
    Q_INVOKABLE void setSpaceUsage(long long total_space, long long free_space)
    {
        total_space_ = total_space;
        free_space_ = free_space;
        emit spaceUsageChanged(total_space, free_space);
    }

    Q_INVOKABLE double batteryLevel() const
    {
        return battery_level_;
    }
    Q_INVOKABLE void setBatteryLevel(int battery_level)
    {
        battery_level_ = battery_level;
        emit batteryLevelChanged(battery_level_);
    }

    Q_INVOKABLE QString oculusOsVersion() const
    {
        return oculus_os_version_;
    }
    Q_INVOKABLE void setOculusOsVersion(const QString &oculus_os_version)
    {
        oculus_os_version_ = oculus_os_version;
        emit oculusOsVersionChanged(oculus_os_version_);
    }

    Q_INVOKABLE QString oculusVersion() const
    {
        return oculus_version_;
    }
    Q_INVOKABLE void setOculusVersion(const QString &oculus_version)
    {
        oculus_version_ = oculus_version;
        emit oculusVersionChanged(oculus_version_);
    }

    Q_INVOKABLE QString oculusRuntimeVersion() const
    {
        return oculus_runtime_version_;
    }
    Q_INVOKABLE void setOculusRuntimeVersion(const QString &oculus_runtime_version)
    {
        oculus_runtime_version_ = oculus_runtime_version;
        emit oculusRuntimeVersionChanged(oculus_runtime_version_);
    }

    Q_INVOKABLE int androidVersion() const
    {
        return android_version_;
    }
    Q_INVOKABLE void setAndroidVersion(int android_version)
    {
        android_version_ = android_version;
        emit androidVersionChanged(android_version_);
    }

    Q_INVOKABLE int androidSdkVersion() const
    {
        return android_sdk_version_;
    }
    Q_INVOKABLE void setAndroidSdkVersion(int android_sdk_version)
    {
        android_sdk_version_ = android_sdk_version;
        emit androidSdkVersionChanged(android_sdk_version_);
    }

signals:
    void devicesListChanged();
    void appListChanged();
    void connectedDeviceChanged();
    void deviceNameChanged(QString device_name);
    void deviceIpChanged(QString device_ip);
    void spaceUsageChanged(long long total_space, long long free_space);
    void batteryLevelChanged(int battery_level);
    void oculusOsVersionChanged(QString oculus_os_version);
    void oculusVersionChanged(QString oculus_version);
    void oculusRuntimeVersionChanged(QString oculus_runtime_version);
    void androidVersionChanged(int android_version);
    void androidSdkVersionChanged(int android_sdk_version);

private:
    QStringList devices_list_;
    GameInfoModel app_list_model_;
    QTimer auto_update_timer_;
    QString connected_device_;
    QString device_name_;
    QString device_ip_;
    int battery_level_;
    long long total_space_;
    long long free_space_;
    QString oculus_os_version_;
    QString oculus_version_;
    QString oculus_runtime_version_;
    int android_version_;
    int android_sdk_version_;
};

#endif /* QROOKIE_DEVICE_MANAGER */
