/*
 * 原pyETRC中Line类
 * 注意：与区间相关时，
 * prev, next指运行方向；
 * left, right指列表方向 （列表先后顺序）；
 * up, down指行别。
 */
#pragma once

#include <QVariant>
#include <QList>
#include <QJsonObject>
#include <memory>
#include <utility>
#include <tuple>

#include "railstation.h"
#include "railinterval.h"
#include "railinfonote.h"



class Ruler;
class Forbid;
struct Config;

class Railway:
    public std::enable_shared_from_this<Railway>
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
    bool numberMapEnabled = false;

    double _diagramHeightCoeff = -1;

public:
    Railway(const QString& name="");
    // 2021.09.28  删除这个构造函数；因为shared_from_this，
    // 只能先默认构造，再读Json。
    //Railway(const QJsonObject& obj);

    /**
     * 2021.09.13实现 拷贝构造
     * 用于从RailDB导入线路的操作  手写深拷贝
     * !!! 2021.09.28：构造Ruler的时候调用了weak_from_this!!
     */
    Railway(const Railway& other) = delete;

    /**
     * 2021.07.04  警告！！ IntervalData持有Railway的反向引用
     * 默认的move行为应该是不正确的！！
     */
    Railway(Railway&& rail) = delete;

    /**
     * 2021.09.28：因为shared_from_this的原因，
     * 拷贝构造不安全，改为实现拷贝赋值。
     */
    Railway& operator=(const Railway& rail);
    Railway& operator=(Railway&& rail) = delete;

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
        int level = 4,
        std::optional<double> counter = std::nullopt,
        PassedDirection direction = PassedDirection::BothVia,bool show=true,
        bool passenger = false, bool freight = false, bool single=false
    );
    void insertStation(int index,
        const StationName& name, double mile,
        int level = 4,
        std::optional<double> counter = std::nullopt,
        PassedDirection direction = PassedDirection::BothVia, bool show = true,
        bool passenger = false, bool freight = false, bool single = false
    );

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

    QString validRulerName(const QString& prefix)const;

    /// <summary>
    /// 签名形式未确定
    /// </summary>
    bool isNewRuler()const;

    /* *
     * Line.delRuler()
     * 原标尺Ruler对象不受影响。但注意它只是个头结点！！
     */
    void removeRuler(std::shared_ptr<Ruler> ruler);

    /**
     * 删除并返回最后一个Ruler。
     * 注意返回的只是个头结点！ seealso: removeRuler
     */
    std::shared_ptr<Ruler> takeLastRuler();

    /**
     * 插入标尺，用于撤销操作
     * 下标由ruler自带；直接把这个当成头结点弄回去
     * 也就是redo之后头结点地址不变
     */
    void undoRemoveRuler(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data);

    /*
     * 一个个删除Ruler效率比较低，因此做一个一次性清空的
     */
    void clearRulers();

    void clearForbids();

    /// <summary>
    /// Line.changeStationNameUpdateMap
    /// 但所做的事情更多一些
    /// 更改站名并更新映射
    /// 新增约定：oldName必须是完整站名！！
    /// 2021.08.15：用不到了。现在用 swapStationName 以支持撤销
    /// </summary>
    //void changeStationName(const StationName& oldname, const StationName& newname);

    /**
     * 2021.08.15 执行站名更改的操作
     * 将站名与`name`交换，同时更新映射表，用以支持撤销重做
     */
    void swapStationName(std::shared_ptr<RailStation> station, StationName& name);

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
        return empty() ? StationName::nullName : _stations.first()->name;
    }

    inline StationName lastStationName()const {
        return empty() ? StationName::nullName : _stations.last()->name;
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
    /// 
    /// 注意拷贝行为暂不确定，先不实现这个
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
    std::shared_ptr<const RailInterval> firstDownInterval()const;
    std::shared_ptr<RailInterval> firstDownInterval();

    std::shared_ptr<const RailInterval> firstUpInterval()const;
    std::shared_ptr<RailInterval> firstUpInterval();

    /**
     * 2021.09.24  最后一个下行区间；从最后一个站倒回去找。
     */
    std::shared_ptr<RailInterval> lastDownInterval();

    std::shared_ptr<const RailStation> firstDownStation()const;

    std::shared_ptr<const RailStation> firstUpStation()const;

    std::shared_ptr<const RailStation> firstDirStation(Direction dir)const;

    inline const QString& name()const{
        return _name;
    }
    inline QString& nameRef() { return _name; }  //用来搞交换
    inline void setName(const QString& n) { _name = n; }

    auto& notes() { return _notes; }
    const auto& notes()const { return _notes; }

    void showStations()const;

    void showIntervals()const;

    /**
     * Line.addEmptyRuler()
     * 最标准的添加标尺方法
     */
    std::shared_ptr<Ruler> addEmptyRuler(const QString& name, bool different);

    /**
     * 注意 TOPO等价时才能用！！ 否则UB  不做检查
     * 从对方ruler中添加一个新的标尺，各个区间数据都完全一致。
     */
    std::shared_ptr<Ruler> addRulerFrom(std::shared_ptr<Ruler> r);

    std::shared_ptr<Ruler> addRulerFrom(const Ruler& r);

    /**
     * 使用head作为头结点（Ruler对象），从data中添加数据。
     * 用于undo/redo过程中，保证Ruler地址不变
     * precondition: head中的数据都是对的，不进行更改
     */
    std::shared_ptr<Ruler> restoreRulerFrom(std::shared_ptr<Ruler> head,
        const Ruler& data);

    std::shared_ptr<Ruler> restoreRulerFrom(std::shared_ptr<Ruler> head,
        std::shared_ptr<const Ruler> data);

    /**
     * 替代Ruler的构造函数
     * 目前兼容pyETRC 3.3的文件，构造效率比较低
     */
    std::shared_ptr<Ruler> addRuler(const QJsonObject& obj);

    /**
     * 寻找相邻区间
     * 注意 不考虑跨区间，因此是常数复杂度
     * 精确查找。
     */
    std::shared_ptr<RailInterval> findInterval(const StationName& from,
                                               const StationName& to);

    std::shared_ptr<const RailInterval> findInterval(const StationName& from,
        const StationName& to)const;


    /**
     * 支持域解析符条件下查找区间，不考虑跨区间
     */
    std::shared_ptr<RailInterval> findGeneralInterval(const StationName& from,
                                                      const StationName& to);

    /**
     * 原Ruler.getInfo()中allow_multi模式
     * 返回路径中的Interval顺序表
     * 允许域解析符匹配
     * 如果有一个不存在，返回空表
     */
    QList<std::shared_ptr<RailInterval>> multiIntervalPath(const StationName& from,
        const StationName& to);

    inline std::shared_ptr<const Ruler> getRuler(int i)const{return _rulers[i];}
    inline auto getRuler(int i){return _rulers[i];}
    inline std::shared_ptr<const Forbid> getForbid(int i)const{return _forbids[i];}
    inline auto getForbid(int i){return _forbids[i];}
    inline const auto& rulers()const { return _rulers; }
    inline const auto& forbids()const { return _forbids; }

    /**
     * 返回第一个天窗对象；
     * 如果不存在，创建然后返回
     */
    std::shared_ptr<Forbid> firstForbid();

    /**
     * @brief ensureForbids
     * 保证天窗数量不少于指定。
     */
    void ensureForbids(int count);

    inline auto& forbids() { return _forbids; }

    inline std::shared_ptr<Ruler> ordinate(){return _ordinate;}
    inline void setOrdinate(std::shared_ptr<Ruler> ord){_ordinate=ord;}
    inline void resetOrdinate(){_ordinate=nullptr;}
    inline void setOrdinate(const QString& rulerName){
        setOrdinate(rulerByName(rulerName));
    }

    /**
     * 按照下标设置排图标尺。-1表示无效。
     * 用在撤销线路信息更改
     */
    inline void setOrdinateIndex(int idx) {
        if (idx >= 0)
            setOrdinate(_rulers.at(idx));
        else
            resetOrdinate();
    }

    int ordinateIndex()const;

    /**
     * @brief 计算每个站的y坐标
     * 如果是标尺排图，返回标尺排图的高度；如果是里程排图，返回里程对应的高度。
     * 如果标尺不完备，reset。注意同时还要保证所有不铺画的站的yValue无效 （-1）
     * @param config  用于计算的配置表
     * @return false-指定标尺排图，但标尺不完备不能用 otherwise true （2021.07.15修改）
     * 2021.10.08：与Config解耦，这里不乘系数。
     */
    bool calStationYCoeff();

    /**
     * @brief diagramHeight
     * 单纯返回图高度。需保证已经调用过calStationYValue()。
     */
    double diagramHeightCoeff()const { return _diagramHeightCoeff; }

    /**
     * 2021.10.08  根据排图情况（里程或标尺），将y坐标coeff转换为y坐标
     */
    double yValueFromCoeff(double coeff, const Config& config)const;

    /**
     * seealso yValueFromCoeff 
     * 反向的转换。
     */
    double yCoeffFromAbsValue(double val, const Config& config)const;

    double diagramHeight(const Config& cfg)const;


    /**
     * 和RailInterval::nextInterval()差不多
     * 但额外支持一次性解决上行的
     */
    std::shared_ptr<RailInterval> nextIntervalCirc(std::shared_ptr<RailInterval> railint);

    std::shared_ptr<const RailInterval>
        nextIntervalCirc(std::shared_ptr<const RailInterval> railint)const;


    inline double totalMile()const { return empty() ? 0.0 : _stations.last()->mile; }

    /**
     * 本线与指定线路是否是Topo等价的。
     * Topo等价当且仅当站数一样，且每个站的单向站性质一样，
     * 此时对线路的任何修改可以理解为修改站名以及修改相应站和区间的数据，
     * 旧有的区间数据可以得到全盘保留。
     */
    bool topoEquivalent(const Railway& another)const;

    bool stationNameExisted(const QString& name)const;

    bool stationNameExisted(const StationName& name)const;

    /**
     * 合并对方的区间数据到本线，即标尺天窗。
     * 分为两种情况：如果topo等价，则逐个区间合并；
     * 否则按区间名检索。皆为线性复杂度。
     * 返回是否topo等价
     */
    bool mergeIntervalData(const Railway& other);

    /**
     * !!危险操作  想清楚再调用!!
     * 
     * 将本线与other的里程数据以及区间数据交换。
     * 即：交换_stations, _rulers, _forbids。
     * 同时交换Ruler/Forbid中的Railway引用！！
     * 此操作后，与列车的绑定数据失效
     * precondition: 标尺、天窗的数目一致。
     * 2022.02.07新增要求：标尺、天窗头结点数据分别一致。
     * other的标尺、天窗数据实际上来自于mergeIntervalData()。
     * postcondition: 有效的ruler, forbid（头结点）地址不变。
     * （实际上就是ruler根本不变）
     * 2022.04.03新增要求：必须以要用的数据为this指针调用本函数，即this和参数地位不等价。
     * this指针保证调用后所有Ruler/Forbid结点的头结点引用正确，
     * 但other不保证（只要有数据就一定不满足）。
     */
    void swapBaseWith(Railway& other);

    /**
     * 生成一个新的对象，仅包含基线数据，不包括标尺
     */
    std::shared_ptr<Railway> cloneBase()const;

    // 断面信息：y坐标，下行区间，上行区间
    using SectionInfo = std::optional<std::tuple<
        double,
        std::shared_ptr<RailInterval>,
        std::shared_ptr<RailInterval>>>;

    /**
     * 由所给里程标获取所在断面的情况：y坐标，下行区间，上行区间 
     * precondition: y坐标值已经正确计算出来
     * 如果所给里程正好是一个站的里程标，则算入站前区间（小里程端区间）
     * 如果不存在，则返回空。
     * 允许给0点里程的；但对应区间指针都是空指针
     * y坐标值一定有效，根据下行区间进行插值。如果下行区间找不到，直接返回空
     * （特例是给首站的情况，直接返回首站的y坐标）
     */
    SectionInfo getSectionInfo(double mile)const;

    /**
     * 从已有的Forbid中复制数据。
     * precondition: 两线路topo等价，否则引发UB
     */
    std::shared_ptr<Forbid> addForbidFrom(std::shared_ptr<Forbid> other);

    std::shared_ptr<Forbid> addForbidFrom(const Forbid& forbid);

    /**
     * 从下行第一个区间开始，数到第i区间，返回。
     * 用于IntervalDataModel行数对应的interval。
     * 线性算法。
     */
    std::shared_ptr<RailInterval> intervalCircByIndex(int i);

    /**
     * 删除小于指定节点数的标尺。不考虑撤销。
     */
    void filtRulerByCount(int minCount);

    /**
     * 使对称：将所有站简单地改成上下行通过，然后生成上行区间。
     * 用于生成对称的切片。
     */
    void symmetrize();

    /**
     * 2022.09.11 新增要求里程标单调不减小的要求。
     * 如果不满足这个要求，getSectionInfo()的二分搜索有可能失败。
     */
    bool isMileSorted()const;

    /**
     * 2022.09.11 检查区间负里程问题。返回第一个负里程区间。
     * 如果没有，返回空
     */
    std::shared_ptr<const RailInterval> firstNegativeInterval()const;

    /**
     * 返回第一个非法的全站名序号，用于启动检查。
     * 如果没有，返回-1。
     */
    int firstInvalidNameIndex()const;

private:
    /**
     * 维护nameMap和fieldMap
     */
    void addMapInfo(const std::shared_ptr<RailStation>& st);

    /**
     * 删除车站时维护数据
     */
    void removeMapInfo(const StationName& name);

    void setMapInfo();

    /**
     * 启用和禁用numberMap
     * 用于初始化时快速找下标
     */
    void enableNumberMap();
    void disableNumberMap();

    /**
     * 暴力线性算法查找
     */
    int stationIndexBrute(const StationName& name)const;

    /**
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

    /**
     * 添加一个区间，代替RailInterval的构造函数。
     * 注意shared_from_this不能再构造函数中调用。
     */
    std::shared_ptr<RailInterval> addInterval(Direction _dir,
            std::shared_ptr<RailStation> from,
            std::shared_ptr<RailStation> to);

    /**
     * 通过一次遍历，补充所有上行区间对象。
     * 用于mergeCounter之后。
     */
    void initUpIntervals();

    std::shared_ptr<Forbid> addEmptyForbid(bool different=true);

    /**
     * 如果不存在，就进来一个空的obj，这样也能构造空的Forbid出来
     */
    std::shared_ptr<Forbid> addForbid(const QJsonObject& obj);



    /**
     * @brief calStationYValueByMile  强制按里程计算每个站的坐标。
     * 2021.10.08：取消Config依赖
     */
    double calStationYCoeffByMile();

    /**
     * @brief clearYValues  清除所有y坐标数据
     * 在标尺排图之前调用，保证所有的yValue数据都是最新
     * 同时清除标签占位信息
     */
    void clearYValues();

    /**
     * 2022.04.03  非平凡操作！
     * swapBase()之后调用，保证所有Ruler/Forbid的数据结点
     * 中的引用正确指向对应的父节点。
     */
    void setupIntervalNodeDatas();

};

Q_DECLARE_METATYPE(std::shared_ptr<Railway>);


