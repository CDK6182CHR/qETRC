#pragma once

#include <cstdint>
#include <QVector>
#include <QPen>
#include <list>
#include <optional>
#include "trainname.h"
#include "trainstation.h"
#include "data/common/qeglobal.h"

class RailStation;
class Railway;
class QJsonObject;
struct Config;
class RailCategory;

enum class TrainPassenger:
        std::int8_t{
    False=0,
    Auto=1,
    True=2
};

class TrainAdapter;

class TrainType;
class TypeManager;
class Routing;
class RoutingNode;
struct AdapterStation;


/**
 * pyETRC.train.Train
 * 2021.06.22逻辑修订：要求这里仅包含于线路无关的操作
 * 也就是说这里的一切操作都要是在没有Railway情况下合法的
 * 与Railway相关的操作，在新建类TrainAdapter中完成
 * 
 * 2021.07.15  删除是否显示运行线/isShow这个内部属性
 * 定义：isShow()返回true当且仅当存在至少一条运行线处于显示状态。
 */
class Train:
    public std::enable_shared_from_this<Train>
{
    TrainName _trainName;
    StationName _starting,_terminal;

    /**
     * @brief _type  类型指针
     * 整个运行过程中，应当保证非空
     */
    std::shared_ptr<TrainType> _type;
    std::optional<QPen> _pen;
    TrainPassenger _passenger;

    bool _show;

    /**
     * 暂定采用std::list来放时刻表。
     * std::list::iterator当作指针来使用；
     * 主要原因是考虑双向迭代的效率
     * 时刻表拥有内部所有结点的所有权
     * 利用std::list的迭代器以及引用不会失效的特性
     * 涉及到删除操作时，要特别小心
     */
    std::list<TrainStation> _timetable;

    /**
     * Train.autoItems  是否采用自动运行线管理
     */
    bool _autoLines;

    /**
     * @brief _adapters 绑定到线路的指针
     * Train对象持有TrainAdapter的所有权。暂定这些信息不写入JSON
     * 注意：仅考虑数量很少的情况，因此一切查找皆为线性
     */
    QVector<std::shared_ptr<TrainAdapter>> _adapters;

    std::weak_ptr<Routing> _routing;
    std::optional<std::list<RoutingNode>::iterator> _routingNode;

    //以下数据：状态不变时有效，一旦状态改变则失效
    std::optional<double> _locMile;
    std::optional<int> _locRunSecs, _locStaySecs;

public:
    using StationPtr=std::list<TrainStation>::iterator;
    using ConstStationPtr=std::list<TrainStation>::const_iterator;

    Train(const TrainName& trainName,
          const StationName& starting=StationName::nullName,
          const StationName& terminal=StationName::nullName,
          TrainPassenger passenger=TrainPassenger::Auto);
    explicit Train(const QJsonObject& obj, TypeManager& manager);

    /**
     * std::list 默认的copy和move都是正确的
     * 但Adapter的复制行为是不对的，因此必须重写
     * 暂定都不复制（移动）Adapter部分
     */
    Train(const Train& another);
    Train(Train&& another) noexcept;

    //需要时再实现
    Train& operator=(const Train&) = delete;
    Train& operator=(Train&&)noexcept = delete;

    void fromJson(const QJsonObject& obj, TypeManager& manager);
    QJsonObject toJson()const;

    inline const TrainName& trainName()const{return _trainName;}
    inline TrainName& trainName(){ return _trainName; }
    inline const StationName& starting()const{return _starting;}
    inline const StationName& terminal()const{return _terminal;}
    inline StationName& startingRef() { return _starting; }
    inline StationName& terminalRef() { return _terminal; }
    inline auto type()const{return _type;}
    inline TrainPassenger passenger()const{return _passenger;}
    inline bool isShow()const { return _show; }

    inline void setTrainName(const TrainName& n){_trainName=n;}
    inline void setStarting(const StationName& s){_starting=s;}
    inline void setTerminal(const StationName& s){_terminal=s;}
    inline void setType(std::shared_ptr<TrainType> t){_type=t;}
    inline void setPassenger(TrainPassenger t){_passenger=t;}
    inline void setIsShow(bool  s) { _show = s; }

    /**
     * 强制判断是否为客车；如果设置为自动，根据类型
     */
    bool getIsPassenger()const;
    
    /**
     * 2021.07.15  对所有运行线设置
     */
    void setLineShow(bool s);

    /**
     * 根据类型名，设定类型。
     * 设计用于读入数据时。
     */
    void setType(const QString& _typeName, TypeManager& manager);

    /**
     * @brief pen
     * 返回非空指针。如果_pen为空（采用默认设置），返回类型默认值
     */
    const QPen& pen()const;

    /**
     * 是否采用自动UI （类型设置的UI）
     */
    bool autoPen()const;

    /**
     * 清空Pen信息，即使用自动的
     */
    void resetPen();

    void setPen(const QPen& pen);

    inline ConstStationPtr nullStation()const { return _timetable.cend(); }
    inline StationPtr nullStation(){return _timetable.end();}
    inline bool isNullStation(ConstStationPtr st)const { return st == _timetable.end(); }

    const std::list<TrainStation>& timetable()const{return _timetable;}
    std::list<TrainStation>& timetable(){return _timetable;}

    auto& adapters() { return _adapters; }
    const auto& adapters()const { return _adapters; }

    void appendStation(const StationName& name,
                       const QTime& arrive,
                       const QTime& depart,
                       bool business=true,
                       const QString& track="",
                       const QString& note="");

    void prependStation(const StationName& name,
        const QTime& arrive,
        const QTime& depart,
        bool business = true,
        const QString& track = "",
        const QString& note = "");

    /**
     * Train.stationDict()
     * 查找算法  精确匹配
     */
    StationPtr findFirstStation(const StationName& name);

    QList<StationPtr>
        findAllStations(const StationName& name);

    ConstStationPtr findFirstStation(const StationName& name)const;

    QList<ConstStationPtr>
        findAllStations(const StationName& name)const;

    /**
     * 非严格查找算法
     * 注意与Railway的匹配机制不同：
     * 使用 equalOrBelongsTo(name)
     * 总是优先匹配严格的
     */
    StationPtr findFirstGeneralStation(const StationName& name);

    QList<StationPtr>
        findAllGeneralStations(const StationName& name);


    /**
     * qETRC新增核心函数
     * 将车次绑定到线路
     * 注意：如果已经绑定到同一条线路，不做任何事
     * （在这里调用TrainAdapter构造函数）
     */
    std::shared_ptr<TrainAdapter> bindToRailway(std::shared_ptr<Railway> railway, const Config& config);

    /**
     * 一次性清除所有绑定数据
     */
    void clearBoundRailways();

    /**
     * 适用于线路可能发生变化时，
     * 即使已经绑定到同一条线路，也会撤销再重来 （转移构造）
     */
    std::shared_ptr<TrainAdapter> updateBoundRailway(std::shared_ptr<Railway> railway, const Config& config);

    /**
     * @brief unbindToRailway
     * 撤销与所选线路的绑定，删除绑定对象。基于地址判定。
     */
    void unbindToRailway(std::shared_ptr<const Railway> railway);

    bool isBoundToRailway(std::shared_ptr<const Railway> railway);

    /**
     * @brief autoLines 自动设置运行线数据
     * 承担了pyETRC中TrainItem::setItem()中划分Item的工作
     * @param railway 绑定到的线路
     * @param config 配置信息
     */
    //void autoLines(std::shared_ptr<Railway> railway, const Config& config);

    /**
     * 时刻表中前一个（不包含当前）成功绑定到线路的车站。
     * 通过本车次指针向前索引的算法。
     */
    StationPtr prevBoundStation(StationPtr st);

    StationPtr nextBoundStation(StationPtr st);

    ConstStationPtr prevBoundStation(ConstStationPtr st)const;

    ConstStationPtr nextBoundStation(ConstStationPtr st)const;

    /**
     * Train.stationDown()
     * 判定局部上下行性质
     * 注意调用了线性算法来查找车站 （非精确查找）
     *
     * 注意: 参考pyETRC的实现，先向左查再向右查，
     * 即行别优先定义为[到达行别]
     */
    Direction stationDirection(const StationName& station);

    /**
     * 注意：强制要求只有当前车站已经绑定到线路时，
     * 才计算结果。
     * 通过先查找上一站、再查找下一站的算法，由所绑定的两个车站的上下行关系判定
     * 准常数复杂度
     */
    Direction stationDirection(ConstStationPtr station);

    /**
     * Train.translation()  使用给定车次平移
     */
    Train translation(TrainName name, int sec);

    /**
     * pyETRC.Train.setStationDeltaTime()
     * 调整指定范围车站的时刻表，前移或后移指定的秒数。
     */
    void adjustTimetable(int startIndex, int endIndex,
        bool includeFirst, bool includeLast, int secs);

    /**
     * Train.delNonLocal()
     * 删除非本线车站 
     */
    //void removeNonLocal();

    /**
     * 注意：右值引用，移动语义
     * 调用前，非本线车站已经删除，都已经绑定到同一条线路
     * former: 对方在本线之前
     * 删除原有线性算法：只检查交叉处的几个站是否重合 （站名一致）
     * 最多检查至10个站 （检查的过程没有做优化，平方复杂度，但深度较小，因此关系不大）
     * 
     * 已通过单元测试
     */
    void jointTrain(Train&& train, bool former);

    void show()const;

    inline bool isStartingStation(const StationName& s)const {
        return starting().generalEqual(s);
    }
    inline bool isTerminalStation(const StationName& s)const {
        return terminal().generalEqual(s);
    }

    /**
     * 用于判断时刻表中所给车站是否是始发（终到）站。
     * 不仅要比较站名，还要考虑是否是某一个Adapter的首站
     */
    bool isStartingStation(const AdapterStation* st)const;

    bool isTerminalStation(const AdapterStation* st)const;

    /**
     * Train.intervalExchange() 区间换线
     * 注意start,end都包含在内
     */
    void intervalExchange(Train& train2, StationPtr start1, StationPtr end1,
        StationPtr start2, StationPtr end2, bool includeStart, bool includeEnd);

    /**
     * @brief Train.detectPassStation()  推定通过站时刻
     */
    void detectPassStation();

    inline bool hasRouting()const { return !_routing.expired(); }

    inline std::weak_ptr<Routing> routing() { return _routing; }
    inline std::weak_ptr<const Routing> routing()const { return _routing; }
    inline auto routingNode()const { return _routingNode; }
    inline auto routingNode() { return _routingNode; }

    /**
     * 设置交路，同时设定Node指针（迭代器）
     * 由Routing调用  注意不负责更新RoutingNode的数据
     */
    void setRouting(std::weak_ptr<Routing> rout, std::list<RoutingNode>::iterator node);

    /**
     * 将交路的数据更新到指定的对象上面。
     * @seealso: setRouting
     * 区别在于，不会将原有的设为virtual
     */
    void setRoutingSimple(std::weak_ptr<Routing> rout, std::list<RoutingNode>::iterator node);

    /**
     * 清除交路数据
     * 2021.07.04实现，用于导入车次时清除所引入图中的交路信息
     * 同时将RoutingNode那边的数据清理掉 （设置为虚拟）
     * 不考虑撤销
     */
    void resetRouting();

    /**
     * 2021.08.26清除交路数据，用于交路更新那边的操作，只是去掉这边的引用。
     * seealso: resetRouting
     * 区别在，不会将原来的设成virtual
     */
    void resetRoutingSimple();

    /**
     * 绑定到线路的最后一站。
     * 如果时刻表最后一站没有绑定到线路，返回空
     */
    const AdapterStation* boundLast()const;

    /**
     * @brief 如果终到站是已经绑定到线路的车站，返回它；
     * 否则返回空
     * 注意需要遍历所有的Adapter （Adapter并不一定是按顺序的）
     * 2021.08.30：改为必须是终到站；不要求是终到站的版本，改为boundLast
     */
    const AdapterStation* boundTerminal()const;

    std::shared_ptr<RailStation> boundTerminalRail()const;

    const AdapterStation* boundFirst()const;

    const AdapterStation* boundStarting()const;

    /**
     * 2021.10.09  性能  
     * seealso: boundStarting
     * 区别是：只考虑在rail所示线路的。解决起点站也可能绑定到多条线路的情况。
     * 考虑：高南线 高兴-南充 与达成线 同时绘制时，终到南充的车。
     */
    const AdapterStation* boundStartingAt(const Railway& rail)const;

    std::shared_ptr<RailStation> boundStartingRail()const;
    std::shared_ptr<RailStation> boundStartingAtRail(const Railway& rail)const;

    inline auto firstStation()const { return _timetable.begin(); }
    inline auto lastStation()const {
        if (_timetable.empty()) return _timetable.end();
        auto it = _timetable.end(); --it;
        return it;
    }

    double localMile();

    int localSecs();

    int localRunSecs();

    std::pair<int, int> localRunStaySecs();

    /**
     * 本线旅速，快速计算（仅用TrainLine收尾数据）  km/h
     * 用于TrainListTable
     */
    double localTraverseSpeed();

    /**
     * 本线技术速度 （忽略停车时长）
     */
    double localTechSpeed();

    /**
     * 数据改变 （例如线路数据变化）
     * 使得保存的临时计算数据失效
     * 一般来说，由绑定操作引起
     */
    void invalidateTempData();

    inline QString startEndString()const {
        return _starting.toSingleLiteral() + "->" + _terminal.toSingleLiteral();
    }

    /**
     * 比较两车次时刻表是否一致。
     * 但不比较其他的东西
     */
    bool timetableSame(const Train& other)const;

    /**
     * 与指定对象交换时刻表，用于undo/redo时刻表更新
     */
    void swapTimetable(Train& other);

#if 0
    /**
     * 连着Adapter一并交换
     * 2022.05.28注意：要同时交换Adapter里面对Train的反引用指针！
     * 2022.05.28批注2：禁用这个操作。带着TrainAdapter交换简直是在玩火！
     * 现在的版本不允许任何带Adapter的交换操作；都要求交换Timetable后重新绑定数据。
     */
    void swapTimetableWithAdapters(Train& other);
#endif

    /**
     * 交换车次、始发终到等信息，但不交换时刻表
     */
    void swapBaseInfo(Train& other);

    /**
     * 交换所有信息，包括Adapter
     */
    void swap(Train& other);

    inline bool empty()const { return _timetable.empty(); }

    /**
     * 线性遍历
     */
    std::shared_ptr<const TrainAdapter> adapterFor(const Railway& railway)const;
    std::shared_ptr<TrainAdapter> adapterFor(const Railway& railway);

    /**
     * 注意只由TrainLine调用
     * Train与TrainLine显示条件同步的临时解决方案
     * Train的显示与否定义为：是否存在至少一个TrainLine为显示的
     * 这个效率大概比较低
     */
    void checkLinesShow();

    int stationCount()const{return static_cast<int>(_timetable.size());}

    int adapterStationCount()const;

    int lineCount()const;

    /**
     * 设始发为第0日，返回终到是第几日。
     * @param byTimetable 若启用，则扫描整个时刻表，每次发现时刻“倒退”就增加一日；
     * 否则只比较首末站
     */
    int deltaDays(bool byTimetable)const;

    /**
     * @brief pyETRC.Train.totalMinTime
     * 终到站时间减去始发站时间
     */
    int totalMinSecs()const;

    /**
     * 列车类型指针的引用，用于更改列车类型的undo/redo操作
     * 注意是指针的引用；交换执行浅拷贝。
     */
    std::shared_ptr<TrainType>& typeRef() { return _type; }

    /**
     * 更新所有车站的Flag。
     */
    void refreshStationFlags();

    /**
     * 清除所有标记为 “推定”的站  
     * 返回是否有实际变更
     */
    bool removeDetected();

    /**
     * 转发给所有的TrainLine。
     * 返回是否变更。
     */
    bool autoBusiness();

    /**
     * 2022.01.23
     * 本车次时刻表与cat所示线路表的车站是否有交集。
     * 这里不考虑跨越站数，直接比对时刻表站名。
     * 有一个交集就返回true。
     */
    bool isLocalTrain(const RailCategory& cat)const;

    void clear();


    //static比较函数 用来排序
    static bool ltName(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool ltStarting(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool ltTerminal(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool ltShow(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool ltType(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool ltMile(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);
    static bool ltTravSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);
    static bool ltTechSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);

    static bool gtName(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool gtStarting(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool gtTerminal(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool gtShow(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool gtType(const std::shared_ptr<const Train>& t1, const std::shared_ptr<const Train>& t2);
    static bool gtMile(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);
    static bool gtTravSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);
    static bool gtTechSpeed(const std::shared_ptr<Train>& t1, const std::shared_ptr<Train>& t2);

    /**
     * 2022.05.14
     * 从单个trf文件读取单个列车。读取成功列车，失败返回空。
     * 暂时不管交路。
     * 注意  没有设置类型！！
     */
    static std::shared_ptr<Train> fromTrf(const QString& filename);

private:

    /**
     * 是否存在至少一条运行线是显示的
     */
    bool anyLineShown()const;

    bool indexValid(int i)const;

};



Q_DECLARE_METATYPE(std::list<TrainStation>::iterator)

