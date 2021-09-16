#pragma once
#include <QString>
#include <QJsonObject>

struct RailInfoNote{
    QString author,version,note;
    RailInfoNote()=default;
    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
    bool operator==(const RailInfoNote& other)const;
    bool operator!=(const RailInfoNote& other)const;
};
