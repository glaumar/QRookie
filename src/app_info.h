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
