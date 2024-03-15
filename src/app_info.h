#ifndef QROOKIE_APP_INFO
#define QROOKIE_APP_INFO
#include <QObject>
#include <QString>

struct AppInfo {
    bool operator==(const AppInfo& other) const {
        return package_name == other.package_name &&
               version_code == other.version_code;
    }

    QString package_name;
    qlonglong version_code;

    Q_GADGET
    Q_PROPERTY(QString package_name MEMBER package_name)
    Q_PROPERTY(qlonglong version_code MEMBER version_code)
};

#endif /* QROOKIE_APP_INFO */
