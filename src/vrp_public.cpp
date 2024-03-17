#include "vrp_public.h"

#include <QCoroNetworkReply>
#include <QCoroSignal>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

QCoro::Task<bool> VrpPublic::update() {
    // TODO: save vrp-public.json to local file
    // TODO: load vrp-public.json from local file when update failed
    static const QVector<QString> urls = {
        "https://raw.githubusercontent.com/vrpyou/quest/main/vrp-public.json",
        "https://vrpirates.wiki/downloads/vrp-public.json"};

    for (auto url : urls) {
        qDebug() << "Downloading vrp-public.json from " << url;
        auto [success, data] = co_await downloadJson(url);
        if (!success) {
            continue;
        }

        auto [base_url, password] = parseJson(data);
        if (!base_url.isEmpty() && !password.isEmpty()) {
            base_url_ = base_url;
            password_ = password;
            co_return true;
        }
    }
}

QCoro::Task<QPair<bool, QByteArray>> VrpPublic::downloadJson(
    const QString url) {
    QNetworkRequest request(url);
    auto *reply = manager_.get(request);
    reply->ignoreSslErrors();
    co_await qCoro(reply, &QNetworkReply::finished);

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Downloading json Error: " << reply->errorString();
        co_return QPair<bool, QByteArray>(false, QByteArray());
    }
    auto data = reply->readAll();
    delete reply;
    co_return QPair<bool, QByteArray>(true, data);
}

QPair<QString, QString> VrpPublic::parseJson(const QByteArray &json) {
    /*
    vrp-public.json example:
    {
        "baseUri":"https://theapp.vrrookie.xyz/",
        "password":"Z0w1OVZmZ1B4b0hS"
    }
    */

    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (doc.isNull()) {
        qDebug() << "Parsing vrp-public.json Error: invalid json: " << json;

        return QPair<QString, QString>("", "");
    }

    QJsonObject obj = doc.object();
    if (obj.isEmpty()) {
        qDebug() << "Parsing vrp-public.json Error: empty json: " << json;
        return QPair<QString, QString>("", "");
    }

    QString base_url = obj["baseUri"].toString();
    // password is base64 encoded
    QString password =
        QString(QByteArray::fromBase64(obj["password"].toString().toUtf8()));

    qDebug() << "Parsed vrp-public.json: baseUri=" << base_url
             << ", password=" << password;
    return QPair<QString, QString>(base_url, password);
}
