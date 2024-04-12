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

#include "game_info_model.h"

#include "vrp_manager.h"

GameInfoModel::GameInfoModel(QObject *parent)
    : QAbstractListModel(parent)
{
    role_names_[nameRole] = "name";
    role_names_[releaseNameRole] = "release_name";
    role_names_[packageNameRole] = "package_name";
    role_names_[versionCodeRole] = "version_code";
    role_names_[lastUpdatedRole] = "last_updated";
    role_names_[sizeRole] = "size";
    role_names_[GameInfoRole] = "game_info";
}

int GameInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return games_info_.size();
}

QVariant GameInfoModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= games_info_.size() || row < 0) {
        return QVariant();
    }

    auto game_info = games_info_.at(row);
    switch (role) {
    case nameRole:
        return game_info.name;
    case releaseNameRole:
        return game_info.release_name;
    case packageNameRole:
        return game_info.package_name;
    case versionCodeRole:
        return game_info.version_code;
    case lastUpdatedRole:
        return game_info.last_updated;
    case sizeRole:
        return game_info.size;
    case GameInfoRole:
        return QVariant::fromValue(game_info);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> GameInfoModel::roleNames() const
{
    return role_names_;
}

void GameInfoModel::insert(int index, const GameInfo &game_info)
{
    if (index < 0 || index > games_info_.size()) {
        return;
    }

    emit beginInsertRows(QModelIndex(), index, index);
    games_info_.insert(index, game_info);
    emit endInsertRows();
}

void GameInfoModel::prepend(const GameInfo &game_info)
{
    insert(0, game_info);
}

void GameInfoModel::append(const GameInfo &game_info)
{
    insert(games_info_.size(), game_info);
}

void GameInfoModel::remove(int index)
{
    if (index < 0 || index >= games_info_.size()) {
        return;
    }

    emit beginRemoveRows(QModelIndex(), index, index);
    auto game = games_info_.at(index);
    games_info_.removeAt(index);
    emit removed(game);
    emit endRemoveRows();
}

void GameInfoModel::remove(const GameInfo &game_info)
{
    int index = games_info_.indexOf(game_info);
    if (index == -1) {
        return;
    }

    remove(index);
}

void GameInfoModel::clear()
{
    if (games_info_.isEmpty()) {
        return;
    }

    emit beginRemoveRows(QModelIndex(), 0, games_info_.size() - 1);
    for (auto &game : games_info_) {
        emit removed(game);
    }
    games_info_.clear();
    emit endRemoveRows();
}