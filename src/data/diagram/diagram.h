#pragma once

#include "data/common/common_header.h"
#include <memory>
#include <QList>
#include <QJsonObject>
#include <QString>
#include <QStringView>
#include <map>
#include "config.h"
#include "data/train/traincollection.h"
#include "data/train/traintype.h"
#include "data/diagram/trainline.h"
#include "diagrampage.h"


/**
 * PyETRC中system.json 默认配置文件的抽象
 */
class SystemJson {
public:
    static constexpr int history_count = 20;
    static SystemJson instance;

    QString last_file;
    QString default_file = "sample.pyetgr";
    int table_row_height = 25;

    bool show_train_tooltip = true;

    //todo: dock show..

    /**
     * 新增  历史记录  从front加，back出
     */
    QList<QString> history;

    void saveFile();

    /**
     * 添加历史记录文件；同时记录为上一次的文件
     */
    void addHistoryFile(const QString& name);

    ~SystemJson();

private:
    /**
     * 构造函数直接读文件
     */
    SystemJson();

    void fromJson(const QJsonObject& obj);

    QJsonObject toJson()const;
};


class Train;
class Railway;

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

    /**
     * 读取默认配置文件 config.json
     * 包含Config和类型系统两部分
     * 注意：似乎应当移动到mainWindow的启动操作里面去 
     * （导入运行图等操作也要创建临时的Diagram对象）
     */
    bool readDefaultConfigs(const QString& filename= "config.json");

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
     * @param t  更新的车次，与所有线路重新绑定
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

    /**
     * 创建默认的运行图视图，即按顺序包含本线的所有线路
     */
    std::shared_ptr<DiagramPage> createDefaultPage();

    bool pageNameExisted(const QString& name)const;

    bool pageNameIsValid(const QString& name, std::shared_ptr<DiagramPage> page);

    bool railwayNameExisted(const QString& name)const;

    QString validRailwayName(const QString& prefix)const;

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
    void removeRailwayAt(int i);

    /**
     * 更新参数（最大跨越站数）时执行
     * 重新绑定所有列车与所有线路
     */
    void rebindAllTrains();

    std::map<std::shared_ptr<RailInterval>, int>
        sectionTrainCount(std::shared_ptr<Railway> railway)const;

private:
    void bindAllTrains();
    QString validPageName(const QString& prefix)const;

    void sectionTrainCount(std::map<std::shared_ptr<RailInterval>, int>& res,
        std::shared_ptr<TrainLine> line)const;
};


