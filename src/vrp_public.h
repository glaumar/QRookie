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
#ifndef QROOKIE_VRP_PUBLIC
#define QROOKIE_VRP_PUBLIC

#include <QByteArray>
#include <QCoroTask>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QString>

class VrpPublic : public QObject
{
    Q_OBJECT
public:
    VrpPublic(QObject *parent = nullptr)
        : QObject(parent)
        , base_url_("https://theapp.vrrookie.xyz/")
        , password_("gL59VfgPxoHR")
        , manager_(nullptr)
    {
    }
    QCoro::Task<bool> update();
    QString baseUrl() const
    {
        return base_url_;
    }
    QString password() const
    {
        return password_;
    }

private:
    QCoro::Task<QPair<bool, QByteArray>> downloadJson(const QString url);
    QPair<QString, QString> parseJson(const QByteArray &json);

    QString base_url_;
    QString password_;
    QNetworkAccessManager manager_;
};

#endif /* QROOKIE_VRP_PUBLIC */
