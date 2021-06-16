/*
 * 原pyETRC中Line类
 */
#ifndef RAILWAY_H
#define RAILWAY_H

#include <QtCore>
#include <memory>

#include "railstation.h"


struct RailInfoNote{
    QString author,version,note;
    RailInfoNote()=default;
    void fromJson(const QJsonObject& obj);
};

class Railway
{
    QString _name;

    /*
     * 注意：暂定采用shared_ptr实现，
     * 这是为了方便使用HashMap。
     * 注意复制语义。
     * 注意不能随便给this指针，必要时修改类定义
     */
    QList<std::shared_ptr<RailStation>> _stations;

    //还没有实现好的数据结构
    //self.rulers = []  # type:List[Ruler]
    //self.routes = []
    //self.forbid = ServiceForbid(self)
    //self.forbid2 = ConstructionForbid(self)
    //self.item = None  # lineDB中使用。
    //self.parent = None  # lineDB中使用

    RailInfoNote _notes;

    QHash<StationName, std::shared_ptr<RailStation>> nameMap;
    QHash<QString, QList<StationName>> fieldMap;
    QHash<StationName, int> numberMap;
    bool numberMapEnabled;

public:
    Railway(const QString& name="");
    Railway(const QJsonObject& obj);

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

#endif // RAILWAY_H
