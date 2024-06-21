#pragma once

#include <QString>

class User
{
public:
    User(int id, const QString &name, bool running)
        : name(name), id(id), running(running), installedApps(-1) {}

public:
    QString name;
    int id;
    bool running;
    int installedApps;
};