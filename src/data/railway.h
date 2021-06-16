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
    QJsonObject toJson()const;
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

    void appendStation(const StationName& name, double mile,
                    int level=4,
                    std::optional<double> counter=std::nullopt,
                    PassedDirection direction=PassedDirection::BothVia);
    void insertStation(int index,
                    const StationName& name, double mile,
                    int level=4,
                    std::optional<double> counter=std::nullopt,
                    PassedDirection direction=PassedDirection::BothVia);

    /*
     * Line.stationDictByName 的严格模式
     * 即不允许域解析符
     */
    std::shared_ptr<RailStation> stationByName(const StationName& name);

    const std::shared_ptr<RailStation>
        stationByName(const StationName& name)const;

    /*
     * 2021.06.16新增
     * 仅允许以下两种情况：严格匹配，或者非Bare匹配到Bare类型
     */
    std::shared_ptr<RailStation> stationByGeneralName(const StationName& name);

    const std::shared_ptr<RailStation>
        stationByGeneralName(const StationName& name)const;

    /*
     * Line.stationInLine 的严格模式
     */
    bool containsStation(const StationName& name)const;

    /*
     * 注意条件与stationByGeneralName一致
     */
    bool containsGeneralStation(const StationName& name)const;

    /*
     * 与pyETRC不同：暂定不存在的站返回-1
     */
    int stationIndex(const StationName& name)const;

    /*
     * Line.delStation
     * 线性算法
     */
    void removeStation(const StationName& name);

    /*
     * Line.adjustLichengTo0
     */
    void adjustMileToZero();

    /*
     * Line.isDownGap()
     * 但注意车站匹配条件变了
     * 如果有至少一个不存在，暂定直接返回true
     */
    bool isDownGap(const StationName& s1,const StationName& s2)const;

    bool isDownGap(const std::shared_ptr<RailStation>& s1,
                   const std::shared_ptr<RailStation>& s2)const;

    /*
     * Line.gapBetween()
     * 如果不存在，抛错
     */
    double mileBetween(const StationName& s1,const StationName& s2)const;

    //2021.06.16  line.py: 370

private:
    /*
     * 维护nameMap和fieldMap
     */
    void addMapInfo(const std::shared_ptr<RailStation>& st);

    /*
     * 删除车站时维护数据
     */
    void removeMapInfo(const StationName& name);

    /*
     * 启用和禁用numberMap
     * 用于初始化时快速找下标
     */
    void enableNumberMap();
    void disableNumberMap();

    //暴力线性算法
    int stationIndexBrute(const StationName& name)const;

    /*
     * Line.nameMapToLine()
     * 将站名映射到本线
     */
    StationName localName(const StationName& name)const;

};

#endif // RAILWAY_H
