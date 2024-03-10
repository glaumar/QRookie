#include "vrp_public.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

const QString VrpPublic::VRP_PUBLIC_JSON_URL =
    "https://raw.githubusercontent.com/vrpyou/quest/main/vrp-public.json";
const QString VrpPublic::VRP_PUBLIC_JSON_URL_FALLBACK =
    "https://vrpirates.wiki/downloads/vrp-public.json";

void VrpPublic::update() {
    QNetworkRequest request(VRP_PUBLIC_JSON_URL);
    auto *reply = manager_.get(request);
    qDebug() << "Downloading vrp-public.json from " << VRP_PUBLIC_JSON_URL;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Downloading vrp-public.json Error: "
                     << reply->errorString();

            // try to download vrp-public.json from fallback url
            QNetworkRequest request_fallback(VRP_PUBLIC_JSON_URL_FALLBACK);
            auto reply_fallback = manager_.get(request_fallback);

            qDebug() << "Downloading vrp-public.json from "
                     << VRP_PUBLIC_JSON_URL_FALLBACK;

            // I don't know why download from fallback url will cause ssl error
            reply_fallback->ignoreSslErrors();

            connect(reply_fallback,
                    &QNetworkReply::finished,
                    this,
                    [this, reply_fallback]() {
                        if (reply_fallback->error() != QNetworkReply::NoError) {
                            qDebug() << "Downloading vrp-public.json Error: "
                                     << reply_fallback->errorString();
                            emit failed();
                        } else {
                            parseJson(reply_fallback->readAll());
                        }
                    });
        } else {
            parseJson(reply->readAll());
        }
    });
}

void VrpPublic::parseJson(const QByteArray &json) {
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
        emit failed();
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.isEmpty()) {
        qDebug() << "Parsing vrp-public.json Error: empty json: " << json;
        emit failed();
        return;
    }

    base_url_ = obj["baseUri"].toString();
    // password is base64 encoded
    password_ =
        QString(QByteArray::fromBase64(obj["password"].toString().toUtf8()));

    qDebug() << "Parsed vrp-public.json: baseUri=" << base_url_
             << ", password=" << password_;
    emit updated();
}
