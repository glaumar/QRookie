#ifndef QROOKIE_DEVICE_MANAGER
#define QROOKIE_DEVICE_MANAGER
#include <QCoroProcess>
#include <QObject>
#include <QTimer>
#include <QVariantList>

class DeviceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList serials READ serials_property NOTIFY serialsChanged)

   public:
    DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();
    QCoro::Task<bool> startServer();
    QCoro::Task<bool> restartServer();
    QVector<QString> serials() const { return serials_; }
    QVariantList serials_property() const;

    // return: QPair<pacakgeName, versionCode>
    QCoro::Task<QVector<QPair<QString, long>>> deviceApps(QString serial);

    QCoro::Task<QString> deviceModel(QString serial);

    // return: QPair<totalSpace (KB), freeSpace (KB)>
    QCoro::Task<QPair<long, long>> spaceUsage(QString serial);

    QCoro::Task<bool> installApk(QString serial, QString path, QString package_name);

    Q_INVOKABLE void autoUpdateSerials(int ms = 3000) {
        update_serials_timer_.start(ms);
    }
    Q_INVOKABLE void stopUpdateSerials() { update_serials_timer_.stop(); }


   signals:
    void serialsChanged();

   private:
    QCoro::Task<void> updateSerials();
    QVector<QString> getSerials();
    QVector<QString> serials_;
    QTimer update_serials_timer_;
};

#endif /* QROOKIE_DEVICE_MANAGER */
