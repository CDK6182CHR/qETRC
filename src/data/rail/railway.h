/*
 * 原pyETRC中Line类
 * 注意：与区间相关时，
 * prev, next指运行方向；
 * left, right指列表方向 （列表先后顺序）；
 * up, down指行别。
 */
#pragma once

#include <QtCore>
#include <memory>
#include <utility>

#include "railstation.h"
//#include "ruler.h"
#include "railinterval.h"


struct RailInfoNote{
    QString author,version,note;
    RailInfoNote()=default;
    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

class Ruler;
class Forbid;
struct Config;

class Railway
{
    QString _name;

    /**
     * 注意：暂定采用shared_ptr实现，
     * 这是为了方便使用HashMap。
     * 注意复制语义。
     * 注意不能随便给this指针，必要时修改类定义
     */
    QList<std::shared_ptr<RailStation>> _stations;

    /**
     * 注意这里相当于只是保存了头结点
     */
    QList<std::shared_ptr<Ruler>> _rulers;
    QList<std::shared_ptr<Forbid>> _forbids;

    /**
     * @brief _ordinate  排图标尺指针
     * 注意排图标尺现在改为Railway的性质，用以支持多条线路平铺。
     */
    std::shared_ptr<Ruler> _ordinate;

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

    double _diagramHeight = -1;
    double _startYValue = -1;   //起始y坐标

public:
    Railway(const QString& name="");
    Railway(const QJsonObject& obj);

    //copy ctor & copy = 需要时再写
    //注意可能需要深拷贝
    Railway(const Railway& rail) = delete;
    Railway(Railway&& rail) = default;

    Railway& operator=(const Railway& rail) = delete;
    Railway& operator=(Railway&& rail) = default;

    /// <summary>
    /// Line.copyData() 不复制ruler的模式
    /// 如果要复制Ruler可以直接operator=
    /// 使用右值引用，强调移动语义
    /// </summary>
    void moveStationInfo(Railway&& another);

    /// <summary>
    /// Line.clear()  清除所有数据
    /// 但C++下似乎可以用构造的方法轻松解决。
    /// 必要时实现
    /// </summary>
    void clear();

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

    const std::shared_ptr<const RailStation>
        stationByName(const StationName& name)const;

    /*
     * 2021.06.16新增
     * 仅允许以下两种情况：严格匹配，或者非Bare匹配到Bare类型
     * 注意总是优先返回严格匹配的
     * （实际上只可能有两种解）
     */
    std::shared_ptr<RailStation> stationByGeneralName(const StationName& name);

    const std::shared_ptr<const RailStation>
        stationByGeneralName(const StationName& name)const;

    inline std::shared_ptr<RailStation> stationByIndex(int i) {
        return _stations[i];
    }

    inline const std::shared_ptr<RailStation> stationByIndex(int i)const {
        return _stations[i];
    }

    /*
     * Line.stationInLine 的严格模式
     */
    bool containsStation(const StationName& name)const;

    /*
     * 注意条件与stationByGeneralName一致
     */
    bool containsGeneralStation(const StationName& name)const;

    /**
     * 与pyETRC不同：暂定不存在的站返回-1
     */
    int stationIndex(const StationName& name)const;

    /**
     * Line.delStation
     * 线性算法
     */
    void removeStation(const StationName& name);

    void removeStation(int index);

    /**
     * Line.adjustLichengTo0
     */
    void adjustMileToZero();

    /**
     * Line.isDownGap()
     * 但注意车站匹配条件变了
     * 如果有至少一个不存在，暂定直接返回true
     * 注意这里根据里程判定，实际上不是很严格
     * 如果有车站不存在，返回Undefined
     */
    Direction gapDirection(const StationName& s1,const StationName& s2)const;

    Direction gapDirection(const std::shared_ptr<const RailStation>& s1,
                   const std::shared_ptr<const RailStation>& s2)const;

    /** 
     * 按照index判定的严格版本，但在没有启用numberMap时代价很大
     */
    Direction gapDirectionByIndex(const StationName& s1, const StationName& s2)const;

    /**
     * 返回两站之间跨越的站数。
     * s1, s2都属于本线。调用gapDirection()判定行别，然后优先按照行别找
     * 注意这里是按照interval找
     * 极端情况：如果因为里程等原因导致算出来的行别不对，算法仍然正确结束，
     * 但会把另外一边遍历完 （线性复杂度）。
     */
    int stationsBetween(std::shared_ptr<const RailStation> s1,
        std::shared_ptr<const RailStation> s2)const;


    /*
     * Line.gapBetween()
     * 如果不存在，返回-1
     */
    double mileBetween(const StationName& s1,const StationName& s2)const;

    /*
     * Line.isSplited()
     * 上下行分设情况
     */
    bool isSplitted()const;

    /*
     * Line.resetRulers()
     * 设置标尺的上下行分设等信息
     * 好像没用？需要时实现
     */
    void updateRulers();

    inline int stationCount()const {
        return _stations.count();
    }

    inline const QList<std::shared_ptr<RailStation>>& 
        stations()const {
        return _stations;
    }

    std::shared_ptr<Ruler> rulerByName(const QString& name);

    bool rulerNameExisted(const QString& name, 
        std::shared_ptr<const Ruler> ignore=nullptr)const;

    /// <summary>
    /// 签名形式未确定
    /// </summary>
    bool isNewRuler()const;

    /* 
     * Line.delRuler()
     */
    void removeRuler(std::shared_ptr<Ruler> ruler);

    /*
     * 一个个删除Ruler效率比较低，因此做一个一次性清空的
     */
    void clearRulers();

    /// <summary>
    /// Line.changeStationNameUpdateMap
    /// 但所做的事情更多一些
    /// 更改站名并更新映射
    /// 新增约定：oldName必须是完整站名！！
    /// </summary>
    void changeStationName(const StationName& oldname, const StationName& newname);

    /// <summary>
    /// Line.lineLength()  线路长度
    /// 最后一个站的里程
    /// </summary>
    inline double railLength()const {
        return _stations.empty() ? 0 : _stations.last()->mile;
    }

    /// <summary>
    /// Line.counterLength()  对里程长度
    /// 最后一站的对里程。如果没有数据，则用正里程
    /// </summary>
    double counterLength()const;

    inline bool empty()const { return _stations.empty(); }

    inline StationName firstStationName()const {
        return empty() ? StationName() : _stations.first()->name;
    }

    inline StationName lastStationName()const {
        return empty() ? StationName() : _stations.last()->name;
    }

    /// <summary>
    /// Line.splitStationDirection()
    /// pyETRC中暂时没用到，因此暂时不实现
    /// </summary>
    void splitStationDirection(const StationName& oldName,
        const StationName& downName, const StationName& upName);

    /// <summary>
    /// Line.reverse()
    /// 反排线路数据
    /// 反排站表；交换里程数据；交换单向站性质；交换里程和对里程
    /// </summary>
    void reverse();

    /// <summary>
    /// Line.adjIntervals()  所有邻接区间的列表
    /// 使用值类型，自动移动构造
    /// </summary>
    QList<QPair<StationName, StationName>> adjIntervals(bool down)const;

    /// <summary>
    /// Line.mergeCounter()  合并反向线路信息
    /// 1. counter的起点是当前线路的终点，但counter的终点未见得是当前线路起点。
    /// 2. counter中的“下行”是本线的“上行”。
    /// 3. 两边的各个站本来都设置的是Line.DownVia，即按照单向站处理的。
    /// 4. 两边的站序是差不多相同的。
    /// </summary>
    /// <param name="another">当前线路的上行版本</param>
    void mergeCounter(const Railway& another);

    /// <summary>
    /// Line.slice()
    /// [start, end) 的切片
    /// 注意  这是浅拷贝
    /// </summary>
    Railway slice(int start, int end)const;

    /// <summary>
    /// Line.jointLine()
    /// 合并线路信息
    /// 注意不同的是：如果reverse，则先行调用reverse
    /// </summary>
    void jointWith(const Railway& another, bool former, bool reverse);

    /// <summary>
    /// 第一个下行区间
    /// first, last皆按运行方向
    /// </summary>
    /// <returns></returns>
    std::shared_ptr<RailInterval> firstDownInterval()const;

    std::shared_ptr<RailInterval> firstUpInterval()const;

    inline const QString& name()const{
        return _name;
    }

    void showStations()const;

    void showIntervals()const;

    /*
     * Line.addEmptyRuler()
     * 最标准的添加标尺方法
     */
    Ruler& addEmptyRuler(const QString& name,bool different);

    /*
     * 替代Ruler的构造函数
     * 目前兼容pyETRC 3.3的文件，构造效率比较低
     */
    Ruler& addRuler(const QJsonObject& obj);

    /*
     * 寻找相邻区间
     * 注意 不考虑跨区间，因此是常数复杂度
     * 精确查找。
     */
    std::shared_ptr<RailInterval> findInterval(const StationName& from,
                                               const StationName& to);

    /*
     * 支持域解析符条件下查找区间，不考虑跨区间
     */
    std::shared_ptr<RailInterval> findGeneralInterval(const StationName& from,
                                                      const StationName& to);

    /*
     * 原Ruler.getInfo()中allow_multi模式
     * 返回路径中的Interval顺序表
     * 允许域解析符匹配
     * 如果有一个不存在，返回空表
     */
    QList<std::shared_ptr<RailInterval>> multiIntervalPath(const StationName& from,
        const StationName& to);

    inline const Ruler& getRuler(int i)const{return *_rulers[i];}
    inline Ruler& getRuler(int i){return *_rulers[i];}
    inline const Forbid& getForbid(int i)const{return *_forbids[i];}
    inline Forbid& getForbid(int i){return *_forbids[i];}

    inline std::shared_ptr<Ruler> ordinate(){return _ordinate;}
    inline void setOrdinate(std::shared_ptr<Ruler> ord){_ordinate=ord;}
    inline void resetOrdinate(){_ordinate=nullptr;}
    inline void setOrdinate(const QString& rulerName){
        setOrdinate(rulerByName(rulerName));
    }

    /**
     * @brief 计算每个站的y坐标
     * 如果是标尺排图，返回标尺排图的高度；如果是里程排图，返回里程对应的高度。
     * 如果标尺不完备，reset。注意同时还要保证所有不铺画的站的yValue无效 （-1）
     * @param config  用于计算的配置表
     * @return 运行图总高度 （不包括上下坐标轴那些）
     */
    double calStationYValue(const Config& config);

    /**
     * @brief diagramHeight
     * 单纯返回图高度。需保证已经调用过calStationYValue()。
     */
    double diagramHeight()const { return _diagramHeight; }

    double startYValue()const { return _startYValue; }
    void setStartYValue(int y) { _startYValue = y; }

    /*
     * 和RailInterval::nextInterval()差不多
     * 但额外支持一次性解决上行的
     */
    std::shared_ptr<RailInterval> nextIntervalCirc(std::shared_ptr<RailInterval> railint);


private:
    /*
     * 维护nameMap和fieldMap
     */
    void addMapInfo(const std::shared_ptr<RailStation>& st);

    /*
     * 删除车站时维护数据
     */
    void removeMapInfo(const StationName& name);

    void setMapInfo();

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

    /// <summary>
    /// Line.addStation_by_origin
    /// 注意这里是复制语义，两边并非同一对象，因此采用const ref强调
    /// </summary>
    void insertStation(int i, const RailStation& station);

    void appendStation(const RailStation& station);

    void appendStation(RailStation&& station);

    /// <summary>
    /// 为缀加的车站添加区间信息
    /// 调用之前，车站已经添加好
    /// 这种比较需要心力开销的地方，就不考虑智能指针复制代价，当成裸指针用
    /// </summary>
    void appendInterval(std::shared_ptr<RailStation> st);

    /// <summary>
    /// 为缀加的车站添加区间信息
    /// 已知调用之前，车站已经添加好，并且下标为index
    /// 需要涉及打断区间操作
    /// </summary>
    void insertInterval(int index, std::shared_ptr<RailStation> st);

    /// <summary>
    /// 删除车站后，清除相应的区间信息
    /// 先调用本函数，再执行车站删除
    /// </summary>
    void removeInterval(int index);

    /// <summary>
    /// 寻找前一个通过指定方向的车站下标
    /// 如果不存在，返回-1
    /// </summary>
    /// <param name="cur">当前下标</param>
    /// <param name="dir">行别</param>
    std::shared_ptr<RailStation>
        leftDirStation(int cur, Direction _dir)const;

    std::shared_ptr<RailStation>
        rightDirStation(int cur, Direction _dir)const;

    /**
     * @brief leftBothStation 上一个双向通过的站。一定存在。
     * 用于标尺排图找上行。
     * 已知 所给车站是上行经过的！！
     */
    std::shared_ptr<RailStation>
        leftBothStation(std::shared_ptr<RailStation>);

    std::shared_ptr<RailStation>
        rightBothStation(std::shared_ptr<RailStation>);

    /*
     * 添加一个区间，代替RailInterval的构造函数。
     * 注意shared_from_this不能再构造函数中调用。
     */
    std::shared_ptr<RailInterval> addInterval(Direction _dir,
            std::shared_ptr<RailStation> from,
            std::shared_ptr<RailStation> to);

    Forbid& addEmptyForbid(bool different=true);

    /*
     * 如果不存在，就进来一个空的obj，这样也能构造空的Forbid出来
     */
    Forbid& addForbid(const QJsonObject& obj);

    /**
     * @brief calStationYValueByMile  强制按里程计算每个站的坐标。
     */
    double calStationYValueByMile(const Config& config);

    /**
     * @brief clearYValues  清除所有y坐标数据
     * 在标尺排图之前调用，保证所有的yValue数据都是最新
     * 同时清除标签占位信息
     */
    void clearYValues();

};


