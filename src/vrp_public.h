#pragma once
#ifndef QROOKIE_VRP_PUBLIC
#define QROOKIE_VRP_PUBLIC

#include <QByteArray>
#include <QCoroTask>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QString>

class VrpPublic : public QObject {
    Q_OBJECT
   public:
    VrpPublic(QObject *parent = nullptr)
        : QObject(parent), base_url_(""), password_(""), manager_(nullptr) {}
    QCoro::Task<bool> update();
    QString baseUrl() const { return base_url_; }
    QString password() const { return password_; }

   private:
    QCoro::Task<QPair<bool, QByteArray>> downloadJson(const QString url);
    QPair<QString, QString> parseJson(const QByteArray &json);

    QString base_url_;
    QString password_;
    QNetworkAccessManager manager_;
};

#endif /* QROOKIE_VRP_PUBLIC */
