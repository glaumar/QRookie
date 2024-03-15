#ifndef QROOKIE_GAME_INFO
#define QROOKIE_GAME_INFO
#include <QObject>
#include <QString>

struct GameInfo {
    bool operator==(const GameInfo& other) const {
        return release_name == other.release_name;
    }
    QString name;
    QString release_name;
    QString package_name;
    QString version_code;
    QString last_updated;
    QString size;

    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString release_name MEMBER release_name)
    Q_PROPERTY(QString package_name MEMBER package_name)
    Q_PROPERTY(QString version_code MEMBER version_code)
    Q_PROPERTY(QString last_updated MEMBER last_updated)
    Q_PROPERTY(QString size MEMBER size)
};
#endif /* QROOKIE_GAME_INFO */
