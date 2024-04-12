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
#ifndef QROOKIE_GAME_INFO
#define QROOKIE_GAME_INFO
#include <QObject>
#include <QString>

struct GameInfo {
    bool operator==(const GameInfo &other) const
    {
        return release_name == other.release_name && package_name == other.package_name && version_code == other.version_code;
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

inline bool operator<(const GameInfo &lhs, const GameInfo &rhs)
{
    return lhs.release_name < rhs.release_name;
}

#endif /* QROOKIE_GAME_INFO */
