/*
 * 原pyETRC中Line类
 * 注意：与区间相关时，
 * prev, next指运行方向；
 * left, right指列表方向 （列表先后顺序）；
 * up, down指行别。
 */
#ifndef RAILWAY_H
#define RAILWAY_H

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

    /*
     * 注意这里相当于只是保存了头结点
     */
    QList<Ruler> _rulers;

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

    const std::shared_ptr<RailStation>
        stationByName(const StationName& name)const;

    /*
     * 2021.06.16新增
     * 仅允许以下两种情况：严格匹配，或者非Bare匹配到Bare类型
     */
    std::shared_ptr<RailStation> stationByGeneralName(const StationName& name);

    const std::shared_ptr<RailStation>
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

    /*
     * 与pyETRC不同：暂定不存在的站返回-1
     */
    int stationIndex(const StationName& name)const;

    /*
     * Line.delStation
     * 线性算法
     */
    void removeStation(const StationName& name);

    void removeStation(int index);

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

    /*
     * Line.isSplited()
     * 上下行分设情况
     */
    bool isSplitted()const;

    /*
     * Line.resetRulers()
     * 设置标尺的上下行分设等信息
     */
    void updateRulers();

    inline int stationCount()const {
        return _stations.count();
    }

    inline const QList<std::shared_ptr<RailStation>>& 
        stations()const {
        return _stations;
    }

    bool rulerNameExisted(const QString& name)const;

    /// <summary>
    /// 签名形式未确定
    /// </summary>
    bool isNewRuler()const;

    /// <summary>
    /// 签名未确定
    /// </summary>
    void removeRuler();

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

    inline const Ruler& getRuler(int i)const{return _rulers[i];}
    inline Ruler& getRuler(int i){return _rulers[i];}


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
    /// previous是指：上行方向
    /// 带index的需要时再实现
    /// </summary>
    /// <param name="cur">当前下标</param>
    /// <param name="down">行别</param>
    int leftDirStationIndex(int cur, bool down)const;

    std::shared_ptr<RailStation>
        leftDirStation(int cur, bool down)const;

    int rightDirStationIndex(int cur, bool down)const;

    std::shared_ptr<RailStation>
        rightDirStation(int cur, bool down)const;

    /*
     * 添加一个区间，代替RailInterval的构造函数。
     * 注意shared_from_this不能再构造函数中调用。
     */
    std::shared_ptr<RailInterval> addInterval(bool down,
            std::shared_ptr<RailStation> from,
            std::shared_ptr<RailStation> to);



};

#endif // RAILWAY_H
