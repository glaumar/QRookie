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
#ifndef QROOKIE_VRP_TORRENT
#define QROOKIE_VRP_TORRENT

#include <QCoroTask>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>

class VrpTorrent : public QObject
{
    Q_OBJECT

public:
    VrpTorrent(QObject *parent = nullptr);
    ~VrpTorrent();
    QCoro::Task<bool> update();
    QString findMagnetURI(const QString &name) const
    {
        return magnet_uri_.value(name);
    }

private:
    QMap<QString, QString> magnet_uri_;
    QNetworkAccessManager manager_;
};

#endif /* QROOKIE_VRP_TORRENT */