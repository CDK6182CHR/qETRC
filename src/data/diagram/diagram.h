#pragma once

#include <memory>
#include <QList>
#include <QString>
#include "config.h"
#include "data/train/traincollection.h"
#include "traingap.h"
#include "data/diagram/trainline.h"    // for: alias
#include "data/rail/railcategory.h"


class Train;
class Railway;
struct TrainGap;
class TrainFilterCore;


class DiagramPage;
namespace readruler {
    /**
     * 每个区间的每一种（四种附加情况）数据的描述：
     * 数据数值 -> 数据量
     */
    using IntervalTypeCount =
        std::map<int, int>;

    struct IntervalTypeReport {
        IntervalTypeCount count;  
        int tot;        // 最终的总数据量
        bool used;      // 是否使用本类数据
        double value;   // 本类计算结果
    };

    /**
     * 标尺综合的返回值类型中属于每个区间的数据部分
     * 与pyETRC基本一致
     *
     */
    struct IntervalReport {
        int interval = 0, start = 0, stop = 0;
        std::map<std::shared_ptr<Train>, std::pair<int, TrainLine::IntervalAttachType>> raw;
        std::map<TrainLine::IntervalAttachType, IntervalTypeReport> types;
        bool isValid()const { return interval || start || stop; }

        /**
         * 计算严格满足标尺的车次总数。
         */
        int satisfiedCount()const;

        int stdInterval(TrainLine::IntervalAttachType type)const;
    };
}

using ReadRulerReport = std::map<std::shared_ptr<RailInterval>, readruler::IntervalReport>;



/**
 * @brief The Diagram class  运行图基础数据的最高层次抽象
 * pyETRC.Graph的扩展
 * 支持多Railway。
 * 暂定仅支持一套TrainCollection
 * 注意全程Train应当是和Railway绑定的状态
 */
class Diagram
{
    RailCategory _railcat;
    TrainCollection _trainCollection;
    Config _config, _defaultConfig;
    QString _filename;
    QString _version, _note;
    TypeManager _defaultManager;
    QList<std::shared_ptr<DiagramPage>> _pages;

public:
    Diagram() = default;

    /**
     * 目前移动构造和赋值的默认行为都是正确的
     * 有问题... copy assign导致_page也被挪过来了
     * 但是Page中有Diagram的引用，这就导致出问题!
     * 暂定重新设置。
     */
    Diagram(const Diagram&)=delete;
    Diagram(Diagram&&)=default;
    Diagram& operator=(const Diagram&)=delete;
    Diagram& operator=(Diagram&&)=default;
    ~Diagram()noexcept = default;

    auto& railCategory() { return _railcat; }
    const auto& railCategory()const { return _railcat; }
    auto& defaultTypeManager(){ return _defaultManager; }

    /**
     * 读取默认配置文件 config.json
     * 包含Config和类型系统两部分
     * 注意：似乎应当移动到mainWindow的启动操作里面去 
     * （导入运行图等操作也要创建临时的Diagram对象）
     */
    bool readDefaultConfigs(const QString& filename= "config.json");

    bool saveDefaultConfigs(const QString& filename = "config.json")const;

    /**
     * @brief fromJson  清空既有数据，从文件读取，同时保存文件名
     */
    bool fromJson(const QString& filename);

    

    /**
     * @brief fromJson  读入数据后绑定车次和线路
     * 返回是否成功 （如果为空则失败）
     */
    bool fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * 导出单条线路的JSON。
     * localOnly这个筛选，仅作用于车次（不管Routing）
     */
    QJsonObject toSingleJson(std::shared_ptr<Railway> rail, bool localOnly)const;

    /**
     * 导出到Trc文件。因为trc仅支持一条线路，需要先选择哪一条线路导出。
     * localOnly: 是否仅导出在指定线路有Adapter的列车
     */
    bool toTrc(const QString& filename,std::shared_ptr<Railway> rail, bool localOnly)const;

    /**
     * 导出单条线路的pyETRC文件
     */
    bool toSinglePyetrc(const QString& filename, 
        std::shared_ptr<Railway> rail, bool localOnly)const;

    /**
     * 另存为 （同时记录文件名）
     */
    bool saveAs(const QString& filename);

    /**
     * 保存 （使用当前文件名）
     */
    bool save()const;

    /**
     * 打开新运行图或者新建等操作调用
     * 清理所有数据
     */
    void clear();

    inline auto& railways(){return _railcat.railways();}
    inline const auto& railways()const{return _railcat.railways();}
    inline auto& pages() { return _pages; }
    inline const auto& pages()const { return _pages; }
    inline auto firstRailway() { return _railcat.railways().first(); }
    inline auto& trainCollection(){return _trainCollection;}
    inline const auto& trainCollection()const{return _trainCollection;}
    inline const QString& filename()const{return _filename;}
    inline void setFilename(const QString& n){_filename=n;}
    inline auto& config(){return _config;}
    inline auto& defaultConfig() { return _defaultConfig; }
    inline const auto& config()const{return _config;}
    inline const QString& version()const { return _version; }
    inline void setVersion(const QString& v) { _version = v; }
    inline const QString& note()const { return _note; }
    inline void setNote(const QString& n) { _note = n; }

    inline std::shared_ptr<Railway> railwayAt(int i) { return _railcat.railways().at(i); }

    /**
     * @brief addRailway 添加线路
     * 同时将所有车次绑定到新线路
     */
    void addRailway(std::shared_ptr<Railway> rail);

    /**
     * 添加Collection中的所有列车到本运行图的列车集合中
     * 同时完成与线路的绑定
     * 注意，复制语义
     */
    void addTrains(const TrainCollection& coll);

    std::shared_ptr<Railway> railwayByName(const QString& name);
    inline std::shared_ptr<const Railway>
        railwayByName(const QString& name)const{
        return const_cast<Diagram*>(this)->railwayByName(name);
    }
    int railwayCount()const { return _railcat.railways().size(); }

    /**
     * @brief updateRailway  铺画准备
     * 更新指定线路，与所有车次重新绑定
     */
    void updateRailway(std::shared_ptr<Railway> r);

    /**
     * @brief updateTrain  铺画准备
     * @param t  更新的车次，与所有线路重新绑定  并不要求属于本运行图的TrainCollection
     */
    void updateTrain(std::shared_ptr<Train> t);

    auto& trains() { return _trainCollection.trains(); }

    /**
     * @brief pyETRC.GraphicsWidget.listTrainEvent  列出事件表
     * 采用类似ETRC的算法，逐个Adapter、逐个Item计算，而不是直接找图元交叠。
     * 暂时写成const，如果写不下去再改
     * preconditions:
     * 1. 所有里程、标尺严格递增。运行图已经铺画（车站y坐标已知）。
     * 2. 所有运行线是严格单调的，也就是说上行运行线y值单调不增。i.e. 运行线分划是严格正确的
     * 3. 折返车次优化：对重叠站，后一段运行线的事件表从出发开始列。即，如果没有开始标签，
     *    则第一站只列出出发时刻数据。
     * 4. 任何区间和停站时长都小于12小时。否则会干扰时刻前后判断。
     *    时刻前后的判断不依赖于前后文，只考虑当前：PBC下使得差值绝对值较小的理解。
     */
    TrainEventList listTrainEvents(const Train& train)const;

    DiagnosisList diagnoseTrain(const Train& train, bool withIntMeet)const;

    DiagnosisList diagnoseAllTrains(bool withIntMeet)const;


    /**
     * 创建默认的运行图视图，即按顺序包含本线的所有线路
     */
    std::shared_ptr<DiagramPage> createDefaultPage();

    bool pageNameExisted(const QString& name)const;
    bool pageNameExisted(const QString& name, std::shared_ptr<DiagramPage> ignore)const;

    bool pageNameIsValid(const QString& name, std::shared_ptr<DiagramPage> page);

    bool railwayNameExisted(const QString& name)const;

    QString validRailwayName(const QString& prefix)const;

    QString validPageName(const QString& prefix)const;

    /**
     * 如果读取失败，则运行图是个Null。
     * 没有任何线路的运行图是Null。
     * 这里还是要想清楚
     */
    inline bool isNull()const { return _railcat.railways().empty(); }

    /**
     * 对指定TrainCollection （不一定是本图的Collection）。
     * 使用本线的所有线路与之绑定。
     * 用于导入列车时
     */
    void applyBindOn(TrainCollection& coll);

    /**
     * 所给线名是否非空且不与别人重复，但忽略rail所示对象
     */
    bool isValidRailName(const QString& name, std::shared_ptr<Railway> rail);

    int getPageIndex(std::shared_ptr<DiagramPage> page)const;

    /**
     * 删除指定下标的线路，同时清理列车、运行图中的数据。
     * 不考虑撤销
     */
    [[deprecated]]
    void removeRailwayAt(int i);

    /**
     * 更新参数（最大跨越站数）时执行
     * 重新绑定所有列车与所有线路
     */
    void rebindAllTrains();

    /**
     * 2021.09.16  删除末尾的最后一条线路。保证没有关联的page之类的，
     * 但需要清除列车的绑定情况。
     */
    void undoImportRailway();

    using sec_cnt_t = std::map<std::shared_ptr<RailInterval>, int>;
    
    std::map<std::shared_ptr<RailInterval>, int>
        sectionTrainCount(std::shared_ptr<Railway> railway)const;

    /**
     * 区分客货的断面对数表
     */
    std::pair<sec_cnt_t, sec_cnt_t>
        sectionPassenFreighCount(std::shared_ptr<Railway> railway)const;

    /**
     * 2021.07.25  pyETRC风格的车站时刻表
     * 按照列车组织，给出每一列车的到点、开点等信息。注意只包含图定，不包含推断时刻
     */
    std::vector<std::pair<std::shared_ptr<TrainLine>, const AdapterStation*>>
        stationTrainsSettled(std::shared_ptr<Railway> railway, 
            std::shared_ptr<RailStation> st)const;

    /**
     * 2021.08.01  基于事件的车站时刻表
     */
    RailStationEventList
        stationEvents(std::shared_ptr<Railway> railway,
            std::shared_ptr<const RailStation> st)const;

    /**
     * 一次性获取所给线路的所有站事件表。
     * 初版暂时是暴力调用`stationEvents`，但很明显这个可以优化。
     */
    std::map<std::shared_ptr<RailStation>, RailStationEventList>
        stationEventsForRail(std::shared_ptr<Railway> railway)const;

    /**
     * 2021.09.06
     * 基于车站事件表，转换成间隔表。
     * 暂时只处理同向事件。
     * 保证所给输入都是同一个站的数据。
     */
    TrainGapList getTrainGaps(const RailStationEventList& events,
        const TrainFilterCore& filter, bool useSingle)const;

    /**
     * 双线模式
     */
    TrainGapList getTrainGapsDouble(const RailStationEventList& events,
        const TrainFilterCore& filter)const;

    TrainGapList getTrainGapsSingle(const RailStationEventList& events,
        const TrainFilterCore& filter)const;

    /**
     * 2021.09.08
     * 统计各种不同类型的列车间隔。
     * Positions只取Pre, Post或者空白三种情况 （空白仅针对Avoid类型的）；
     * 通通的，同时算入两边。set按照TrainGap的间隔自动排序。
     */
    TrainGapStatistics countTrainGaps(const TrainGapList& gaps, int curSecs)const;

    using SectionEventList = std::vector<std::pair<std::shared_ptr<TrainLine>, QTime>>;

    /**
     * 2021.08.05  区间某个位置的事件表  
     * 注意入参为纵坐标，不是里程标！
     */
    SectionEventList
        sectionEvents(std::shared_ptr<Railway> railway, double y)const;

    SnapEventList getSnapEvents(std::shared_ptr<Railway> railway, const QTime& time)const;

    /**
     * @brief rulerFromMultiTrains  pyETRC.graph.rulerFromMultiTrains() 标尺综合
     * 参数、返回值、算法、函数名等基本照搬pyETRC。
     * @param railway  qETRC新增  要读取的线路
     * @param intervals  要计算的区间
     * @param trains  用来计算的列车
     * @param useAverage   是否采用均值
     * @param defaultStart   默认起步附加
     * @param defaultStop    默认停车附加
     * @param cutStd  截断标准差倍数
     * @param cutSec  截断秒数
     * @param prec  精度/数据粒度
     * @param cutCount  最少类数据量
     * @return  计算结果和相关的报告数据 详见类定义
     */
    ReadRulerReport rulerFromMultiTrains(
            std::shared_ptr<Railway> railway,
            const QVector<std::shared_ptr<RailInterval>> intervals,
            const QList<std::shared_ptr<Train>> trains,
            bool useAverage, int defaultStart, int defaultStop,
            int cutStd=1, int cutSec=10, int prec=1,
            int cutCount=1
            );

    /**
     * 返回包含所给线路的Page的下标集合。
     */
    QVector<int> pageIndexWithRail(std::shared_ptr<const Railway> railway)const;

private:
    void bindAllTrains();

    void sectionTrainCount(std::map<std::shared_ptr<RailInterval>, int>& res,
        std::shared_ptr<TrainLine> line)const;

    bool fromTrc(QTextStream& fin);

    /**
     * pyETRC.data.Graph.__intervalFt()  区间数据统计
     * 对四种起停附加情况，统计频数
     */
    void __intervalFt(readruler::IntervalReport& itrep);

    /**
     * 众数模式下的计算
     */
    void __intervalRulerMode(readruler::IntervalReport& itrep, int defaultStart,
        int defaultStop, int prec, int cutCount);

    /**
     * 均值模式下的计算
     * cutSec, cutStd皆用0表示不启用。
     */
    void __intervalRulerMean(readruler::IntervalReport& itrep,
        int defaultStart, int defaultStop, int prec, int cutStd, int cutSec, int cutCount);

    /**
     * 解方程计算区间标尺。返回tuple，以避免修改。
     */
    std::tuple<int, int, int>
        __computeIntervalRuler(const std::map<TrainLine::IntervalAttachType,double>& values,
            int defaultStart, int defaultStop, int prec);

    /**
     * 倒着查，从车站事件表中查找最后一个符合指定方向、位置的事件。
     * 如果找不到（极端情况），返回空。
     */
    std::shared_ptr<const RailStationEvent>
        findLastEvent(const RailStationEventList& lst, const TrainFilterCore& filter,
            const Direction& dir,
            RailStationEvent::Positions pos)const;
};


