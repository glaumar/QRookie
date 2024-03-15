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
    // Q_ENUMS(Status)

    Q_PROPERTY(QVariantList serials READ serials_property NOTIFY serialsChanged)

   public:
    DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();

    QCoro::Task<bool> startServer();
    QCoro::Task<bool> restartServer();
    QVector<QString> serials() const { return serials_; }
    QVariantList serials_property() const;

    QCoro::Task<QVector<AppInfo>> installedApps(const QString& serial) const;

    QCoro::Task<QVariantList> deviceApps(const QString& serial) const;
    Q_INVOKABLE QCoro::QmlTask deviceAppsQml(const QString& serial) const {
        return deviceApps(serial);
    }

    QCoro::Task<QVariantMap> deviceApp(const QString& serial,
                                       const QString& package_name) const;
    Q_INVOKABLE QCoro::QmlTask deviceAppQml(const QString& serial,
                                            const QString& package_name) const {
        return deviceApp(serial, package_name);
    }

    QCoro::Task<QString> deviceModel(const QString& serial) const;
    Q_INVOKABLE QCoro::QmlTask deviceModelQml(const QString& serial) const {
        return deviceModel(serial);
    }

    QCoro::Task<QPair<long long, long long>> deviceSpaceUsage(
        const QString& serial) const;

    QCoro::Task<QVariantMap> spaceUsage(const QString& serial) const;
    Q_INVOKABLE QCoro::QmlTask spaceUsageQml(const QString& serial) const {
        return spaceUsage(serial);
    }

    QCoro::Task<bool> installApk(const QString serial, const QString path,
                                 const QString package_name) const;
    Q_INVOKABLE QCoro::QmlTask installApkQml(
        const QString& serial, const QString& path,
        const QString& package_name) const {
        return installApk(serial, path, package_name);
    }

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
