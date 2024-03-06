#ifndef QROOKIE_VRP_PUBLIC
#define QROOKIE_VRP_PUBLIC

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class VrpPublic : public QObject {
    Q_OBJECT
   public:
    VrpPublic(QObject *parent = nullptr)
        : QObject(parent), base_url_(""), password_(""), manager_(nullptr) {}
    void update();
    QString baseUrl() const { return base_url_; }
    QString password() const { return password_; }

   signals:
    void updated();
    void failed();

   private:
    void parseJson(const QByteArray &json);

    QString base_url_;
    QString password_;
    QNetworkAccessManager manager_;
    static const QString VRP_PUBLIC_JSON_URL;
    static const QString VRP_PUBLIC_JSON_URL_FALLBACK;
};

#endif /* QROOKIE_VRP_PUBLIC */
