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
#ifndef QROOKIE_GAME_INFO_MODEL
#define QROOKIE_GAME_INFO_MODEL

#include <QAbstractListModel>
#include <QList>

#include "game_info.h"
class VrpDownloader;

class GameInfoModel : public QAbstractListModel {
    Q_OBJECT

   public:
    enum RoleNames {
        nameRole = Qt::UserRole + 1,
        releaseNameRole,
        packageNameRole,
        versionCodeRole,
        lastUpdatedRole,
        sizeRole,
        GameInfoRole,
    };

    explicit GameInfoModel(QObject* parent = nullptr);
    virtual ~GameInfoModel() {}
    Q_INVOKABLE void insert(int index, const GameInfo& game);
    Q_INVOKABLE void prepend(const GameInfo& game);
    Q_INVOKABLE void append(const GameInfo& game);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void remove(const GameInfo& game);
    Q_INVOKABLE void clear();

    size_t size() const { return games_info_.size(); }

    GameInfo& operator[](size_t index) { return games_info_[index]; }
    const GameInfo& operator[](size_t index) const {
        return games_info_[index];
    }


    virtual int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;

   signals:
    void removed(const GameInfo game);

   protected:
    virtual QHash<int, QByteArray> roleNames() const override;

   private:
    QList<GameInfo> games_info_;
    QHash<int, QByteArray> role_names_;
};

#endif /* QROOKIE_GAME_INFO_MODEL */
